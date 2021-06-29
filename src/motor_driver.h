#ifndef MOTOR_DRIVER_H
#define MOTOR_DRIVER_H

#include <avr/wdt.h>  //Watchdog timer library

#define INA 4
#define ENAB 5
#define INB 7
#define PWM 6

#define STOP_BUFFER 0
#define UPPER_LIMIT 1023
#define LOWER_LIMIT 0

#define WD_TIME WDTO_250MS
#define D_BUFFER_LEN 10

#define POS_PIN A7

#define AVG_NUM 4
#define BUF_LEN 3

#define MIN_VEL 30

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

class RollingAverage{
  public:
    RollingAverage();
    float average(float newVal);
    float average();
    void addItem(float newVal);
  private:
    byte oldestVal = 0;
    byte arrLength = BUF_LEN;
    float arr[BUF_LEN];
};

class MotorDriver{
  public:
    MotorDriver(float p, float i, float d);
    int init();
    int speedCommand(float velocityCommand);
    int posCommand(uint16_t posCommand);
    bool status();
    void enable();
    void disable();
    unsigned long micro;
    int rawPos;
    
  private:
    float k_p;
    float k_i;
    float k_d;
    bool enabled;

    unsigned long prevMicro;
    int prevRawPos;
    RollingAverage velBuffer;
    RollingAverage posBuffer;
    
};


#endif
