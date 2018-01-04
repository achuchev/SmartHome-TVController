#ifndef TV_H
#define TV_H
#include <Arduino.h>

class TV {
public:

  TV();
  TV(bool         powerOn,
     unsigned int channelNumber,
     bool         muted,
     int          volume,
     int          volumeDesired,
     String       playbackAction);

  bool powerOn;
  unsigned int channelNumber;
  bool muted;
  int volume;
  int volumeDesired;
  String playbackAction;
};
#endif // ifndef TV_H
