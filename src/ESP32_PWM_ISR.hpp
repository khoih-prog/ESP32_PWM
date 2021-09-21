/****************************************************************************************************************************
  ESP32_PWM_ISR.hpp
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

  Version: 1.0.0

  Version Modified By   Date      Comments
  ------- -----------  ---------- -----------
  1.0.0   K Hoang      20/09/2021 Initial coding for ESP32, ESP32_S2, ESP32_C3 boards with ESP32 core v2.0.0+
*****************************************************************************************************************************/

#pragma once

#ifndef PWM_ISR_GENERIC_HPP
#define PWM_ISR_GENERIC_HPP

#include <string.h>

/////////////////////////////////////////////////// 


uint64_t timeNow()
{
#if USING_MICROS_RESOLUTION  
  return ( (uint64_t) micros() );
#else
  return ( (uint64_t) millis() );
#endif    
}
  
/////////////////////////////////////////////////// 

ESP32_PWM_ISR::ESP32_PWM_ISR()
  : numChannels (-1)
{
}

///////////////////////////////////////////////////

void ESP32_PWM_ISR::init() 
{
  uint64_t currentTime = timeNow();
   
  for (uint8_t channelNum = 0; channelNum < MAX_NUMBER_CHANNELS; channelNum++) 
  {
    memset((void*) &PWM[channelNum], 0, sizeof (PWM_t));
    PWM[channelNum].prevTime = currentTime;
  }
  
  numChannels = 0;

  // ESP32 is a multi core / multi processing chip. It is mandatory to disable task switches during ISR
  PWM_Mux = portMUX_INITIALIZER_UNLOCKED;
}

///////////////////////////////////////////////////

void IRAM_ATTR ESP32_PWM_ISR::run() 
{ 
  // ESP32 is a multi core / multi processing chip. It is mandatory to disable task switches during ISR
  portENTER_CRITICAL_ISR(&PWM_Mux);
    
  uint64_t currentTime = timeNow();

  for (uint8_t channelNum = 0; channelNum < MAX_NUMBER_CHANNELS; channelNum++) 
  {
    // If enabled => check
    // start period / dutyCycle => digitalWrite HIGH
    // end dutyCycle =>  digitalWrite LOW
    if (PWM[channelNum].enabled) 
    {
      if ( (uint32_t) (currentTime - PWM[channelNum].prevTime) <= PWM[channelNum].onTime ) 
      {              
        if (!PWM[channelNum].pinHigh)
        {
          digitalWrite(PWM[channelNum].pin, HIGH);
          PWM[channelNum].pinHigh = true;
          
          // callback
          if (PWM[channelNum].callback != nullptr)
          {
            (*(timer_callback) PWM[channelNum].callback)();
          }
        }
      }
      else if ( (uint32_t) (currentTime - PWM[channelNum].prevTime) < PWM[channelNum].period ) 
      {
        if (PWM[channelNum].pinHigh)
        {
          digitalWrite(PWM[channelNum].pin, LOW);
          PWM[channelNum].pinHigh = false;
        }
      }
      //else 
      else if ( (uint32_t) (currentTime - PWM[channelNum].prevTime) >= PWM[channelNum].period )   
      {
        PWM[channelNum].prevTime = currentTime;
      }      
    }
  }

  // ESP32 is a multi core / multi processing chip. It is mandatory to disable task switches during ISR
  portEXIT_CRITICAL_ISR(&PWM_Mux);
}


///////////////////////////////////////////////////

// find the first available slot
// return -1 if none found
int ESP32_PWM_ISR::findFirstFreeSlot() 
{
  // all slots are used
  if (numChannels >= MAX_NUMBER_CHANNELS) 
  {
    return -1;
  }

  // return the first slot with no callback (i.e. free)
  for (uint8_t channelNum = 0; channelNum < MAX_NUMBER_CHANNELS; channelNum++) 
  {
    if ( (PWM[channelNum].period == 0) && !PWM[channelNum].enabled )
    {
      return channelNum;
    }
  }

  // no free slots found
  return -1;
}

///////////////////////////////////////////////////

int ESP32_PWM_ISR::setupPWMChannel(uint32_t pin, uint32_t period, uint32_t dutycycle, void* cbFunc) 
{
  int channelNum;
  
  // Invalid input, such as pin = 0, period = 0, etc
  if ( (pin == 0) || (period == 0) || (dutycycle > 100) )
  {
    PWM_LOGERROR("Error: Invalid pin, period or dutycycle");
    return -1;
  }

  if (numChannels < 0) 
  {
    init();
  }
 
  channelNum = findFirstFreeSlot();
  
  if (channelNum < 0) 
  {
    return -1;
  }

  PWM[channelNum].pin          = pin;
  PWM[channelNum].period       = period;
  PWM[channelNum].onTime       = ( period * dutycycle ) / 100;
  
  pinMode(pin, OUTPUT);
  digitalWrite(pin, HIGH);
  PWM[channelNum].pinHigh      = true;
  
  PWM[channelNum].callback     = cbFunc;
  
  PWM_LOGDEBUG0("Channel : "); PWM_LOGDEBUG0(channelNum); PWM_LOGDEBUG0("\tPeriod : "); PWM_LOGDEBUG0(PWM[channelNum].period);
  PWM_LOGDEBUG0("\t\tOnTime : "); PWM_LOGDEBUG0(PWM[channelNum].onTime); PWM_LOGDEBUG0("\tStart_Time : "); PWM_LOGDEBUGLN0(PWM[channelNum].prevTime);
 
  numChannels++;
  
  PWM[channelNum].enabled      = true;
  
  return channelNum;
}

