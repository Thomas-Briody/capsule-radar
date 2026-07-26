#pragma once
// ICAO aircraft type designator -> the name a human would use.
//
// ADS-B carries the four-character designator ("B38M"); nobody outside the
// industry reads that as a 737 MAX 8. This maps the types that actually turn up
// over Europe. Anything unlisted falls through to the raw designator, which is
// still better than a blank.
//
// Note the designator is coarser than reality: B38M covers both the 737 MAX 8
// and Ryanair's MAX 8-200, and E75L/E75S are both E175s. Exact sub-variants need
// a per-airframe lookup by registration (adsbdb /v0/aircraft/{mode_s}).
//
// Table lives in flash, so it costs no RAM. Lookup is a linear scan, which on a
// couple of hundred entries is a few microseconds — and only runs on a tap.

struct AcTypeName { const char *icao; const char *name; };

static const AcTypeName ACTYPE_NAMES[] = {
    // ---- Airbus ----
    { "A19N", "Airbus A319neo" },
    { "A20N", "Airbus A320neo" },
    { "A21N", "Airbus A321neo" },
    { "A318", "Airbus A318" },
    { "A319", "Airbus A319" },
    { "A320", "Airbus A320" },
    { "A321", "Airbus A321" },
    { "A306", "Airbus A300-600" },
    { "A310", "Airbus A310" },
    { "A332", "Airbus A330-200" },
    { "A333", "Airbus A330-300" },
    { "A338", "Airbus A330-800neo" },
    { "A339", "Airbus A330-900neo" },
    { "A342", "Airbus A340-200" },
    { "A343", "Airbus A340-300" },
    { "A345", "Airbus A340-500" },
    { "A346", "Airbus A340-600" },
    { "A359", "Airbus A350-900" },
    { "A35K", "Airbus A350-1000" },
    { "A388", "Airbus A380-800" },
    { "A3ST", "Airbus Beluga" },
    { "A337", "Airbus BelugaXL" },
    { "BCS1", "Airbus A220-100" },
    { "BCS3", "Airbus A220-300" },
    { "A400", "Airbus A400M Atlas" },

    // ---- Boeing ----
    { "B37M", "Boeing 737 MAX 7" },
    { "B38M", "Boeing 737 MAX 8" },
    { "B39M", "Boeing 737 MAX 9" },
    { "B3XM", "Boeing 737 MAX 10" },
    { "B732", "Boeing 737-200" },
    { "B733", "Boeing 737-300" },
    { "B734", "Boeing 737-400" },
    { "B735", "Boeing 737-500" },
    { "B736", "Boeing 737-600" },
    { "B737", "Boeing 737-700" },
    { "B738", "Boeing 737-800" },
    { "B739", "Boeing 737-900" },
    { "B712", "Boeing 717" },
    { "B703", "Boeing 707" },
    { "B741", "Boeing 747-100" },
    { "B742", "Boeing 747-200" },
    { "B743", "Boeing 747-300" },
    { "B744", "Boeing 747-400" },
    { "B748", "Boeing 747-8" },
    { "BLCF", "Boeing 747 Dreamlifter" },
    { "B752", "Boeing 757-200" },
    { "B753", "Boeing 757-300" },
    { "B762", "Boeing 767-200" },
    { "B763", "Boeing 767-300" },
    { "B764", "Boeing 767-400" },
    { "B772", "Boeing 777-200" },
    { "B773", "Boeing 777-300" },
    { "B77L", "Boeing 777-200LR" },
    { "B77W", "Boeing 777-300ER" },
    { "B778", "Boeing 777-8" },
    { "B779", "Boeing 777-9" },
    { "B788", "Boeing 787-8" },
    { "B789", "Boeing 787-9" },
    { "B78X", "Boeing 787-10" },

    // ---- Embraer ----
    { "E135", "Embraer ERJ-135" },
    { "E145", "Embraer ERJ-145" },
    { "E170", "Embraer E170" },
    { "E175", "Embraer E175" },
    { "E75L", "Embraer E175" },
    { "E75S", "Embraer E175" },
    { "E190", "Embraer E190" },
    { "E195", "Embraer E195" },
    { "E290", "Embraer E190-E2" },
    { "E295", "Embraer E195-E2" },
    { "E50P", "Embraer Phenom 100" },
    { "E55P", "Embraer Phenom 300" },
    { "E545", "Embraer Legacy 450" },
    { "E550", "Embraer Legacy 500" },
    { "E35L", "Embraer Legacy 600" },

    // ---- Bombardier / de Havilland Canada ----
    { "CRJ1", "Bombardier CRJ100" },
    { "CRJ2", "Bombardier CRJ200" },
    { "CRJ7", "Bombardier CRJ700" },
    { "CRJ9", "Bombardier CRJ900" },
    { "CRJX", "Bombardier CRJ1000" },
    { "CL30", "Challenger 300" },
    { "CL35", "Challenger 350" },
    { "CL60", "Challenger 600" },
    { "GLEX", "Global Express" },
    { "GL5T", "Global 5000" },
    { "GL7T", "Global 7500" },
    { "DH8A", "Dash 8-100" },
    { "DH8B", "Dash 8-200" },
    { "DH8C", "Dash 8-300" },
    { "DH8D", "Dash 8 Q400" },
    { "DHC6", "Twin Otter" },

    // ---- Turboprops / regional ----
    { "AT43", "ATR 42-300" },
    { "AT45", "ATR 42-500" },
    { "AT46", "ATR 42-600" },
    { "AT72", "ATR 72" },
    { "AT75", "ATR 72-500" },
    { "AT76", "ATR 72-600" },
    { "SF34", "Saab 340" },
    { "SB20", "Saab 2000" },
    { "F50",  "Fokker 50" },
    { "F70",  "Fokker 70" },
    { "F100", "Fokker 100" },
    { "J328", "Dornier 328JET" },
    { "D328", "Dornier 328" },
    { "L410", "Let L-410" },
    { "SW4",  "Fairchild Metroliner" },
    { "JS32", "BAe Jetstream 32" },
    { "JS41", "BAe Jetstream 41" },
    { "B190", "Beech 1900" },
    { "B461", "BAe 146-100" },
    { "B462", "BAe 146-200" },
    { "B463", "BAe 146-300" },
    { "RJ85", "Avro RJ85" },
    { "RJ1H", "Avro RJ100" },
    { "CVLT", "Convair 580" },
    { "SU95", "Sukhoi Superjet 100" },

    // ---- McDonnell Douglas ----
    { "MD11", "McDonnell Douglas MD-11" },
    { "MD82", "McDonnell Douglas MD-82" },
    { "MD83", "McDonnell Douglas MD-83" },
    { "MD88", "McDonnell Douglas MD-88" },
    { "MD90", "McDonnell Douglas MD-90" },
    { "DC10", "McDonnell Douglas DC-10" },

    // ---- Business jets ----
    { "C25A", "Citation CJ2" },
    { "C25B", "Citation CJ3" },
    { "C25C", "Citation CJ4" },
    { "C510", "Citation Mustang" },
    { "C550", "Citation II" },
    { "C56X", "Citation Excel" },
    { "C68A", "Citation Latitude" },
    { "C700", "Citation Longitude" },
    { "C750", "Citation X" },
    { "GLF4", "Gulfstream IV" },
    { "GLF5", "Gulfstream V" },
    { "GLF6", "Gulfstream G650" },
    { "G280", "Gulfstream G280" },
    { "FA7X", "Falcon 7X" },
    { "FA8X", "Falcon 8X" },
    { "F2TH", "Falcon 2000" },
    { "F900", "Falcon 900" },
    { "H25B", "Hawker 800" },
    { "LJ35", "Learjet 35" },
    { "LJ45", "Learjet 45" },
    { "LJ60", "Learjet 60" },
    { "LJ75", "Learjet 75" },
    { "HDJT", "HondaJet" },
    { "PC24", "Pilatus PC-24" },

    // ---- Light aircraft ----
    { "PC12", "Pilatus PC-12" },
    { "C208", "Cessna Caravan" },
    { "C152", "Cessna 152" },
    { "C172", "Cessna 172" },
    { "C182", "Cessna 182" },
    { "C206", "Cessna 206" },
    { "BE20", "King Air 200" },
    { "B350", "King Air 350" },
    { "BE9L", "King Air 90" },
    { "BE58", "Beechcraft Baron" },
    { "BE36", "Beechcraft Bonanza" },
    { "PA31", "Piper Navajo" },
    { "PA34", "Piper Seneca" },
    { "PA46", "Piper Malibu" },
    { "P28A", "Piper PA-28" },
    { "P28R", "Piper Arrow" },
    { "TBM7", "Daher TBM 700" },
    { "TBM8", "Daher TBM 850" },
    { "TBM9", "Daher TBM 900" },
    { "SR20", "Cirrus SR20" },
    { "SR22", "Cirrus SR22" },
    { "DA40", "Diamond DA40" },
    { "DA42", "Diamond DA42" },
    { "DA62", "Diamond DA62" },
    { "M20P", "Mooney M20" },
    { "GA8",  "GippsAero Airvan" },
    { "GLID", "Glider" },
    { "BALL", "Balloon" },
    { "ULAC", "Microlight" },
    { "UAV",  "Drone" },

    // ---- Helicopters ----
    { "EC35", "Airbus H135" },
    { "EC45", "Airbus H145" },
    { "EC75", "Airbus H175" },
    { "AS50", "Airbus AS350" },
    { "A139", "Leonardo AW139" },
    { "A169", "Leonardo AW169" },
    { "A189", "Leonardo AW189" },
    { "S76",  "Sikorsky S-76" },
    { "S92",  "Sikorsky S-92" },
    { "B06",  "Bell 206" },
    { "B429", "Bell 429" },
    { "R44",  "Robinson R44" },
    { "R66",  "Robinson R66" },
    { "H60",  "Sikorsky Black Hawk" },

    // ---- Military / heavy freight ----
    { "C130", "C-130 Hercules" },
    { "C30J", "C-130J Hercules" },
    { "C17",  "C-17 Globemaster" },
    { "C5M",  "C-5 Galaxy" },
    { "K35R", "KC-135 Stratotanker" },
    { "E3TF", "E-3 Sentry (AWACS)" },
    { "P8",   "P-8 Poseidon" },
    { "EUFI", "Eurofighter Typhoon" },
    { "HAWK", "BAe Hawk" },
    { "F15",  "F-15 Eagle" },
    { "F16",  "F-16 Fighting Falcon" },
    { "F35",  "F-35 Lightning II" },
    { "A124", "Antonov An-124" },
    { "A225", "Antonov An-225" },
    { "AN12", "Antonov An-12" },
    { "AN26", "Antonov An-26" },
    { "AN72", "Antonov An-72" },
    { "IL76", "Ilyushin Il-76" },
    { "IL96", "Ilyushin Il-96" },
};

// Case-insensitive compare, written out rather than pulled from <strings.h> so
// this header builds the same on the device and the native simulator.
inline bool actype_eq(const char *a, const char *b) {
    for (;; ++a, ++b) {
        char ca = *a, cb = *b;
        if (ca >= 'a' && ca <= 'z') ca -= 32;
        if (cb >= 'a' && cb <= 'z') cb -= 32;
        if (ca != cb) return false;
        if (!ca) return true;
    }
}

inline const char *actype_name(const char *icao) {
    if (!icao || !icao[0]) return "";
    for (unsigned i = 0; i < sizeof(ACTYPE_NAMES) / sizeof(ACTYPE_NAMES[0]); ++i)
        if (actype_eq(icao, ACTYPE_NAMES[i].icao)) return ACTYPE_NAMES[i].name;
    return icao;   // unknown designator — show it raw rather than nothing
}
