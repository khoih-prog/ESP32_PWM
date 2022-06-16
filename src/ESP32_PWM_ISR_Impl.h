/****************************************************************************************************************************
  ESP32_PWM_ISR_Impl.h
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

  Version: 1.3.3

  Version Modified By   Date      Comments
  ------- -----------  ---------- -----------
  1.0.0   K Hoang      20/09/2021 Initial coding for ESP32, ESP32_S2, ESP32_C3 boards with ESP32 core v2.0.0+
  1.0.1   K Hoang      21/09/2021 Fix bug. Ading PWM end-of-duty-cycle callback function. Improve examples
  1.1.0   K Hoang      06/11/2021 Add functions to modify PWM settings on-the-fly
  1.1.1   K Hoang      09/11/2021 Fix examples to not use GPIO1/TX0 for core v2.0.1+
  1.2.0   K Hoang      29/01/2022 Fix multiple-definitions linker error. Improve accuracy. Fix bug
  1.2.1   K Hoang      30/01/2022 DutyCycle to be updated at the end current PWM period
  1.2.2   K Hoang      01/02/2022 Use float for DutyCycle and Freq, uint32_t for period. Optimize code
  1.3.0   K Hoang      12/02/2022 Add support to new ESP32-S3
  1.3.1   K Hoang      04/03/2022 Fix `DutyCycle` and `New Period` display bugs. Display warning only when debug level > 3
  1.3.2   K Hoang      09/05/2022 Remove crashing PIN_D24 from examples
  1.3.3   K Hoang      16/06/2022 Add support to new Adafruit boards
*****************************************************************************************************************************/

#pragma once

#ifndef PWM_ISR_GENERIC_IMPL_H
#define PWM_ISR_GENERIC_IMPL_H

#include <string.h>

/////////////////////////////////////////////////// 


