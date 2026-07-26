#pragma once
// Look up a flight's origin/destination by callsign via adsbdb.com (free, no key).
// Device-only (uses WiFi/HTTPS). City names are returned in English.
//
// adsbdb is a callsign -> route lookup table, not live tracking, so it can hand
// back a route the aircraft is nowhere near. Every lookup is therefore checked
// against the aircraft's real position; pass NAN for acLat/acLon to skip the check.
#include <stddef.h>

struct RouteInfo {
    char   from[48];              // "IATA~City" — the UI splits on the tilde
    char   to[48];
    double oLat, oLon;            // origin airport
    double dLat, dLon;            // destination airport
};

bool route_fetch(const char *callsign, double acLat, double acLon, RouteInfo &out);

// NVS route cache (avoids re-querying adsbdb for the same flight across reboots).
void route_cache_begin();                                   // call once at boot; clears on format change
bool route_cache_get(const char *callsign, double acLat, double acLon, RouteInfo &out);
void route_cache_put(const char *callsign, const RouteInfo &r);
