#include "TV.h"
#include "Arduino.h"

TV::TV() {
  this->powerOn        = false;
  this->channelNumber  = 0;
  this->muted          = false;
  this->volume         = -1;
  this->volumeDesired  = -1;
  this->playbackAction = "";
};

TV::TV(bool         powerOn,
       unsigned int channelNumber,
       bool         muted,
       int          volume,
       int          volumeDesired,
       String       playbackAction) {
  this->powerOn        = powerOn;
  this->channelNumber  = channelNumber;
  this->muted          = muted;
  this->volume         = volume;
  this->volumeDesired  = volumeDesired;
  this->playbackAction = playbackAction;
};
