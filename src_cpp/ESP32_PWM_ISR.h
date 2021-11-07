/****************************************************************************************************************************
  ESP32_PWM_ISR.h
  For ESP32, ESP32_S2, ESP32_C3 boards with ESP32 core v2.0.0+
  Written by Khoi Hoang

  Built by Khoi Hoang https://github.com/khoih-prog/ESP32_PWM
  Licensed under MIT license

  The ESP32, ESP32_S2, ESP32_C3 have two timer groups, TIMER_GROUP_0 and TIMER_GROUP_1
  1) each group of ESP32, ESP32_S2 has two general purpose hardware timers, TIMER_0 and TIMER_1
  2) each group of ESP32_C3 has ony one general purpose hardware timer, TIMER_0
  
  All the timers are based on 64 bits counters and 16 bit prescalers. The timer counters can be configured to count up or down 
  and support automatic reload and software reload. They can also generate alarms when they reach a specific value, defined by 
  the software. The value of the counter can be read by the software program.

  Now even you use all these new 16 ISR-based timers,with their maximum interval practically unlimited (limited only by
  unsigned long miliseconds), you just consume only one ESP32-S2 timer and avoid conflicting with other cores' tasks.
  The accuracy is nearly perfect compared to software timers. The most important feature is they're ISR-based timers
  Therefore, their executions are not blocked by bad-behaving functions / tasks.
  This important feature is absolutely necessary for mission-critical tasks.

  Version: 1.1.0

  Version Modified By   Date      Comments
  ------- -----------  ---------- -----------
  1.0.0   K Hoang      20/09/2021 Initial coding for ESP32, ESP32_S2, ESP32_C3 boards with ESP32 core v2.0.0+
  1.0.1   K Hoang      21/09/2021 Fix bug. Ading PWM end-of-duty-cycle callback function. Improve examples
  1.1.0   K Hoang      06/11/2021 Add functions to modify PWM settings on-the-fly
*****************************************************************************************************************************/

#pragma once

#ifndef PWM_ISR_GENERIC_H
#define PWM_ISR_GENERIC_H

#if !defined( ESP32 )
  #error This code is designed to run on ESP32 platform, not Arduino nor ESP8266! Please check your Tools->Board setting.
#elif ( ARDUINO_ESP32C3_DEV )
  #error This code is not ready to run on ESP32-C3 platform! Please check your Tools->Board setting.     
#endif

#ifndef ESP32_PWM_VERSION
  #define ESP32_PWM_VERSION       "ESP32_PWM v1.1.0"
#endif

#ifndef _PWM_LOGLEVEL_
  #define _PWM_LOGLEVEL_       1
#endif

#include "PWM_Generic_Debug.h"

#define CONFIG_ESP32_APPTRACE_ENABLE

#if 0
  #ifndef configMINIMAL_STACK_SIZE
    #define configMINIMAL_STACK_SIZE    2048
  #else
    #undef configMINIMAL_STACK_SIZE
    #define configMINIMAL_STACK_SIZE    2048
  #endif
#endif

#include <stddef.h>

#include <inttypes.h>

#if defined(ARDUINO)
  #if ARDUINO >= 100
    #include <Arduino.h>
  #else
    #include <WProgram.h>
  #endif
#endif

#define ESP32_PWM_ISR ESP32_PWM

typedef void (*timer_callback)();
typedef void (*timer_callback_p)(void *);

#if !defined(USING_MICROS_RESOLUTION)
  #warning Not USING_MICROS_RESOLUTION, using millis resolution
  #define USING_MICROS_RESOLUTION       false
#endif

class ESP32_PWM_ISR 
{

  public:
    // maximum number of PWM channels
#define MAX_NUMBER_CHANNELS        16

    // constructor
    ESP32_PWM_ISR();

    void init();

    // this function must be called inside loop()
    void IRAM_ATTR run();
    