///////////////////////////////////////////////////

void ESP32_PWM_ISR::deleteChannel(unsigned channelNum) 
{
  if (channelNum >= MAX_NUMBER_CHANNELS) 
  {
    return;
  }

  // nothing to delete if no timers are in use
  if (numChannels == 0) 
  {
    return;
  }

  // don't decrease the number of timers if the specified slot is already empty
  if (PWM[channelNum].callback != NULL) 
  {
    // ESP32 is a multi core / multi processing chip. It is mandatory to disable task switches during modifying shared vars
    portENTER_CRITICAL(&PWM_Mux);

    memset((void*) &PWM[channelNum], 0, sizeof (PWM_t));
    
    // update number of timers
    numChannels--;

    // ESP32 is a multi core / multi processing chip. It is mandatory to disable task switches during modifying shared vars
    portEXIT_CRITICAL(&PWM_Mux);

  }
}

///////////////////////////////////////////////////

// function contributed by code@rowansimms.com
void ESP32_PWM_ISR::restartChannel(unsigned channelNum) 
{
  if (channelNum >= MAX_NUMBER_CHANNELS) 
  {
    return;
  }

  // ESP32 is a multi core / multi processing chip. It is mandatory to disable task switches during modifying shared vars
  portENTER_CRITICAL(&PWM_Mux);

  PWM[channelNum].prevTime = timeNow();

  // ESP32 is a multi core / multi processing chip. It is mandatory to disable task switches during modifying shared vars
  portEXIT_CRITICAL(&PWM_Mux);
}

///////////////////////////////////////////////////

bool ESP32_PWM_ISR::isEnabled(unsigned channelNum) 
{
  if (channelNum >= MAX_NUMBER_CHANNELS) 
  {
    return false;
  }

  return PWM[channelNum].enabled;
}

///////////////////////////////////////////////////

void ESP32_PWM_ISR::enable(unsigned channelNum) 
{
  if (channelNum >= MAX_NUMBER_CHANNELS) 
  {
    return;
  }

  PWM[channelNum].enabled = true;
}

///////////////////////////////////////////////////

void ESP32_PWM_ISR::disable(unsigned channelNum) 
{
  if (channelNum >= MAX_NUMBER_CHANNELS) 
  {
    return;
  }

  PWM[channelNum].enabled = false;
}

///////////////////////////////////////////////////

void ESP32_PWM_ISR::enableAll() 
{
  // Enable all timers with a callback assigned (used)

  // ESP32 is a multi core / multi processing chip. It is mandatory to disable task switches during modifying shared vars
  portENTER_CRITICAL(&PWM_Mux);

  for (uint8_t channelNum = 0; channelNum < MAX_NUMBER_CHANNELS; channelNum++) 
  {
    if (PWM[channelNum].callback != NULL) 
    {
      PWM[channelNum].enabled = true;
    }
  }

  // ESP32 is a multi core / multi processing chip. It is mandatory to disable task switches during modifying shared vars
  portEXIT_CRITICAL(&PWM_Mux);
}

///////////////////////////////////////////////////

void ESP32_PWM_ISR::disableAll() 
{
  // Disable all timers with a callback assigned (used)

  // ESP32 is a multi core / multi processing chip. It is mandatory to disable task switches during modifying shared vars
  portENTER_CRITICAL(&PWM_Mux);

  for (uint8_t channelNum = 0; channelNum < MAX_NUMBER_CHANNELS; channelNum++) 
  {
    if (PWM[channelNum].callback != NULL) 
    {
      PWM[channelNum].enabled = false;
    }
  }

  // ESP32 is a multi core / multi processing chip. It is mandatory to disable task switches during modifying shared vars
  portEXIT_CRITICAL(&PWM_Mux);

}

///////////////////////////////////////////////////

void ESP32_PWM_ISR::toggle(unsigned channelNum) 
{
  if (channelNum >= MAX_NUMBER_CHANNELS) 
  {
    return;
  }

  PWM[channelNum].enabled = !PWM[channelNum].enabled;
}

///////////////////////////////////////////////////

unsigned ESP32_PWM_ISR::getnumChannels() 
{
  return numChannels;
}

#endif    // PWM_ISR_GENERIC_HPP

