#include <Arduino.h>
#include <ArduinoJson.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <MqttClient.h>
#include <FotaClient.h>
#include <ESPWifiClient.h>
#include <RemotePrint.h>
#include <TemperatureClient.h>
#include <TV.h>
#include "settings.h"

IRsend irsend(PIN_IR_SENDER);

decode_results results;
IRrecv irrecv(PIN_IR_RECEVIER);  // From the TV send IR device

unsigned long codeValueReceived; // The code value if not raw
long lastStatusMsgSentAt  = 0;
long lastPressedKey       = 0;
long lastTransmissionTime = 0;

MqttClient *mqttClient               = NULL;
TemperatureClient *temperatureClient = new TemperatureClient();
FotaClient *fotaClient               = new FotaClient(DEVICE_NAME);
ESPWifiClient *wifiClient            = new ESPWifiClient(WIFI_SSID, WIFI_PASS);
TV *tv                               = new TV();

void resetLastPassedKeysArray() {
  for (int i = 0; i < KEY_COMBINATION_COUNT; i++) {
    lastPressedKeys[i] = invalidCode;
  }
  lastPressedKeysIndex = 0;
}

void sendIRCode(unsigned long code,
                bool          useNEC              = true,
                int           delayBeforeTransmit = 0,
                int           repeat              = 1) {
  PRINT("TV: Send IR Code: ");
  PRINTLN(code, HEX);
  PRINT_D("    delayBeforeTransmit: ");
  PRINTLN_D(delayBeforeTransmit, DEC);
  PRINT_D("    codeLen:");
  PRINTLN_D(codeLen, DEC);
  PRINT_D("    Type: ");

  delay(delayBeforeTransmit);

  for (uint8_t i = 0; i < repeat; i++) {
    if (useNEC == true) {
      irsend.sendNEC(code, codeLen);
      PRINTLN_D("NEC");
    } else {
      irsend.sendSAMSUNG(code, codeLen);
      PRINTLN_D("Samsung");
    }
    lastTransmissionTime = millis();

    if (i + 1 < repeat) {
      // Wait a bit between retransmissions
      delay(50);
    }
  }
}

void tvPublishStatus(const char *messageId    = NULL,
                     bool        forcePublish = false) {
  long now = millis();

  if ((forcePublish) or (now - lastStatusMsgSentAt >
                         MQTT_PUBLISH_STATUS_INTERVAL)) {
    lastStatusMsgSentAt = now;

    const size_t bufferSize = JSON_ARRAY_SIZE(4) + 5 * JSON_OBJECT_SIZE(1);
    DynamicJsonBuffer jsonBuffer(bufferSize);
    JsonObject& root   = jsonBuffer.createObject();
    JsonObject& status = root.createNestedObject("status");

    if (messageId != NULL) {
      root["messageId"] = messageId;
    }
    status["powerOn"]        = tv->powerOn;
    status["channelNumber"]  = tv->channelNumber;
    status["mute"]           = tv->muted;
    status["volume"]         = tv->volumeDesired;
    status["playbackAction"] = tv->playbackAction;

    // convert to String
    String outString;
    root.printTo(outString);

    // publish the message
    mqttClient->publish(MQTT_TOPIC_GET, outString);
  }
}

void tvSendChannelNumber(unsigned int channelNumber) {
  PRINT("TV: Send IR codes for TV channel: ");
  PRINTLN(channelNumber);

  unsigned int tempNum, divisor, digit, count = 0;
  tempNum = channelNumber;

  // Counting the number of digits in the entered integer
  while (tempNum != 0)
  {
    tempNum = tempNum / 10;
    count++;
  }

  tempNum = channelNumber;

  unsigned int timeBetweenTransmission = 0;

  // Extracting the digits
  do
  {
    divisor = static_cast<int>(pow(10.0, --count));
    digit   = tempNum / divisor;
    tempNum = tempNum % divisor;

    // send the digigs one by one
    switch (digit) {
      case 0:
      {
        sendIRCode(codeNecKey0);
        break;
      }
      case 1:
      {
        sendIRCode(codeNecKey1, true, timeBetweenTransmission);
        break;
      }
      case 2:
      {
        sendIRCode(codeNecKey2, true, timeBetweenTransmission);
        break;
      }
      case 3:
      {
        sendIRCode(codeNecKey3, true, timeBetweenTransmission);
        break;
      }
      case 4:
      {
        sendIRCode(codeNecKey4, true, timeBetweenTransmission);
        break;
      }
      case 5:
      {
        sendIRCode(codeNecKey5, true, timeBetweenTransmission);
        break;
      }
      case 6:
      {
        sendIRCode(codeNecKey6, true, timeBetweenTransmission);
        break;
      }
      case 7:
      {
        sendIRCode(codeNecKey7, true, timeBetweenTransmission);
        break;
      }
      case 8:
      {
        sendIRCode(codeNecKey8, true, timeBetweenTransmission);
        break;
      }
      case 9:
      {
        sendIRCode(codeNecKey9, true, timeBetweenTransmission);
        break;
      }
      default: { PRINTLN("Unknown digit."); }
    }
    timeBetweenTransmission = TIME_BETWEEN_CHANNEL_DIGITS_TRANSMITTION;
  }
  while (count != 0);
}

