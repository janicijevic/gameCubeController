//GameCube module
#include "Controller.cpp"

//10 counts of TCNT0 means 40us
#define TIMEOUT 100

class GameCube{
private:
  byte pins[4];
  bool enabled[4] = {true, true, true, true};
  
  Controller *conts[4];
    
public:
  GameCube(){
  }
  void init(int p1, int p2, int p3, int p4){
    //Initializes ports and calculates masks for each of the four controllers
    //TODO:
    //  make it run on all ports
    //  only clear necessary pins ie dont clear entire port
    PORTB = 0;                //Clear PORTB state
    DDRB = 0;                 //Make PORTB input
    MCUSR |= _BV(PUD);        //Disable internal pullups
    int _pins[4] = {p1, p2, p3, p4};
    int enabledCount = 0;
    for(int i = 0; i<4; i++){ //Disable controller port if pin is -1(not connected)
      if(_pins[i] < 0 || _pins[i] > 12){
        this->enabled[i] = false;
        this->pins[i] = -1;
        this->conts[i] = new Controller(-1, -1);
      }
      else{
        this->pins[i] = _pins[i];
        this->conts[i] = new Controller(_pins[i], enabledCount);
        enabledCount++;
      }
      Serial.println(conts[i]->id);
    }
  }

  void update(){
    //Update all enabled controllers
    for(int i = 0; i<4; i++){
      update(i);
    }
  }

  
  void update(int i){
    //Update i-th controller if enabled
    if(conts[i]->enabled){
      Serial.println(conts[i]->pin);
      if(probe(conts[i])){
        conts[i]->connected = true;
        delay(2);
        if(request(conts[i])){
          sendData(conts[i]);
        }
        else{
          conts[i]->connected = false;
        } 
      }
      else{
        conts[i]->connected = false;
      }
    }
  }
  

  bool probe(Controller *c){
    //Send probe signal to controller and check if it responds, 
    //returns true if controller responds
    int p = c->pin;
    byte a = 0;
    byte mask = digitalPinToBitMask(p);
    byte portNum = digitalPinToPort(p);
    volatile byte *port = portModeRegister(portNum);  //Data direction register
    volatile byte *in = portInputRegister(portNum);
    
    asm("cli");

    //Send probe signal -> 0000 0000 1
    sendZero(port, mask);sendZero(port, mask);sendZero(port, mask);sendZero(port, mask);
    sendZero(port, mask);sendZero(port, mask);sendZero(port, mask);sendZero(port, mask);
    sendOne(port, mask);

    __builtin_avr_delay_cycles(7);  //Wait for controller to respond

    a = *in & mask; //check if controller pulled line to low

    asm("sei");
    c->connected = a==0;
    return a==0; //True if controller pulled low
  }



  bool request(Controller *c){
    //Sends specific signal to controller, reads its response and sends it over Serial
    int p = c->pin;
    byte numOfBits = 0, numOfBytes = 0;
    byte data[8];
    byte tmp = 0;
    int i = 0;
    byte mask = digitalPinToBitMask(p);
    byte portNum = digitalPinToPort(p);
    volatile byte *port = portModeRegister(portNum);
    volatile byte *in = portInputRegister(portNum);
    
    asm("cli");
//TODO
    //Send message
    //Read button to see whether rumble signal is sent
    /*if(PIND & 0x40){                  //No rumble
      digitalWrite(LED_BUILTIN, LOW);
      PORTD &= 0x7F;
      sendMessage(port, mask);
    }else{                            //With rumble
      digitalWrite(LED_BUILTIN, HIGH);
      PORTD |= 0x80;
      sendMessageRumble(port, mask);
    }*/

    //Send request message to get data
    sendMessage(port, mask);

    //Read response
    while(numOfBytes < 8){
      numOfBits = 0;
      while(numOfBits < 8){
        tmp = *in & mask;    //Get state of data pin
        TCNT0 = 0;                //Clear timer0
        while(tmp != 0){          //Wait for drop
          tmp = *in & mask;  //Get state of data pin
          if(TCNT0 > TIMEOUT){    //The timeout period has passed and no drop has been detected
            disconnected(c);       //Controller not responding to message
            return 0;
          }
        };                    
        __builtin_avr_delay_cycles(18);         //Wait ~1.5us
        data[numOfBytes] = (data[numOfBytes] << 1 ) + ((*in & mask)!=0);  //Append bit to data
        numOfBits++;             
        tmp = *in & mask;    //Get state of data pin
        TCNT0 = 0;                //Clear timer0       
        while(tmp == 0){          //Wait for rise    
          tmp = *in & mask;  //Get state of data pin
          if(TCNT0 > TIMEOUT){    //The timeout period has passed and no drop has been detected
            disconnected(c);
            return 0;
          }
        };                    
        __builtin_avr_delay_cycles(6);          //Wait for signal to stabilize
      }
      numOfBytes++;
    }

    asm("sei");

    //Save respone
    for(numOfBytes = 0; numOfBytes<8; numOfBytes++){
      c->data[numOfBytes] = data[numOfBytes];
    }
    return 1;
  }


