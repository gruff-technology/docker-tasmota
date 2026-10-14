/*
  user_config_override.h - Gruff Hospitality relay firmware

  Target: Shelly 1PM Gen4 (ESP32-C6, BL0942), build env `tasmota32c6`.
  Build:  ./compile.sh tasmota32c6

  Relays connect to the live broker, hospitality.grufftechnology.ee:8883.
  Devices identify themselves by MAC (Topic %012X), which the API uses to
  auto-register them as unassigned.

  The broker requires a client certificate whose CN is the device's topic
  (use_identity_as_username + ACL patterns on %u), so each relay needs
  `TLSKey1`/`TLSKey2` from gruff-automation-ca/generate-client-cert.sh <MAC>
  before it can connect.

  The broker's own cert is Let's Encrypt (ISRG Root X1, built in). The Gruff
  CA trust anchor (local_ca_data.h, local_ca_descriptor.h, copied in by
  compile.sh) is also included, for the local dev broker.
*/

#ifndef _USER_CONFIG_OVERRIDE_H_
#define _USER_CONFIG_OVERRIDE_H_

// Bump to force these settings over whatever is already in a device's flash
// (otherwise they only apply on first boot or after `Reset 1`).
#undef  CFG_HOLDER
#define CFG_HOLDER             4702

/*********************************************************************************************\
 * Device template - Shelly 1PM Gen4
 *
 *   GPIO0  Led1_i        status LED (active low)
 *   GPIO1  Button1       PCB button (active low)
 *   GPIO3  ADC Temp      board NTC (AdcParam1 below)
 *   GPIO4  Relay1
 *   GPIO6  TX            -> BL0942
 *   GPIO7  BL0942 RX     9600 baud
 *   GPIO10 Switch_n1     SW terminal
\*********************************************************************************************/

#undef  USER_TEMPLATE
#define USER_TEMPLATE "{\"NAME\":\"Shelly 1PM Gen4\",\"GPIO\":[320,32,0,4736,224,0,3200,8161,0,0,192,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],\"FLAG\":0,\"BASE\":1}"
#undef  MODULE
#define MODULE                 USER_MODULE

#undef  FRIENDLY_NAME
#define FRIENDLY_NAME          "Gruff Relay"

/*********************************************************************************************\
 * Wi-Fi
\*********************************************************************************************/

#undef  STA_SSID1
#define STA_SSID1              "U"
#undef  STA_PASS1
#define STA_PASS1              "tulipunane vihmavari"

/*********************************************************************************************\
 * MQTT over TLS (mutual: client certificate via TLSKey1/TLSKey2)
\*********************************************************************************************/

#undef  MQTT_HOST
#define MQTT_HOST              "hospitality.grufftechnology.ee"
#undef  MQTT_PORT
#define MQTT_PORT              8883

// Topic and client id are the full MAC, e.g. tele/A0B765F1C001/STATE
#undef  MQTT_TOPIC
#define MQTT_TOPIC             "%012X"
#undef  MQTT_CLIENT_ID
#define MQTT_CLIENT_ID         "%012X"

#undef  TELE_PERIOD
#define TELE_PERIOD            15

#ifndef USE_MQTT_TLS
#define USE_MQTT_TLS
#endif
// Per-device client certificate, set with TLSKey1/TLSKey2.
#ifndef USE_MQTT_CLIENT_CERT
#define USE_MQTT_CLIENT_CERT
#endif
#define INCLUDE_LOCAL_CERT
#define OMIT_AWS_CERT

#undef  MQTT_TLS_ENABLED
#define MQTT_TLS_ENABLED       true
// Client certs (and the local dev broker's) are EC (prime256v1); without this
// the first handshake can fail with error 296 before Tasmota enables it itself.
#undef  MQTT_TLS_ECDSA
#define MQTT_TLS_ECDSA         true

/*********************************************************************************************\
 * Time - UTC, no DST
\*********************************************************************************************/

#undef  APP_TIMEZONE
#define APP_TIMEZONE           0

/*********************************************************************************************\
 * Timers
\*********************************************************************************************/

#ifndef USE_TIMERS
#define USE_TIMERS
#endif
#undef  TIMERS_ENABLED
#define TIMERS_ENABLED         true

/*********************************************************************************************\
 * First-boot commands
\*********************************************************************************************/

// NTC: 10k pull-down, 10k at 25 C, B 3350 (same as the Shelly Plus 1PM)
#undef  USER_BACKLOG
#define USER_BACKLOG           "AdcParam1 2,10000,10000,3350"

#endif  // _USER_CONFIG_OVERRIDE_H_