void sendSTBPowerOnOff() {
  PRINTLN("Sending STB Power On Off Signal.");
  sendIRCode(codeNecPower);
  sendIRCode(codeNecPower);
}

void tvAdjustTVVolume(int value) {
  tv->muted = false; // chaning the volume unmutes the TV

  if (tv->volume == -1) {
    for (int i = 0; i < abs(value); i++) {
      if (value > 0) {
        sendIRCode(codeSamsungSimpleVolumeUp, false);
      } else {
        sendIRCode(codeSamsungSimpleVolumeDown, false);
      }
    }
  } else {
    // Do not change the tv->volume as we do not have initial value
    int newVolume = tv->volumeDesired + value;

    if (newVolume < 0) {
      tv->volumeDesired = 0;
    } else {
      tv->volumeDesired = newVolume;
    }
  }
}

void tvSetVolumeIfNeeded() {
  if (tv->volume == tv->volumeDesired) {
    return;
  }
  tv->muted = false; // changing the volume unmutes the TV

  if (tv->volume < 0) {
    // As we don't know the current volume we will first set it to 0
    for (int i = 0; i <= 30; i++) {
      sendIRCode(codeSamsungSimpleVolumeDown, false);
    }
    tv->volume = -1;
  }

  if (tv->volumeDesired > 0) {
    int volumeDelta = tv->volume - tv->volumeDesired;

    // for (int i = 1; i <= abs(volumeDelta); i++) {
    if (volumeDelta < 0) {
      sendIRCode(codeSamsungSimpleVolumeUp, false, 75);
      tv->volume++;
    } else {
      sendIRCode(codeSamsungSimpleVolumeDown, false, 75);
      tv->volume--;
    }
  }
}

