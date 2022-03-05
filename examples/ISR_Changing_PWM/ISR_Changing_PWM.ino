/****************************************************************************************************************************
  ISR_Changing_PWM.ino
  For ESP32, ESP32_S2, ESP32_S3, ESP32_C3 boards with ESP32 core v2.0.0+
  Written by Khoi Hoang

  Built by Khoi Hoang https://github.com/khoih-prog/ESP32_PWM
  Licensed under MIT license

  The ESP32, ESP32_S2, ESP32_S3, ESP32_C3 have two timer groups, TIMER_GROUP_0 and TIMER_GROUP_1
  1) each group of ESP32, ESP32_S2, ESP32_S3 has two general purpose hardware timers, TIMER_0 and TIMER_1
  2) each group of ESP32_C3 has ony one general purpose hardware timer, TIMER_0
  
  All the timers are based on 64-bit counters (except 54-bit counter for ESP32_S3 counter) and 16 bit prescalers. 
  The timer counters can be configured to count up or down and support automatic reload and software reload. 
  They can also generate alarms when they reach a specific value, defined by the software. 
  The value of the counter can be read by the software program.

  Now even you use all these new 16 ISR-based timers,with their maximum interval practically unlimited (limited only by
  unsigned long miliseconds), you just consume only one ESP32-S2 timer and avoid conflicting with other cores' tasks.
  The accuracy is nearly perfect compared to software timers. The most important feature is they're ISR-based timers
  Therefore, their executions are not blocked by bad-behaving functions / tasks.
  This important feature is absolutely necessary for mission-critical tasks.
*****************************************************************************************************************************/

#if !defined( ESP32 )
  #error This code is designed to run on ESP32 platform, not Arduino nor ESP8266! Please check your Tools->Board setting.
#endif

// These define's must be placed at the beginning before #include "ESP32_PWM.h"
// _PWM_LOGLEVEL_ from 0 to 4
// Don't define _PWM_LOGLEVEL_ > 0. Only for special ISR debugging only. Can hang the system.
#define _PWM_LOGLEVEL_                3

#define USING_MICROS_RESOLUTION       true    //false

// Default is true, uncomment to false
//#define CHANGING_PWM_END_OF_CYCLE     false

// To be included only in main(), .ino with setup() to avoid `Multiple Definitions` Linker Error
#include "ESP32_PWM.h"

#ifndef LED_BUILTIN
  #define LED_BUILTIN       2
#endif

#define HW_TIMER_INTERVAL_US      20L

volatile uint32_t startMicros = 0;

// Init ESP32 timer 1
ESP32Timer ITimer(1);

// Init ESP32_ISR_PWM
ESP32_PWM ISR_PWM;

bool IRAM_ATTR TimerHandler(void * timerNo)
{
  ISR_PWM.run();

  return true;
}

//////////////////////////////////////////////////////

#define USING_PWM_FREQUENCY     true

//////////////////////////////////////////////////////

// You can assign pins here. Be carefull to select good pin to use or crash
uint32_t PWM_Pin    = LED_BUILTIN;

// You can assign any interval for any timer here, in Hz
float PWM_Freq1   = 100.0f;
// You can assign any interval for any timer here, in Hz
float PWM_Freq2   = 200.0f;

// You can assign any interval for any timer here, in microseconds
uint32_t PWM_Period1 = 1000000 / PWM_Freq1;
// You can assign any interval for any timer here, in microseconds
uint32_t PWM_Period2 = 1000000 / PWM_Freq2;

// You can assign any duty_cycle for any PWM here, from 0-100
float PWM_DutyCycle1  = 50.0;
// You can assign any duty_cycle for any PWM here, from 0-100
float PWM_DutyCycle2  = 90.0;

// Channel number used to identify associated channel
int channelNum;

////////////////////////////////////////////////

void setup()
{
  Serial.begin(115200);
  while (!Serial);

  delay(2000);

  Serial.print(F("\nStarting ISR_Changing_PWM on ")); Serial.println(ARDUINO_BOARD);
  Serial.println(ESP32_PWM_VERSION);
  Serial.print(F("CPU Frequency = ")); Serial.print(F_CPU / 1000000); Serial.println(F(" MHz"));

  // Interval in microsecs
  if (ITimer.attachInterruptInterval(HW_TIMER_INTERVAL_US, TimerHandler))
  {
    startMicros = micros();
    Serial.print(F("Starting ITimer OK, micros() = ")); Serial.println(startMicros);
  }
  else
    Serial.println(F("Can't set ITimer. Select another freq. or timer"));

  // Just to demonstrate, don't use too many ISR Timers if not absolutely necessary
  // You can use up to 16 timer for each ISR_PWM
  //void setPWM(uint32_t pin, uint32_t frequency, uint32_t dutycycle
  // , timer_callback_p StartCallback = nullptr, timer_callback_p StopCallback = nullptr)
}

void loop()
{
  Serial.print(F("Using PWM Freq = ")); Serial.print(PWM_Freq1); Serial.print(F(", PWM DutyCycle = ")); Serial.println(PWM_DutyCycle1);

#if USING_PWM_FREQUENCY

  // You can use this with PWM_Freq in Hz
  channelNum = ISR_PWM.setPWM(PWM_Pin, PWM_Freq1, PWM_DutyCycle1);

#else
#if USING_MICROS_RESOLUTION
  // Or using period in microsecs resolution
  channelNum = ISR_PWM.setPWM_Period(PWM_Pin, PWM_Period1, PWM_DutyCycle1);
#else
  // Or using period in millisecs resolution
  channelNum = ISR_PWM.setPWM_Period(PWM_Pin, PWM_Period1 / 1000, PWM_DutyCycle1);
#endif
#endif

  delay(10000);

  ISR_PWM.deleteChannel((uint8_t) channelNum);

  Serial.print(F("Using PWM Freq = ")); Serial.print(PWM_Freq2); Serial.print(F(", PWM DutyCycle = ")); Serial.println(PWM_DutyCycle2);

#if USING_PWM_FREQUENCY

  // You can use this with PWM_Freq in Hz
  channelNum = ISR_PWM.setPWM(PWM_Pin, PWM_Freq2, PWM_DutyCycle2);

#else
#if USING_MICROS_RESOLUTION
  // Or using period in microsecs resolution
  channelNum = ISR_PWM.setPWM_Period(PWM_Pin, PWM_Period2, PWM_DutyCycle2);
#else
  // Or using period in millisecs resolution
  channelNum = ISR_PWM.setPWM_Period(PWM_Pin, PWM_Period2 / 1000, PWM_DutyCycle2);
#endif
#endif

  delay(10000);

  ISR_PWM.deleteChannel((uint8_t) channelNum);
}