uint64_t IRAM_ATTR timeNow()
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
    PWM[channelNum].pin      = INVALID_ESP32_PIN;
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
          
          // callbackStart
          if (PWM[channelNum].callbackStart != nullptr)
          {
            (*(timer_callback) PWM[channelNum].callbackStart)();
          }
        }
      }
      else if ( (uint32_t) (currentTime - PWM[channelNum].prevTime) < PWM[channelNum].period ) 
      {
        if (PWM[channelNum].pinHigh)
        {
          digitalWrite(PWM[channelNum].pin, LOW);
          PWM[channelNum].pinHigh = false;
          
          // callback when PWM pulse stops (LOW)
          if (PWM[channelNum].callbackStop != nullptr)
          {
            (*(timer_callback) PWM[channelNum].callbackStop)();
          }
        }
      }
      //else 
      else if ( (uint32_t) (currentTime - PWM[channelNum].prevTime) >= PWM[channelNum].period )   
      {
        PWM[channelNum].prevTime = currentTime;
        
#if CHANGING_PWM_END_OF_CYCLE
        // Only update whenever having newPeriod
        if (PWM[channelNum].newPeriod != 0)
        {
          PWM[channelNum].period    = PWM[channelNum].newPeriod;
          PWM[channelNum].newPeriod = 0;
          
          PWM[channelNum].onTime  = PWM[channelNum].newOnTime;
        }
#endif
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

  // return the first slot with no callbackStart (i.e. free)
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

int ESP32_PWM_ISR::setupPWMChannel(const uint32_t& pin, const uint32_t& period, const float& dutycycle, void* cbStartFunc, void* cbStopFunc)
{
  int channelNum;
  
  // Invalid input, such as period = 0, etc
  if ( (period == 0) || (dutycycle < 0.0) || (dutycycle > 100.0) )
  {
    PWM_LOGERROR("Error: Invalid period or dutycycle");
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
  
  // ESP32 is a multi core / multi processing chip. It is mandatory to disable task switches during modifying shared vars
  portENTER_CRITICAL(&PWM_Mux);

  PWM[channelNum].pin           = pin;
  PWM[channelNum].period        = period;
  
  // Must be 0 for new PWM channel
  PWM[channelNum].newPeriod     = 0;
  
  PWM[channelNum].onTime        = ( period * dutycycle ) / 100;
  
  pinMode(pin, OUTPUT);
  digitalWrite(pin, HIGH);
  PWM[channelNum].pinHigh       = true;
  
  PWM[channelNum].prevTime      = timeNow();
  
  PWM[channelNum].callbackStart = cbStartFunc;
  PWM[channelNum].callbackStop  = cbStopFunc;
  
  // ESP32 is a multi core / multi processing chip. It is mandatory to disable task switches during modifying shared vars
  portEXIT_CRITICAL(&PWM_Mux);
   
  PWM_LOGINFO0("Channel : ");      PWM_LOGINFO0(channelNum); 
  PWM_LOGINFO0("\t    Period : "); PWM_LOGINFO0(PWM[channelNum].period);
  PWM_LOGINFO0("\t\tOnTime : ");   PWM_LOGINFO0(PWM[channelNum].onTime); 
  PWM_LOGINFO0("\tStart_Time : "); PWM_LOGINFOLN0(PWM[channelNum].prevTime);
 
  numChannels++;
  
  PWM[channelNum].enabled      = true;
  
  return channelNum;
}

///////////////////////////////////////////////////

bool ESP32_PWM_ISR::modifyPWMChannel_Period(const uint8_t& channelNum, const uint32_t& pin, const uint32_t& period, const float& dutycycle)
{
  // Invalid input, such as period = 0, etc
  if ( (period == 0) || (dutycycle < 0.0) || (dutycycle > 100.0) )
  {
    PWM_LOGERROR("Error: Invalid period or dutycycle");
    return false;
  }

  if (channelNum > MAX_NUMBER_CHANNELS) 
  {
    PWM_LOGERROR("Error: channelNum > MAX_NUMBER_CHANNELS");
    return false;
  }
  
  if (PWM[channelNum].pin != pin) 
  {
    PWM_LOGERROR("Error: channelNum and pin mismatched");
    return false;
  }

  // ESP32 is a multi core / multi processing chip. It is mandatory to disable task switches during modifying shared vars
  portENTER_CRITICAL(&PWM_Mux);

#if CHANGING_PWM_END_OF_CYCLE

  PWM[channelNum].newPeriod     = period;
  PWM[channelNum].newDutyCycle  = dutycycle;
  PWM[channelNum].newOnTime     = ( period * dutycycle ) / 100;
  
  // ESP32 is a multi core / multi processing chip. It is mandatory to disable task switches during modifying shared vars
  portEXIT_CRITICAL(&PWM_Mux);
  
  PWM_LOGINFO0("Channel : ");      PWM_LOGINFO0(channelNum); 
  PWM_LOGINFO0("\t    Period : "); PWM_LOGINFO0(period);
  PWM_LOGINFO0("\t\tOnTime : ");   PWM_LOGINFO0(PWM[channelNum].newOnTime); 
  PWM_LOGINFO0("\tStart_Time : "); PWM_LOGINFOLN0(PWM[channelNum].prevTime);
  
#else
    
  PWM[channelNum].period        = period;
  PWM[channelNum].onTime        = ( period * dutycycle ) / 100;
  
  digitalWrite(pin, HIGH);
  PWM[channelNum].pinHigh       = true;
  
  PWM[channelNum].prevTime      = timeNow();
  
  // ESP32 is a multi core / multi processing chip. It is mandatory to disable task switches during modifying shared vars
  portEXIT_CRITICAL(&PWM_Mux);
   
  PWM_LOGINFO0("Channel : ");      PWM_LOGINFO0(channelNum); 
  PWM_LOGINFO0("\t    Period : "); PWM_LOGINFO0(PWM[channelNum].period);
  PWM_LOGINFO0("\t\tOnTime : ");   PWM_LOGINFO0(PWM[channelNum].onTime); 
  PWM_LOGINFO0("\tStart_Time : "); PWM_LOGINFOLN0(PWM[channelNum].prevTime);
  
#endif
  
  return true;
}


///////////////////////////////////////////////////

void ESP32_PWM_ISR::deleteChannel(const uint8_t& channelNum) 
{
  // nothing to delete if no timers are in use
  if ( (channelNum >= MAX_NUMBER_CHANNELS)  || (numChannels == 0) )
  {
    return;
  }

  // don't decrease the number of timers if the specified slot is already empty (zero period, invalid)
  if ( (PWM[channelNum].pin != INVALID_ESP32_PIN) && (PWM[channelNum].period != 0) )
  {
    // ESP32 is a multi core / multi processing chip. It is mandatory to disable task switches during modifying shared vars
    portENTER_CRITICAL(&PWM_Mux);

    memset((void*) &PWM[channelNum], 0, sizeof (PWM_t));
    
    PWM[channelNum].pin = INVALID_ESP32_PIN;
    
    // update number of timers
    numChannels--;

    // ESP32 is a multi core / multi processing chip. It is mandatory to disable task switches during modifying shared vars
    portEXIT_CRITICAL(&PWM_Mux);

  }
}

///////////////////////////////////////////////////

// function contributed by code@rowansimms.com
void ESP32_PWM_ISR::restartChannel(const uint8_t& channelNum) 
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

bool ESP32_PWM_ISR::isEnabled(const uint8_t& channelNum) 
{
  if (channelNum >= MAX_NUMBER_CHANNELS) 
  {
    return false;
  }

  return PWM[channelNum].enabled;
}

///////////////////////////////////////////////////

void ESP32_PWM_ISR::enable(const uint8_t& channelNum) 
{
  if (channelNum >= MAX_NUMBER_CHANNELS) 
  {
    return;
  }

  PWM[channelNum].enabled = true;
}

///////////////////////////////////////////////////

void ESP32_PWM_ISR::disable(const uint8_t& channelNum) 
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
  // Enable all timers with a callbackStart assigned (used)

  // ESP32 is a multi core / multi processing chip. It is mandatory to disable task switches during modifying shared vars
  portENTER_CRITICAL(&PWM_Mux);

  for (uint8_t channelNum = 0; channelNum < MAX_NUMBER_CHANNELS; channelNum++) 
  {
    if (PWM[channelNum].period != 0)
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
  // Disable all timers with a callbackStart assigned (used)

  // ESP32 is a multi core / multi processing chip. It is mandatory to disable task switches during modifying shared vars
  portENTER_CRITICAL(&PWM_Mux);

  for (uint8_t channelNum = 0; channelNum < MAX_NUMBER_CHANNELS; channelNum++) 
  {
    if (PWM[channelNum].period != 0)
    {
      PWM[channelNum].enabled = false;
    }
  }

  // ESP32 is a multi core / multi processing chip. It is mandatory to disable task switches during modifying shared vars
  portEXIT_CRITICAL(&PWM_Mux);

}

///////////////////////////////////////////////////

void ESP32_PWM_ISR::toggle(const uint8_t& channelNum) 
{
  if (channelNum >= MAX_NUMBER_CHANNELS) 
  {
    return;
  }

  PWM[channelNum].enabled = !PWM[channelNum].enabled;
}

///////////////////////////////////////////////////

int8_t ESP32_PWM_ISR::getnumChannels() 
{
  return numChannels;
}

#endif    // PWM_ISR_GENERIC_IMPL_H