void tvIRSend(String payload) {
  PRINTLN("TV: Sending IR signal.");

  // parse the JSON
  const size_t bufferSize = JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(8) + 130;
  DynamicJsonBuffer jsonBuffer(bufferSize);
  JsonObject& root   = jsonBuffer.parseObject(payload);
  JsonObject& status = root.get<JsonObject&>("status");

  if (!status.success()) {
    PRINTLN(
      "AC: JSON with \"status\" key not received.");
#ifdef DEBUG_ENABLED
    root.prettyPrintTo(Serial);
#endif // ifdef DEBUG_ENABLED
    return;
  }

  const char *powerOnChar = status.get<const char *>("powerOn");

  if (powerOnChar) {
    bool powerOn = (strcasecmp(powerOnChar, "true") == 0);

    sendIRCode(codeSamsungSimplePower, false);
    tv->powerOn = powerOn;
  }

  const char *skipChannels = status.get<const char *>("skipChannels");

  if (skipChannels) {
    int skipChannelsInt = atoi(skipChannels);
    tv->channelNumber = tv->channelNumber + skipChannelsInt;

    if (skipChannelsInt > 0) {
      sendIRCode(codeNecChannelUp);
    }
    else if  (skipChannelsInt < 0) {
      sendIRCode(codeNecChannelDown);
    }
  }

  const char *channelNumber = status.get<const char *>("changeChannel");

  if (channelNumber) {
    tv->channelNumber = atoi(channelNumber);
    tvSendChannelNumber(tv->channelNumber);
  }

  const char *setVolumeChar = status.get<const char *>("SetVolume");

  if (setVolumeChar) {
    tv->volumeDesired = atoi(setVolumeChar);
  }

  const char *adjustVolumeChar = status.get<const char *>("AdjustVolume");

  if (adjustVolumeChar) {
    int adjustVolume = atoi(adjustVolumeChar);

    if (abs(adjustVolume) >= 10) {
      adjustVolume = adjustVolume / 10;
    }
    tvAdjustTVVolume(adjustVolume);
  }

  const char *setMuteChar = status.get<const char *>("SetMute");

  if (setMuteChar) {
    bool muted = (strcasecmp(setMuteChar, "true") == 0);

    sendIRCode(codeSamsungSimpleRCMute, false);
    tv->muted = muted;
  }
  String playbackAction = status.get<String>("playbackAction");

  if (playbackAction) {
    if (strcasecmp(playbackAction.c_str(), "play") == 0) {
      PRINTLN("PLAY");
      sendIRCode(codeNecPlayPause);
      sendIRCode(codeNecSpecialRepeatSequence);
      sendIRCode(codeSamsungSimplePlay, false);
    } else if (strcasecmp(playbackAction.c_str(), "pause") == 0) {
      PRINTLN("PAUSE");
      sendIRCode(codeNecPlayPause);
      sendIRCode(codeNecSpecialRepeatSequence);
      sendIRCode(codeSamsungSimplePause, false);
    } else if (strcasecmp(playbackAction.c_str(), "stop") == 0) {
      PRINTLN("STOP");
      sendIRCode(codeNecStop);
      sendIRCode(codeNecSpecialRepeatSequence);
      sendIRCode(codeSamsungSimpleStop, false);
    } else if (strcasecmp(playbackAction.c_str(), "rewind") == 0) {
      PRINTLN("REWIND");
      sendIRCode(codeNecRewind);
      sendIRCode(codeNecSpecialRepeatSequence);
      sendIRCode(codeSamsungSimpleRewind, false);
    } else if (strcasecmp(playbackAction.c_str(), "fastForward") == 0) {
      PRINTLN("FAST FORWARD");
      sendIRCode(codeNecFastForward);
      sendIRCode(codeNecSpecialRepeatSequence);
      sendIRCode(codeSamsungSimpleFastForward, false);
    } else if (strcasecmp(playbackAction.c_str(), "previous") == 0) {
      PRINTLN("PREVIOUS");
      sendIRCode(codeNecPrevious);
      sendIRCode(codeNecSpecialRepeatSequence);

      // "Previous" action is not avaliable in Samsung
    } else if (strcasecmp(playbackAction.c_str(), "next") == 0) {
      PRINTLN("NEXT");
      sendIRCode(codeNecNext);
      sendIRCode(codeNecSpecialRepeatSequence);

      // "Next" action is not avaliable in Samsung
    }
    tv->playbackAction = playbackAction;
  }


  const char *messageId = root.get<const char *>("messageId");
  tvPublishStatus(messageId, true);
}

void mqttCallback(char *topic, byte *payload, unsigned int length) {
  PRINT("MQTT Message arrived [");
  PRINT(topic);
  PRINTLN("] ");

  // Convert the payload to string
  char spayload[length + 1];
  memcpy(spayload, payload, length);
  spayload[length] = '\0';
  String payloadString = String(spayload);

  // Do something according the topic
  if (strcmp(topic, MQTT_TOPIC_SET) == 0) {
    tvIRSend(payloadString);
  }
  else {
    PRINT("MQTT: Warning: Unknown topic: ");
    PRINTLN(topic);
  }
}

void setup()
{
  wifiClient->init();
  mqttClient = new MqttClient(MQTT_SERVER,
                              MQTT_SERVER_PORT,
                              DEVICE_NAME,
                              MQTT_USERNAME,
                              MQTT_PASS,
                              MQTT_TOPIC_SET,
                              MQTT_SERVER_FINGERPRINT,
                              mqttCallback);

  temperatureClient->init(DEVICE_NAME,
                          PIN_TEMP,
                          TEMP_SENSOR_TYPE,
                          mqttClient,
                          MQTT_TOPIC_TEMPERATURE_GET,
                          TEMP_SENSOR_CORRECTION);
  fotaClient->init();

  resetLastPassedKeysArray();
  #ifdef DEBUG_ENABLED
  Serial.begin(115200);
  #endif // ifdef DEBUG_ENABLED


  // Sleeping for a while to allow the TV menu to show
  // delay(1500);
  // sendIRCode(codeNecBack);


  // for (int i = 200; i >= 0; i--) {
  //   sendIRCode(codeNecNavLeft);
  //   delay(1000);
  // }

  // Start the IR sender
  irsend.begin();

  // Start the IR receiver
  irrecv.enableIRIn();
}

