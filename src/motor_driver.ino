#include "motor_driver.h"

/*  ############## IMPORTANT #################
 *  A watchdog timer is used to make sure that
 *  the speedCommand is called at a fast enough
 *  rate so that the Actuator does not move past
 *  set points. This timer is enabled when
 *  MotorDriver.enable() is called. After this
 *  speedCommand needs to be called at least every
 *  250ms otherwise the mirco will reset. This is
 *  turned off when MotorDriver.disable() is called
*/

MotorDriver::MotorDriver(float p, float i, float d){
  k_p = p;
  k_i = i;
  k_d = d;

  velBuffer = RollingAverage();
  posBuffer = RollingAverage();
  
}

int MotorDriver::init(){
  // Setup motor driver outputs
  pinMode(INA, OUTPUT);
  digitalWrite(INA, LOW);
  pinMode(ENAB, OUTPUT);
  digitalWrite(ENAB, LOW);
  pinMode(INB, OUTPUT);
  digitalWrite(INB, LOW);
  pinMode(PWM, OUTPUT);
  digitalWrite(PWM, LOW);

  //
  enabled = false;

  // Setup potentiometer read pin
  pinMode(POS_PIN, INPUT);

  rawPos = analogRead(POS_PIN);
  prevRawPos = rawPos;
  micro = micros();
  prevMicro = micro;

  return prevRawPos;
}

void MotorDriver::enable(){
  if(enabled == false){
    digitalWrite(ENAB, HIGH);
    wdt_enable(WD_TIME);
    enabled = true; 
  }
}

void MotorDriver::disable(){
  if(enabled == true){
    digitalWrite(ENAB, LOW);
    wdt_disable();
    enabled = false;
  }
}

bool MotorDriver::status(){
  return enabled;
}

int MotorDriver::speedCommand(float velocityCommand){
  // reset watchdog timer
  wdt_reset();

  // get the current position
  rawPos = analogRead(POS_PIN);
  micro = micros();

  // micro overflow handling
  if (micro <= prevMicro){
    prevRawPos = rawPos;
    prevMicro = micro;
    rawPos = analogRead(POS_PIN);
    micro = micros();
  }

  float rawVel = (float)(rawPos - prevRawPos)/((float)(micro - prevMicro)/1000000.0);
  velBuffer.addItem((rawVel/1023.0)*200.0);
  

  // if it is past either of the set limits
  if(rawPos > UPPER_LIMIT | rawPos < LOWER_LIMIT){
    velocityCommand = 0;
  }
  if(velocityCommand > 0){
    // Move in a positive direction if
    // not in the upper stop buffer
    if(rawPos < UPPER_LIMIT - STOP_BUFFER){
      digitalWrite(INA, LOW);
      digitalWrite(INB, HIGH);
    }
    else
    {
      digitalWrite(INA, LOW);
      digitalWrite(INB, LOW);
    }
  }
  else if(velocityCommand < 0){
    // Move in a negative direction if
    // not in the lower stop buffer
    if(rawPos > LOWER_LIMIT + STOP_BUFFER){
      digitalWrite(INA, HIGH);
      digitalWrite(INB, LOW);
    }
    else {
      digitalWrite(INA, LOW);
      digitalWrite(INB, LOW);
    }
  }
  else{
    // Break to ground
    digitalWrite(INA, LOW);
    digitalWrite(INB, LOW);
  }

  int speedCommand = (abs(velocityCommand/100*255));

  if (speedCommand > 255){
    speedCommand = 255;
  }
  else if (speedCommand < 0){
    speedCommand = 0;
  }
  analogWrite(PWM, abs(speedCommand));

  return rawPos;
}

int MotorDriver::posCommand(uint16_t posCommand){
  // reset watchdog timer
  wdt_reset();

  if(posCommand > UPPER_LIMIT - STOP_BUFFER){
    posCommand = UPPER_LIMIT - STOP_BUFFER;
  }
  if(posCommand < LOWER_LIMIT + STOP_BUFFER){
    posCommand = LOWER_LIMIT + STOP_BUFFER;
  }

  // get the current position
//  rawPos = (int)posBuffer.average((float)analogRead(POS_PIN));
  int tempSum = 0;
  for(int i = 0; i < AVG_NUM; i++){
    tempSum += analogRead(POS_PIN);
  }
  rawPos = (int)(tempSum/AVG_NUM);
//  rawPos = analogRead(POS_PIN);
  int error = posCommand - rawPos;
//  Serial.print(", err, ");
//  Serial.print(error);
  int speedCommand = error * 5;
//  Serial.print(", spd, ");
//  Serial.print(speedCommand);
  

  // if it is past either of the set limits
  if(rawPos > UPPER_LIMIT | rawPos < LOWER_LIMIT){
    speedCommand = 0;
  }
  if(speedCommand > 0){
    // Move in a positive direction if
    // not in the upper stop buffer
    if(rawPos < UPPER_LIMIT - STOP_BUFFER){
      digitalWrite(INA, LOW);
      digitalWrite(INB, HIGH);
    }
    else
    {
      digitalWrite(INA, LOW);
      digitalWrite(INB, LOW);
    }
  }
  else if(speedCommand < 0){
    // Move in a negative direction if
    // not in the lower stop buffer
    if(rawPos > LOWER_LIMIT + STOP_BUFFER){
      digitalWrite(INA, HIGH);
      digitalWrite(INB, LOW);
    }
    else {
      digitalWrite(INA, LOW);
      digitalWrite(INB, LOW);
    }
  }
  else{
    // Break to ground
    digitalWrite(INA, LOW);
    digitalWrite(INB, LOW);
  }

  speedCommand = abs(speedCommand);
  if (speedCommand > 255){
    speedCommand = 255;
  }
  else if (speedCommand <= 0){
    speedCommand = 0;
  }
  else if (speedCommand < MIN_VEL){
    speedCommand = MIN_VEL;
  }
//  Serial.print(", spd, ");
//  Serial.print(speedCommand);
  analogWrite(PWM, speedCommand);
//  Serial.print(", pos: ");
  return rawPos;
}


RollingAverage::RollingAverage(){
  for (int i=0; i < arrLength; i++){
    arr[i] = 0;
  }
}
float RollingAverage::average(float newVal){
  addItem(newVal);
  float sum = 0;
//  Serial.print("Array: ");
  for(int i=0; i < arrLength; i++){
//    Serial.print(i);
//    Serial.print(":");
//    Serial.print(arr[i]);
//    Serial.print(" ");
    sum += arr[i];
  }
//  Serial.print(" Sum: ");
//  Serial.println(sum);
//  Serial.print("Average: ");
//  Serial.println(sum/(float)arrLength);
  return sum/(float)arrLength;
}
float RollingAverage::average(){
  float sum = 0;
  for(int i=0; i < arrLength; i++){
    sum += arr[i];
  }
  return sum/(float)arrLength;
}
void RollingAverage::addItem(float newVal){
  arr[oldestVal] = newVal;
  oldestVal ++;
  if(oldestVal >= arrLength){
    oldestVal = 0;
  }
}
