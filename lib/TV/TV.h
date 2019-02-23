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

  bool powerOn               = false;
  unsigned int channelNumber = 0;
  bool muted                 = false;
  int volume                 = -1;
  int volumeDesired          = -1;
  String playbackAction      = "";
};
#endif // ifndef TV_H