void storeCode(decode_results *results) {
  // FIXME: No need to have this method, but first find out what actualy it does
  codeValueReceived = results->value;

  // FIXME: For some reason the code below is needed, otherwise the program will
  // not work:(
  PRINT_D("TV: Received: ");
  PRINTLN_D(codeValueReceived, HEX);
}

bool compareArrays(unsigned long actualArray[],
                   unsigned long expectedArray[],
                   int           n,
                   bool        & isSequance) {
  for (int i = 0; i < n; i++) {
    if (actualArray[i] != invalidCode) {
      if (actualArray[i] != expectedArray[i]) {
        // invalid sequance
        isSequance = false;

        // resetLastPassedKeysArray();
        return false;
      } else {
        // not full, but valid sequance
      }
    } else {
      return false;
    }
  }

  // the sequance is fully correct
  return true;
}

void sendTVBrightness(bool highSettings = true) {
  PRINTLN("TV: Sending sendTVBrightness Combination");
  sendIRCode(codeSamsungSimpleRCKeyMenu,  false,
             TIME_BETWEEN_TRANSMITTION);
  sendIRCode(codeSamsungSimpleRCKeyRight, false, 1500);
  sendIRCode(codeSamsungSimpleRCKeyDown,  false, 500);
  sendIRCode(codeSamsungSimpleRCKeyRight, false, 500);

  uint8 delayBetweenButtons = 50;
  uint8 blacklight          = 1;
  uint8 contrast            = 10;

  if (highSettings) {
    blacklight = 14;
    contrast   = 45;
  }

  // Blacklight
  for (uint8_t i = 0; i < 25; i++) {
    sendIRCode(codeSamsungSimpleRCKeyLeft, false, delayBetweenButtons);
  }

  for (uint8_t i = 0; i < blacklight; i++) {
    sendIRCode(codeSamsungSimpleRCKeyRight, false, delayBetweenButtons);
  }

  // Contrast
  sendIRCode(codeSamsungSimpleRCKeyDown, false, 1500);

  for (uint8_t i = 0; i < 50; i++) {
    sendIRCode(codeSamsungSimpleRCKeyLeft, false, delayBetweenButtons);
  }

  for (uint8_t i = 0; i < contrast; i++) {
    sendIRCode(codeSamsungSimpleRCKeyRight, false, delayBetweenButtons);
  }

  // Exit
  for (uint8_t i = 0; i < 2; i++) {
    // sendIRCode(codeSamsungSimpleRCReturn, false, 1500);
    sendIRCode(codeSamsungSimpleRCMute, false, 500);
  }
}

void sendTVSleepTimer(bool is60min) {
  PRINTLN("TV: Sending sendTVScreenOff Combination");
  sendIRCode(codeSamsungSimpleRCKeyTools, false,
             TIME_BETWEEN_TRANSMITTION);
  sendIRCode(codeSamsungSimpleRCKeyDown,  false, 1500);
  sendIRCode(codeSamsungSimpleRCKeyDown,  false,
             TIME_BETWEEN_TRANSMITTION);
  sendIRCode(codeSamsungSimpleRCKeyDown,  false,
             TIME_BETWEEN_TRANSMITTION);
  sendIRCode(codeSamsungSimpleRCOk,       false,
             TIME_BETWEEN_TRANSMITTION);
  sendIRCode(codeSamsungSimpleRCKeyDown,  false,
             TIME_BETWEEN_TRANSMITTION);

  if (is60min == true) {
    sendIRCode(codeSamsungSimpleRCKeyDown, false,
               TIME_BETWEEN_TRANSMITTION);
  }
  sendIRCode(codeSamsungSimpleRCOk,     false, TIME_BETWEEN_TRANSMITTION);
  sendIRCode(codeSamsungSimpleRCReturn, false, TIME_BETWEEN_TRANSMITTION);
}