  void sendData(Controller *c){
    int numOfBytes = 0, numOfBits = 0;
    Serial.print(char('A'+c->id));
    for(numOfBytes = 0; numOfBytes<8; numOfBytes++){
      for(numOfBits = 0; numOfBits<8; numOfBits++){
        Serial.print((c->data[numOfBytes] & (0x80 >> numOfBits)) ? "1": "0");
      }
    }
    Serial.print('-');
    Serial.println();
  }

  

  void disconnected(Controller *c){
    //Handles controller being unplugged while reading response
    asm("sei");
    c->connected = false;
    Serial.print("Controller on pin ");
    Serial.print(c->pin);
    Serial.println(" disconnected-");
  }

  
  void sendMessage(volatile byte* port, byte mask){
    //0100 0000 0000 0011 0000 0010 1
    sendZero(port, mask);sendOne(port, mask);sendZero(port, mask);sendZero(port, mask);
    sendZero(port, mask);sendZero(port, mask);sendZero(port, mask);sendZero(port, mask);
    sendZero(port, mask);sendZero(port, mask);sendZero(port, mask);sendZero(port, mask);
    sendZero(port, mask);sendZero(port, mask);sendOne(port, mask);sendOne(port, mask);
    sendZero(port, mask);sendZero(port, mask);sendZero(port, mask);sendZero(port, mask);
    sendZero(port, mask);sendZero(port, mask);sendOne(port, mask);sendZero(port, mask);
    sendStop(port, mask);
  }
  
  void sendMessageRumble(volatile byte* port, byte mask){
    //0100 0000 0000 0011 0000 0001 1
    sendZero(port, mask);sendOne(port, mask);sendZero(port, mask);sendZero(port, mask);
    sendZero(port, mask);sendZero(port, mask);sendZero(port, mask);sendZero(port, mask);
    sendZero(port, mask);sendZero(port, mask);sendZero(port, mask);sendZero(port, mask);
    sendZero(port, mask);sendZero(port, mask);sendOne(port, mask);sendOne(port, mask);
    sendZero(port, mask);sendZero(port, mask);sendZero(port, mask);sendZero(port, mask);
    sendZero(port, mask);sendZero(port, mask);sendZero(port, mask);sendOne(port, mask);
    sendStop(port, mask);
  }

  void sendStop(volatile byte* port, byte mask){
    //Send 1us low, 2us high  _---
    //DDRB |= masks[p];
    *port |= mask;
    __builtin_avr_delay_cycles(14);
    //DDRB &= 0b1111111 ^ masks[p];
    //1110111 0111
    //DDRB &= 0b11111111 ^ masks[p];
    *port &= ~mask;
    __builtin_avr_delay_cycles(28);
    return;
  }
  void sendOne(volatile byte* port, byte mask){
    //Send 1us low, 3us high  _---
    *port |= mask;
    __builtin_avr_delay_cycles(14);
    //DDRB &= 0b1111110;
    *port &= ~mask;
    __builtin_avr_delay_cycles(44);
    return;
  }
  void sendZero(volatile byte* port, byte mask){
    //Send 3us low, 1us high  ___-
    *port |= mask;
    __builtin_avr_delay_cycles(45);
    *port &= ~mask;
    __builtin_avr_delay_cycles(12);
    return;
  }

    
  
};
