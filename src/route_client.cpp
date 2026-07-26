// Route lookup via adsbdb.com (free, no API key): GET /v0/callsign/{callsign}.
// Returns "IATA~City" per end; the UI splits on the tilde. Device-only.
//
// adsbdb is a *callsign -> route* lookup table built from historical schedule
// data, not live flight tracking. Airlines reuse callsigns across city pairs, so
// the answer is sometimes a route this aircraft is nowhere near — RYR86DX came
// back as Weeze -> Nador for an aircraft descending into Luton. We can't fix the
// data, but we can refuse to display it: every route is checked against the
// aircraft's actual position and discarded if it doesn't fit. Showing nothing
// beats showing something wrong.
#include "route_client.h"
#include "config.h"
#include "geo.h"           // haversineKm
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <string.h>
#include <math.h>
#include <time.h>          // route-cache TTL

#define ROUTE_CACHE_MAX 200   // wrap the cache before it can crowd NVS
#define ROUTE_FMT_VER   5     // bump to invalidate cached routes when the stored format changes

// How far off the direct origin->destination path an aircraft may sit before we
// decide the route can't be its own. Measured as
//     (distance to origin) + (distance to destination) - (route length)
// which is ~0 for an aircraft on its route and grows quickly off it. Usefully,
// long routes are naturally forgiving — a 250 km weather dog-leg on a 7000 km
// flight adds only ~18 km of excess — so a single figure suits short-haul and
// long-haul alike. Raise it if legitimate routes start disappearing.
#define ROUTE_MAX_EXCESS_KM 150.0

// ----------------------------------------------------------------------------
// Plausibility
// ----------------------------------------------------------------------------
static bool route_plausible(double acLat, double acLon,
                            double oLat, double oLon, double dLat, double dLon) {
    if (isnan(acLat) || isnan(acLon)) return true;                  // no position -> can't judge
    if ((oLat == 0.0 && oLon == 0.0) || (dLat == 0.0 && dLon == 0.0)) return true;  // no airport coords
    const double len = geo::haversineKm(oLat, oLon, dLat, dLon);
    const double d1  = geo::haversineKm(acLat, acLon, oLat, oLon);
    const double d2  = geo::haversineKm(acLat, acLon, dLat, dLon);
    const double excess = d1 + d2 - len;
    if (excess > ROUTE_MAX_EXCESS_KM) {
        Serial.printf("[route] rejected: aircraft is %.0f km off this route\n", excess);
        return false;
    }
    return true;
}

// ----------------------------------------------------------------------------
// NVS cache.  Stored value: "epoch|from|to|oLat|oLon|dLat|dLon"
// ----------------------------------------------------------------------------

// strip spaces -> a valid NVS key (callsigns are <= 8 chars)
static void route_key(const char *callsign, char *out, size_t on) {
    size_t j = 0;
    for (const char *p = callsign; *p && j < on - 1; ++p)
        if (*p != ' ') out[j++] = *p;
    out[j] = 0;
}

void route_cache_begin() {
    Preferences p;
    if (!p.begin("routes", false)) return;
    if (p.getUChar("__v", 0) != ROUTE_FMT_VER) { p.clear(); p.putUChar("__v", ROUTE_FMT_VER); }
    p.end();
}

// Split into exactly 7 pipe-separated fields. 'from'/'to' use '~' internally and
// never contain '|', so this is unambiguous.
static bool split7(const String &v, String out[7]) {
    int start = 0;
    for (int i = 0; i < 6; ++i) {
        const int p = v.indexOf('|', start);
        if (p < 0) return false;
        out[i] = v.substring(start, p);
        start = p + 1;
    }
    out[6] = v.substring(start);
    return true;
}

bool route_cache_get(const char *callsign, double acLat, double acLon, RouteInfo &out) {
    memset(&out, 0, sizeof(out));
    if (!callsign || !callsign[0]) return false;
    char key[12];
    route_key(callsign, key, sizeof(key));
    if (!key[0]) return false;

    Preferences p;
    if (!p.begin("routes", true)) return false;
    const String v = p.getString(key, "");
    p.end();
    if (v.length() == 0) return false;

    String f[7];
    if (!split7(v, f)) return false;

    const uint32_t ts  = (uint32_t)f[0].toInt();
    const uint32_t now = (uint32_t)time(nullptr);      // expire stale routes (reused callsigns)
    if (now > 1700000000UL && ts > 1700000000UL && (now - ts) > 86400UL) return false;  // 24 h TTL

    const double oLat = f[3].toDouble(), oLon = f[4].toDouble();
    const double dLat = f[5].toDouble(), dLon = f[6].toDouble();
    // Re-check on every read: the cache is keyed by callsign alone, so a route
    // that was right for yesterday's aircraft may be wrong for today's.
    if (!route_plausible(acLat, acLon, oLat, oLon, dLat, dLon)) return false;

    snprintf(out.from, sizeof(out.from), "%s", f[1].c_str());
    snprintf(out.to,   sizeof(out.to),   "%s", f[2].c_str());
    out.oLat = oLat; out.oLon = oLon;
    out.dLat = dLat; out.dLon = dLon;
    return true;
}