void sendTVScreenOff() {
  PRINTLN("TV: Sending sendTVSleepTimer Combination");
  sendIRCode(codeSamsungSimpleRCKeyTools, false,
             TIME_BETWEEN_TRANSMITTION);
  sendIRCode(codeSamsungSimpleRCKeyUp,    false, 1500);
  sendIRCode(codeSamsungSimpleRCOk,       false,
             TIME_BETWEEN_TRANSMITTION);
}

bool isKeyCombination(unsigned long code) {
  unsigned int timeSinceLastSequanceKeyPressed = 0;

  if (timeOfTheLastPressedKeySequance == 0) {
    timeOfTheLastPressedKeySequance = millis();
  }

  timeSinceLastSequanceKeyPressed = millis()  - timeOfTheLastPressedKeySequance;

  PRINT_D("TV: Milis since last key pressed: ");
  PRINTLN_D(timeSinceLastSequanceKeyPressed);

  if (timeSinceLastSequanceKeyPressed >= 5000) {
    // Too much time since last sequance key was pressed
    PRINTLN_D(
      "TV: Too much time since last sequance key was pressed. Resetting the sequance.");
    timeOfTheLastPressedKeySequance = 0;
    resetLastPassedKeysArray();
  }
  else {
    timeOfTheLastPressedKeySequance = millis();
  }

  lastPressedKeys[lastPressedKeysIndex] = code;

  if (lastPressedKeysIndex == (KEY_COMBINATION_COUNT - 1)) {
    lastPressedKeysIndex = 0;
  } else {
    lastPressedKeysIndex++;
  }
  PRINTLN_D("TV: lastPressedKeys Start");

  // FIXME: For some reason the code below is needed, otherwise the program will
  // not work:(
  for (int i = 0; i < KEY_COMBINATION_COUNT; i++) {
    Serial.println(lastPressedKeys[i], HEX);
  }
  PRINTLN_D("TV: lastPressedKeys End");

  bool isCombination = false;
  bool isAnySequance = false;

  // TV sleep timer 60 min
  bool isSequance = true;

  if (!isCombination &&
      compareArrays(lastPressedKeys, keyCombinationTVSleepTimer60,
                    KEY_COMBINATION_COUNT, isSequance)) {
    sendTVSleepTimer(true);
    isCombination = true;
  }

  if (isSequance || isAnySequance) {
    isAnySequance = true;
  }

  // TV Brightness Low
  isSequance = true;

  if (!isCombination &&
      compareArrays(lastPressedKeys, keyCombinationTVBrightnessLow,
                    KEY_COMBINATION_COUNT, isSequance)) {
    sendTVBrightness(false);
    isCombination = true;
  }

  if (isSequance || isAnySequance) {
    isAnySequance = true;
  }

  // TV Brightness High
  isSequance = true;

  if (!isCombination &&
      compareArrays(lastPressedKeys, keyCombinationTVBrightnessHigh,
                    KEY_COMBINATION_COUNT, isSequance)) {
    sendTVBrightness(true);
    isCombination = true;
  }

  if (isSequance || isAnySequance) {
    isAnySequance = true;
  }

  // STB on/off
  isSequance = true;

  if (!isCombination &&
      compareArrays(lastPressedKeys, keyCombinationSTBPowenOnOFF,
                    KEY_COMBINATION_COUNT, isSequance)) {
    sendSTBPowerOnOff();
    isCombination = true;
  }

  if (isSequance || isAnySequance) {
    isAnySequance = true;
  }

  // TV Screen off
  isSequance = true;

  if (!isCombination &&
      (compareArrays(lastPressedKeys, keyCombinationTVScreenOff,
                     KEY_COMBINATION_COUNT, isSequance))) {
    sendTVScreenOff();
    isCombination = true;
  }

  if (isSequance || isAnySequance) {
    isAnySequance = true;
  }

  if (isCombination || !isAnySequance) {
    resetLastPassedKeysArray();
  }

  if (isCombination) {
    return true;
  }
  return false;

  PRINT_D("TV: isCombination: ");
  PRINTLN_D(isCombination);
  PRINT_D("TV: isAnySequance: ");
  PRINTLN_D(isAnySequance);

  PRINTLN_D("TV: lastPressedKeys Start");

  for (int i = 0; i < KEY_COMBINATION_COUNT; i++) {
    PRINTLN_D(lastPressedKeys[i], HEX);
  }
  PRINTLN_D("TV: lastPressedKeys End");
  return false;
}

