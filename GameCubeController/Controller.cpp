#include "Arduino.h"

class Controller{
public:
  int pin;
  int id;
  bool enabled;    //Is the controller even able to connect (is the arduino pin connected to port)
  bool connected;  //Is the controller currently connected
  byte data[8];
  bool rumble;
public:
  Controller(){
    pin = -1;
    id = -1;
    enabled = false; 
    rumble = false;
    connected = false;
  }

  Controller(int p, int i){
    pin = p;
    id = i;
    enabled = p!=-1; 
    rumble = false;
    connected = false;
  }

};
