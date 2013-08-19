/* json.c */

#include "json.h"
#include <stdio.h>
#include <string.h>

#define BUFLEN (1024 * 500)

static bool in_list = false;
static char buf[BUFLEN];
static char *buf_end = buf + BUFLEN - 1; // -1 to allow for final terminator char
static char *b = buf;

/* private functions */

static void overflow() { 
    fprintf(stderr, "prevented json buffer overflow.\n");
}

static void check(const char c) { 
    if (b >= buf_end) overflow();
    else *(b++) = c;
}

static void comma() { if (in_list) *(b++) = ','; }

/* Write a string out to the buffer, surrounding it in quotes and escaping all quotes or slashes. */
static void string (const char *s) {
    size_t n = 2; // first, check that there is enough space in the buffer (at least two quotes).
    for (const char *c = s; *c != '\0'; ++c) if (*c == '"' || *c == '/') n += 2; else ++n;
    if (b + n < buf_end) {
        *(b++) = '"';
        for (const char *c = s; *c != '\0'; ++c) {
            if (*c == '"' || *c == '/') (*b++) = '/';
            (*b++) = *c;
        }
        *(b++) = '"';
    } else overflow();        
}

/* Escape a key and copy it to the buffer, preparing for a single value. 
   This should only be used internally, since it sets in_list before the value is printed. */
static void ekey (const char *k) {
    comma();
    string(k);
    *(b++) = ':';
    in_list = true;
}

/* public functions (eventually) */

static void json_begin() { b = buf; }

static void json_dump() { *b = '\0'; printf("%s\n", buf); }

static size_t json_length() { return b - buf; }

static void json_kv(char *key, char *value) {
    ekey(key);
    string(value);
}

static void json_kd(char *key, int value) {
    ekey(key);
    b += sprintf(b, "%d", value);
}

static void json_kl(char *key, long value) {
    ekey(key);
    b += sprintf(b, "%ld", value);
}

static void json_kb(char *key, bool value) {
    ekey(key);
    b += sprintf(b, value ? "true" : "false");
}

static void json_key_obj(char *key) {
    ekey(key);
    *(b++) = '{';
    in_list = false;
}

static void json_key_arr(char *key) {
    ekey(key);
    *(b++) = '[';
    in_list = false;
}

static void json_obj() {
    comma();
    *(b++) = '{';
    in_list = false;
}

static void json_arr() {
    comma();
    *(b++) = '[';
    in_list = false;
}

static void json_end_obj() {
    *(b++) = '}';
    in_list = true;
}

static void json_end_arr() {
    *(b++) = ']';
    in_list = true;
}

static void json_leg (struct leg *leg, tdata_t *tdata) {
    json_obj(); /* one leg */
                        
/* 
    "startTime": 1376897760000,
    "endTime": 1376898480000,
    "departureDelay": 0,
    "arrivalDelay": 0,
    "realTime": false,
    "distance": 2656.2383456335,
    "mode": "BUS",
    "route": "39",
    "agencyName": "RET",
    "agencyUrl": "http:\/\/www.ret.nl",
    "agencyTimeZoneOffset": 7200000,
    "routeColor": null,
    "routeType": 3,
    "routeId": "1836",
    "routeTextColor": null,
    "interlineWithPreviousLeg": false,
    "tripShortName": "48562",
    "tripBlockId": null,
    "headsign": "Rotterdam Centraal",
    "agencyId": "RET",
    "tripId": "2597372",
    "serviceDate": "20130819",
    "from": {
      "name": "Rotterdam, Nieuwe Crooswijksewe",
      "stopId": {
        "agencyId": "ARR",
        "id": "52272"
      },
      "stopCode": "HA2286",
      "platformCode": null,
      "lon": 4.49654,
      "lat": 51.934423,
      "arrival": 1376897759000,
      "departure": 1376897760000,
      "orig": null,
      "zoneId": null,
      "stopIndex": 3
    },
    "to": {
      "name": "Rotterdam, Rotterdam Centraal",
      "stopId": {
        "agencyId": "ARR",
        "id": "51175"
      },
      "stopCode": "HA3940",
      "platformCode": null,
      "lon": 4.467403,
      "lat": 51.923529,
      "arrival": 1376898480000,
      "departure": 1376898770000,
      "orig": null,
      "zoneId": null,
      "stopIndex": 10
    },
    "legGeometry": {
      "points": "cm~{HkfmZfI|JDfj@zMfHpUlHpI`\\nDzZdChr@",
      "levels": null,
      "length": 8
    },
    "notes": null,
    "alerts": null,
    "routeShortName": "39",
    "routeLongName": null,
    "boardRule": null,
    "alightRule": null,
    "rentedBike": false,
    "transitLeg": true,
    "duration": 720000,
    "intermediateStops": null,
    "steps": [
      
    ]
*/                        
    json_end_obj();
}

static void json_itinerary (struct itinerary *itin, tdata_t *tdata) {
    json_obj(); /* one itinerary */
        json_kd("duration", 5190000);
        json_kl("startTime", 1376896831000);
        json_kl("endTime", 1376902021000);
        json_kd("walkTime", 1491);
        json_kd("transitTime", 3267);
        json_kd("waitingTime", 432);
        json_kd("walkDistance", 1887);
        json_kb("walkLimitExceeded",true);
        json_kd("elevationLost",0);
        json_kd("elevationGained",0);
        json_kd("transfers", 2);
        json_key_arr("legs");
            for (int l = 0; l < itin->n_legs; ++l) json_leg (itin->legs + l, tdata);
        json_end_arr();    
    json_end_obj();
}

void render_plan_json(struct plan *plan, tdata_t *tdata) {
    json_begin();
    json_obj();
        json_kv("error", "null");
        json_key_obj("requestParameters");
            json_kv("time", "9:12am");
            json_kv("arriveBy", "false");
            json_kv("maxWalkDistance", "1500");
            json_kv("fromPlace", "51.93749209045435,4.51263427734375");
            json_kv("toPlace", "52.36469901960148,4.9053955078125");
            json_kv("date", "08-19-2013");
            json_kv("mode", "TRANSIT,WALK");
        json_end_obj();
        json_key_obj("plan");
            json_kl("date", 1376896320000);
            json_key_obj("from");
                json_kv("name", "Langepad");
                json_kd("stopId", 0);
                json_kd("stopCode", 0);
                json_kd("platformCode", 0);
                json_kd("lon", 4.50870);
                json_kd("lat", 51.9364);
            json_end_obj();
            json_key_obj("to");
                json_kv("name", "Weesperstraat");
                json_kd("lon", 4.9057266219348);
                json_kd("lat", 52.364778630779);
            json_end_obj();
            json_key_arr("itineraries");
                for (int i = 0; i < plan->n_itineraries; ++i) json_itinerary (plan->itineraries + i, tdata);
            json_end_arr();    
        json_end_obj();
        json_key_obj("debug");
            json_kd("precalculationTime", 12);
            json_kd("pathCalculationTime", 808);
            json_kb("timedOut", false);
        json_end_obj();
    json_end_obj();
    json_dump();
}