void tvGuessChannelNumber(uint8 digit) {
  // The idea is to guess the channel number by the pressed buttons
  if (millis() <= lastPressedKey + (unsigned)1000) {
    tv->channelNumber = tv->channelNumber * 10 + digit;
  } else {
    tv->channelNumber = digit;
  }
  lastPressedKey = millis();
}

void irLoop() {
  if (irrecv.decode(&results)) {
    irrecv.disableIRIn();
    storeCode(&results);

    switch (codeValueReceived) {
      case code1PlusNavUp:
      {
        sendIRCode(codeNecNavUp);
        tv->channelNumber++;
        break;
      }
      case code1PlusNavDown:
      {
        sendIRCode(codeNecNavDown);
        tv->channelNumber--;
        break;
      }
      case code1PlusNavLeft:
      {
        sendIRCode(codeNecNavLeft);
        break;
      }
      case code1PlusNavRight:
      {
        sendIRCode(codeNecNavRight);
        break;
      }
      case code1PlusNavEnter:
      {
        sendIRCode(codeNecOk);
        break;
      }
      case code1PlusPower:
      {
        sendSTBPowerOnOff();

        // FIXME: replace with get real status
        // tv->powerOn = not (tv->powerOn);
        break;
      }
      case code1PlusChannelList:
      {
        sendIRCode(codeNecBack);
        break;
      }
      case code1PlusKey0:
      {
        sendIRCode(codeNecKey0);
        tvGuessChannelNumber(0);
        break;
      }
      case code1PlusKey1:
      {
        sendIRCode(codeNecKey1);
        tvGuessChannelNumber(1);
        break;
      }
      case code1PlusKey2:
      {
        sendIRCode(codeNecKey2);
        tvGuessChannelNumber(2);
        break;
      }
      case code1PlusKey3:
      {
        sendIRCode(codeNecKey3);
        tvGuessChannelNumber(3);
        break;
      }
      case code1PlusKey4:
      {
        sendIRCode(codeNecKey4);
        tvGuessChannelNumber(4);
        break;
      }
      case code1PlusKey5:
      {
        sendIRCode(codeNecKey5);
        tvGuessChannelNumber(5);
        break;
      }
      case code1PlusKey6:
      {
        sendIRCode(codeNecKey6);
        tvGuessChannelNumber(6);
        break;
      }
      case code1PlusKey7:
      {
        sendIRCode(codeNecKey7);
        tvGuessChannelNumber(7);
        break;
      }
      case code1PlusKey8:
      {
        sendIRCode(codeNecKey8);
        tvGuessChannelNumber(8);
        break;
      }
      case code1PlusKey9:
      {
        sendIRCode(codeNecKey9);
        tvGuessChannelNumber(9);
        break;
      }
      case codeNecChannelUp:
      {
        tv->channelNumber++;
        break;
      }
      case codeNecChannelDown:
      {
        tv->channelNumber--;
        break;
      }
      case codeSamsungSimpleVolumeUp:
      {
        break;
      }
      case codeSamsungSimpleVolumeDown:
      {
        break;
      }
      case codeNecKey0:
      {
        tvGuessChannelNumber(0);
        break;
      }
      case codeNecKey1:
      {
        tvGuessChannelNumber(1);
        break;
      }
      case codeNecKey2:
      {
        tvGuessChannelNumber(2);
        break;
      }
      case codeNecKey3:
      {
        tvGuessChannelNumber(3);
        break;
      }
      case codeNecKey4:
      {
        tvGuessChannelNumber(4);
        break;
      }
      case codeNecKey5:
      {
        tvGuessChannelNumber(5);
        break;
      }
      case codeNecKey6:
      {
        tvGuessChannelNumber(6);
        break;
      }
      case codeNecKey7:
      {
        tvGuessChannelNumber(7);
        break;
      }
      case codeNecKey8:
      {
        tvGuessChannelNumber(8);
        break;
      }
      case codeNecKey9:
      {
        tvGuessChannelNumber(9);
        break;
      }
    }
    isKeyCombination(codeValueReceived);
    irrecv.enableIRIn();
  }
}

void loop() {
  wifiClient->reconnectIfNeeded();
  RemotePrint::instance()->handle();
  fotaClient->loop();
  mqttClient->loop();
  tvSetVolumeIfNeeded();
  tvPublishStatus();
  temperatureClient->loop();
  irLoop();
}