void route_cache_put(const char *callsign, const RouteInfo &r) {
    if (!callsign || !callsign[0]) return;
    char key[12];
    route_key(callsign, key, sizeof(key));
    if (!key[0]) return;
    Preferences p;
    if (!p.begin("routes", false)) return;
    int n = p.getInt("__n", 0);
    if (n >= ROUTE_CACHE_MAX) { p.clear(); n = 0; }   // wrap to bound NVS usage
    char v[160];
    snprintf(v, sizeof(v), "%u|%s|%s|%.4f|%.4f|%.4f|%.4f",
             (unsigned)time(nullptr), r.from, r.to, r.oLat, r.oLon, r.dLat, r.dLon);
    if (p.putString(key, v) > 0) p.putInt("__n", n + 1);
    p.end();
}

// ----------------------------------------------------------------------------
// Network lookup
// ----------------------------------------------------------------------------

// "IATA~City" — the UI splits on the tilde. Never use '|' here: the cache
// already uses it as its own field separator.
static void pick_airport(JsonObjectConst ap, char *out, size_t n, double *lat, double *lon) {
    const char *iata = ap["iata_code"] | "";
    const char *muni = ap["municipality"] | "";
    String nm = (const char *)(ap["name"] | "");
    nm.replace(" International Airport", "");
    nm.replace(" Regional Airport", "");
    nm.replace(" Airport", "");
    nm.replace(" International", "");
    nm.trim();
    const char *city = muni[0] ? muni : nm.c_str();
    snprintf(out, n, "%s~%s", iata[0] ? iata : "---", city);
    *lat = ap["latitude"]  | 0.0;
    *lon = ap["longitude"] | 0.0;
}

bool route_fetch(const char *callsign, double acLat, double acLon, RouteInfo &out) {
    memset(&out, 0, sizeof(out));
    if (!callsign || !callsign[0] || WiFi.status() != WL_CONNECTED) return false;

    // strip spaces from the callsign
    char cs[12];
    size_t j = 0;
    for (const char *p = callsign; *p && j < sizeof(cs) - 1; ++p)
        if (*p != ' ') cs[j++] = *p;
    cs[j] = 0;
    if (j == 0) return false;

    char url[96];
    snprintf(url, sizeof(url), "https://api.adsbdb.com/v0/callsign/%s", cs);

    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    http.setReuse(false);
    http.setConnectTimeout(3000);   // short: runs on the feed task, don't stall the live poll
    http.setTimeout(6000);
    if (!http.begin(client, url)) return false;
    http.addHeader("User-Agent", ADSB_USER_AGENT);

    const int code = http.GET();
    if (code != 200) { http.end(); return false; }

    JsonDocument filter;
    for (const char *end : { "origin", "destination" }) {
        filter["response"]["flightroute"][end]["municipality"] = true;
        filter["response"]["flightroute"][end]["iata_code"]    = true;
        filter["response"]["flightroute"][end]["name"]         = true;
        filter["response"]["flightroute"][end]["latitude"]     = true;
        filter["response"]["flightroute"][end]["longitude"]    = true;
    }

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, http.getStream(),
                                               DeserializationOption::Filter(filter));
    http.end();
    if (err) return false;

    JsonObjectConst fr = doc["response"]["flightroute"].as<JsonObjectConst>();
    if (fr.isNull()) return false;   // "unknown callsign" etc.

    pick_airport(fr["origin"].as<JsonObjectConst>(),      out.from, sizeof(out.from), &out.oLat, &out.oLon);
    pick_airport(fr["destination"].as<JsonObjectConst>(), out.to,   sizeof(out.to),   &out.dLat, &out.dLon);

    if (!route_plausible(acLat, acLon, out.oLat, out.oLon, out.dLat, out.dLon)) {
        memset(&out, 0, sizeof(out));
        return false;
    }
    return (out.from[0] || out.to[0]);
}