    //////////////////////////////////////////////////////////////////
    // PWM
    // Return the channelNum if OK, -1 if error
    int setPWM(uint32_t pin, double frequency, uint32_t dutycycle, timer_callback StartCallback = nullptr, 
                timer_callback StopCallback = nullptr)
    {
      uint32_t period = 0;
      
      if ( ( frequency > 0 ) && ( frequency <= 500 ) )
      {
#if USING_MICROS_RESOLUTION
      // period in us
      period = 1000000.0f / frequency;
#else    
      // period in ms
      period = 1000.0f / frequency;
#endif
      }
      else
      {       
        PWM_LOGERROR("Error: Invalid frequency, max is 500Hz");
        
        return -1;
      }
      
      return setupPWMChannel(pin, period, dutycycle, (void *) StartCallback, (void *) StopCallback);      
    }

    // period in us
    // Return the channelNum if OK, -1 if error
    int setPWM_Period(uint32_t pin, uint32_t period, uint32_t dutycycle, timer_callback StartCallback = nullptr,
                       timer_callback StopCallback = nullptr)  
    {     
      return setupPWMChannel(pin, period, dutycycle, (void *) StartCallback, (void *) StopCallback);      
    } 
    
    //////////////////////////////////////////////////////////////////
    
    // low level function to modify a PWM channel
    // returns the true on success or false on failure
    bool modifyPWMChannel(unsigned channelNum, uint32_t pin, double frequency, uint32_t dutycycle)
    {
      uint32_t period = 0;
      
      if ( ( frequency > 0 ) && ( frequency <= 500 ) )
      {
#if USING_MICROS_RESOLUTION
      // period in us
      period = 1000000.0f / frequency;
#else    
      // period in ms
      period = 1000.0f / frequency;
#endif
      }
      else
      {       
        PWM_LOGERROR("Error: Invalid frequency, max is 500Hz");
        return false;
      }
      
      return modifyPWMChannel_Period(channelNum, pin, period, dutycycle);
    }
    
    //period in us
    bool modifyPWMChannel_Period(unsigned channelNum, uint32_t pin, uint32_t period, uint32_t dutycycle);

    // destroy the specified PWM channel
    void deleteChannel(unsigned channelNum);

    // restart the specified PWM channel
    void restartChannel(unsigned channelNum);

    // returns true if the specified PWM channel is enabled
    bool isEnabled(unsigned channelNum);

    // enables the specified PWM channel
    void enable(unsigned channelNum);

    // disables the specified PWM channel
    void disable(unsigned channelNum);

    // enables all PWM channels
    void enableAll();

    // disables all PWM channels
    void disableAll();

    // enables the specified PWM channel if it's currently disabled, and vice-versa
    void toggle(unsigned channelNum);

    // returns the number of used PWM channels
    unsigned getnumChannels();

    // returns the number of available PWM channels
    unsigned getNumAvailablePWMChannels() 
    {
      return MAX_NUMBER_CHANNELS - numChannels;
    };

  private:

    // low level function to initialize and enable a new PWM channel
    // returns the PWM channel number (channelNum) on success or
    // -1 on failure (f == NULL) or no free PWM channels 
    int setupPWMChannel(uint32_t pin, uint32_t period, uint32_t dutycycle, void* cbStartFunc = nullptr, void* cbStopFunc = nullptr);

    // find the first available slot
    int findFirstFreeSlot();

    typedef struct 
    {
      ///////////////////////////////////
      
      
      ///////////////////////////////////
      
      uint64_t      prevTime;           // value returned by the micros() or millis() function in the previous run() call
      uint32_t      period;             // period value, in us / ms
      uint32_t      onTime;             // onTime value, ( period * dutyCycle / 100 ) us  / ms
      
      void*         callbackStart;      // pointer to the callback function when PWM pulse starts (HIGH)
      void*         callbackStop;       // pointer to the callback function when PWM pulse stops (LOW)
      
      ////////////////////////////////////////////////////////////
      
      uint32_t      pin;                // PWM pin
      bool          pinHigh;            // true if PWM pin is HIGH
      ////////////////////////////////////////////////////////////
      
      bool          enabled;            // true if enabled
    } PWM_t;

    volatile PWM_t PWM[MAX_NUMBER_CHANNELS];

    // actual number of PWM channels in use (-1 means uninitialized)
    volatile int numChannels;

    // ESP32 is a multi core / multi processing chip. It is mandatory to disable task switches during ISR
    portMUX_TYPE PWM_Mux = portMUX_INITIALIZER_UNLOCKED;
};


#endif    // PWM_ISR_GENERIC_H


