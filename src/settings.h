#ifndef SETTING_H
#define SETTING_H

#include "DHT.h"

#define DEVICE_NAME "LivingRoomTV"

#define MQTT_TOPIC_GET "get/apartment/livingRoom/tv"
#define MQTT_TOPIC_SET "set/apartment/livingRoom/tv"

#define PIN_IR_RECEVIER D1        // IR Receiver PIN
#define PIN_IR_SENDER D2          // IR LED PIN
#define PIN_TEMP D3               // DHT 22 PIN
#define TEMP_SENSOR_TYPE DHT22
#define TEMP_SENSOR_CORRECTION -1 // The correction in degrees
#define MQTT_TOPIC_TEMPERATURE_GET "get/temperature/apartment/livingRoom/tv"

#define TIME_BETWEEN_TRANSMITTION 500
#define TIME_BETWEEN_CHANNEL_DIGITS_TRANSMITTION 150

//////////// Codes Set Top Box MegaLan
// Navigation Controls
const unsigned long codeNecNavUp    = 0xFFE21D;
const unsigned long codeNecNavDown  = 0xFFD22D;
const unsigned long codeNecNavRight = 0xFF52AD;
const unsigned long codeNecNavLeft  = 0xFF12ED;

// Regular Buttons
const unsigned long codeNecPlayPause = 0xFF22DD;   // Requres to send
                                                   // codeNecSpecialRepeatSequence
const unsigned long codeNecStop = 0xFFA25D;        // Requres to send
                                                   // codeNecSpecialRepeatSequence

const unsigned long codeNecRewind = 0xFFC23D;      // Requres to send
                                                   // codeNecSpecialRepeatSequence
const unsigned long codeNecFastForward = 0xFF629D; // Requres to send
                                                   // codeNecSpecialRepeatSequence
const unsigned long codeNecPrevious = 0xFFB04F;    // Requres to send
                                                   // codeNecSpecialRepeatSequence
const unsigned long codeNecNext = 0xFFC837;        // Requres to send
                                                   // codeNecSpecialRepeatSequence
const unsigned long codeNecChannelUp   = 0xFFD02F;
const unsigned long codeNecChannelDown = 0xFFA857;
const unsigned long codeNecVolUp       = 0xFF30CF;
const unsigned long codeNecVolDown     = 0xFF18E7;
const unsigned long codeNecOk          = 0xFF926D;
const unsigned long codeNecBack        = 0xFF32CD;
const unsigned long codeNecPower       = 0xFF02FD; // Requres to send
                                                   // codeNecSpecialRepeatSequence
const unsigned long codeNecKey0                  = 0xFF00FF;
const unsigned long codeNecKey1                  = 0xFF807F;
const unsigned long codeNecKey2                  = 0xFF40BF;
const unsigned long codeNecKey3                  = 0xFFC03F;
const unsigned long codeNecKey4                  = 0xFF20DF;
const unsigned long codeNecKey5                  = 0xFFA05F;
const unsigned long codeNecKey6                  = 0xFF609F;
const unsigned long codeNecKey7                  = 0xFFE01F;
const unsigned long codeNecKey8                  = 0xFF10EF;
const unsigned long codeNecKey9                  = 0xFF906F;
const unsigned long codeNecSpecialRepeatSequence = 0xFFFFFFFF;

//////////// Codes Samsung 1+ provider
const unsigned long code1PlusNavUp       = 0x10EF50AF;
const unsigned long code1PlusNavDown     = 0x10EFD02F;
const unsigned long code1PlusNavLeft     = 0x10EFB04F;
const unsigned long code1PlusNavRight    = 0x10EF30CF;
const unsigned long code1PlusNavEnter    = 0x10EF9867;
const unsigned long code1PlusPower       = 0x10EFF00F;
const unsigned long code1PlusChannelList = 0x10EF28D7;
const unsigned long code1PlusKey0        = 0x10EF906F;
const unsigned long code1PlusKey1        = 0x10EF00FF;
const unsigned long code1PlusKey2        = 0x10EF807F;
const unsigned long code1PlusKey3        = 0x10EF40BF;
const unsigned long code1PlusKey4        = 0x10EFC03F;
const unsigned long code1PlusKey5        = 0x10EF20DF;
const unsigned long code1PlusKey6        = 0x10EFA05F;
const unsigned long code1PlusKey7        = 0x10EF609F;
const unsigned long code1PlusKey8        = 0x10EFE01F;
const unsigned long code1PlusKey9        = 0x10EF10EF;
const unsigned long code1PlusKeyPreCh    = 0x10EFE817;

//////////// Codes Samsung from Simple Remote Control
const unsigned long codeSamsungSimpleRCKeyMenu   = 0xE0E058A7;
const unsigned long codeSamsungSimpleRCKeyTools  = 0xE0E0D22D;
const unsigned long codeSamsungSimpleRCKeyUp     = 0xE0E006F9;
const unsigned long codeSamsungSimpleRCKeyDown   = 0xE0E08679;
const unsigned long codeSamsungSimpleRCKeyRight  = 0xE0E046B9;
const unsigned long codeSamsungSimpleRCKeyLeft   = 0xE0E0A659;
const unsigned long codeSamsungSimpleRCOk        = 0xE0E016E9;
const unsigned long codeSamsungSimpleRCReturn    = 0xE0E01AE5;
const unsigned long codeSamsungSimpleRCMute      = 0xE0E0F00F;
const unsigned long codeSamsungSimpleVolumeUp    = 0xE0E0E01F;
const unsigned long codeSamsungSimpleVolumeDown  = 0xE0E0D02F;
const unsigned long codeSamsungSimplePower       = 0xE0E040BF;
const unsigned long codeSamsungSimplePlay        = 0xE0E0E21D;
const unsigned long codeSamsungSimplePause       = 0xE0E052AD;
const unsigned long codeSamsungSimpleStop        = 0xE0E0629D;
const unsigned long codeSamsungSimpleRewind      = 0xE0E0A25D;
const unsigned long codeSamsungSimpleFastForward = 0xE0E012ED;

// Key Combination Variables
#define KEY_COMBINATION_COUNT 4
int lastPressedKeysIndex                      = 0;
unsigned long timeOfTheLastPressedKeySequance = 0;
unsigned long lastPressedKeys[KEY_COMBINATION_COUNT];
unsigned long keyCombinationTVSleepTimer60[KEY_COMBINATION_COUNT] =
{ code1PlusNavLeft, code1PlusNavRight, code1PlusNavLeft, code1PlusNavRight };

unsigned long keyCombinationTVBrightnessLow[KEY_COMBINATION_COUNT] =
{ code1PlusNavLeft, code1PlusNavLeft, code1PlusNavLeft, code1PlusNavRight };

unsigned long keyCombinationTVBrightnessHigh[KEY_COMBINATION_COUNT] =
{ code1PlusNavLeft, code1PlusNavLeft, code1PlusNavRight, code1PlusNavLeft };

unsigned long keyCombinationTVScreenOff[KEY_COMBINATION_COUNT] =
{ code1PlusNavLeft, code1PlusNavLeft, code1PlusNavRight, code1PlusNavRight };

unsigned long keyCombinationSTBPowenOnOFF[KEY_COMBINATION_COUNT] =
{ code1PlusNavRight, code1PlusNavLeft, code1PlusNavRight, code1PlusNavLeft };

unsigned long invalidCode = 0xFFFFFF;
int codeLen               = 32;

#endif // ifndef SETTING_H
