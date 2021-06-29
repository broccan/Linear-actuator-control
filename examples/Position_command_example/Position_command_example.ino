#include "motor_driver.h"

MotorDriver motorDriver;

int pos = 0;

void setup() {
  Serial.begin(9600);
  motorDriver.init();

}


void loop() {
  if(Serial.available()){
    pos = Serial.parseInt();
  }
  motorDriver.posCommand(pos);
}
