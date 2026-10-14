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

// These settings apply on first boot and after `Reset 1`. Bumping this does
// NOT reliably re-apply them over an earlier build: on ESP32 a changed
// CFG_HOLDER restores the last-known-good settings file first. Send `Reset 1`.
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
 * Wi-Fi and web UI credentials: user_config_secrets.h, kept out of git and
 * copied in by compile.sh (template: user_config_secrets.example.h).
\*********************************************************************************************/

#include "user_config_secrets.h"

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
 * Fail-safe heating
 *
 * The API decides when a relay is on. If the relay can't reach it, the room
 * mustn't stay cold until it can: after 30 minutes without MQTT the relay
 * switches on and the room's thermostat regulates the floor alone, as without
 * Gruff. Once it reconnects, the API's next command corrects it.
 *
 * Mqtt#Disconnected fires on every failed reconnect attempt (about every two
 * minutes), which would keep restarting a plain timer, so Var1 counts them
 * and only the first starts the timer. Mqtt#Connected resets both. A relay
 * that boots without a connection also gets its first Mqtt#Disconnected.
 *
 * After a power cut the relay starts on too (PowerOnState 1), for the same
 * reason.
\*********************************************************************************************/

#undef  APP_POWERON_STATE
#define APP_POWERON_STATE      POWER_ALL_ON

#define FAILSAFE_RULE \
  "ON Mqtt#Disconnected DO Add1 1 ENDON " \
  "ON Var1#State==1 DO RuleTimer1 1800 ENDON " \
  "ON Rules#Timer=1 DO Power1 1 ENDON " \
  "ON Mqtt#Connected DO Var1 0 ENDON " \
  "ON Mqtt#Connected DO RuleTimer1 0 ENDON"

/*********************************************************************************************\
 * First-boot commands (and after `Reset 1`), run as one Backlog: no `;`
 * inside a command.
\*********************************************************************************************/

#undef  USER_BACKLOG
#define USER_BACKLOG           "AdcParam1 2,10000,10000,3350;Rule1 " FAILSAFE_RULE ";Rule1 1"

#endif  // _USER_CONFIG_OVERRIDE_H_
