#include "GameCube.cpp"

GameCube gc;

void setup() {
  Serial.begin(115200);
  Serial.println("ready");
  gc.init(8, -1, -1, -1);
  delay(10);
}

void loop() {
  delay(10);  
  gc.update();
  

}
