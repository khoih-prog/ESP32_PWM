/****************************************************************************************************************************
  PWM_GENERIC_Debug.h
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

#ifndef PWM_GENERIC_DEBUG_H
#define PWM_GENERIC_DEBUG_H

#ifdef PWM_GENERIC_DEBUG_PORT
  #define PWM_DBG_PORT      PWM_GENERIC_DEBUG_PORT
#else
  #define PWM_DBG_PORT      Serial
#endif

// Change _PWM_LOGLEVEL_ to set tracing and logging verbosity
// 0: DISABLED: no logging
// 1: ERROR: errors
// 2: WARN: errors and warnings
// 3: INFO: errors, warnings and informational (default)
// 4: DEBUG: errors, warnings, informational and debug

#ifndef _PWM_LOGLEVEL_
  #define _PWM_LOGLEVEL_       1
#endif

///////////////////////////////////////

const char PWM_MARK[]  = "[PWM] ";
const char PWM_SPACE[] = " ";

#define PWM_PRINT        PWM_DBG_PORT.print
#define PWM_PRINTLN      PWM_DBG_PORT.println

#define PWM_PRINT_MARK   PWM_PRINT(PWM_MARK)
#define PWM_PRINT_SP     PWM_PRINT(PWM_SPACE)
#define PWM_PRINT_LINE   PWM_PRINT(PWM_LINE)

///////////////////////////////////////

#define PWM_LOGERROR(x)         if(_PWM_LOGLEVEL_>0) { PWM_PRINT_MARK; PWM_PRINTLN(x); }
#define PWM_LOGERROR0(x)        if(_PWM_LOGLEVEL_>0) { PWM_PRINT(x); }
#define PWM_LOGERRORLN0(x)      if(_PWM_LOGLEVEL_>0) { PWM_PRINTLN(x); }
#define PWM_LOGERROR1(x,y)      if(_PWM_LOGLEVEL_>0) { PWM_PRINT_MARK; PWM_PRINT(x); PWM_PRINT_SP; PWM_PRINTLN(y); }
#define PWM_LOGERROR2(x,y,z)    if(_PWM_LOGLEVEL_>0) { PWM_PRINT_MARK; PWM_PRINT(x); PWM_PRINT_SP; PWM_PRINT(y); PWM_PRINT_SP; PWM_PRINTLN(z); }
#define PWM_LOGERROR3(x,y,z,w)  if(_PWM_LOGLEVEL_>0) { PWM_PRINT_MARK; PWM_PRINT(x); PWM_PRINT_SP; PWM_PRINT(y); PWM_PRINT_SP; PWM_PRINT(z); PWM_PRINT_SP; PWM_PRINTLN(w); }

///////////////////////////////////////

#define PWM_LOGWARN(x)          if(_PWM_LOGLEVEL_>1) { PWM_PRINT_MARK; PWM_PRINTLN(x); }
#define PWM_LOGWARN0(x)         if(_PWM_LOGLEVEL_>1) { PWM_PRINT(x); }
#define PWM_LOGWARNLN0(x)       if(_PWM_LOGLEVEL_>1) { PWM_PRINTLN(x); }
#define PWM_LOGWARN1(x,y)       if(_PWM_LOGLEVEL_>1) { PWM_PRINT_MARK; PWM_PRINT(x); PWM_PRINT_SP; PWM_PRINTLN(y); }
#define PWM_LOGWARN2(x,y,z)     if(_PWM_LOGLEVEL_>1) { PWM_PRINT_MARK; PWM_PRINT(x); PWM_PRINT_SP; PWM_PRINT(y); PWM_PRINT_SP; PWM_PRINTLN(z); }
#define PWM_LOGWARN3(x,y,z,w)   if(_PWM_LOGLEVEL_>1) { PWM_PRINT_MARK; PWM_PRINT(x); PWM_PRINT_SP; PWM_PRINT(y); PWM_PRINT_SP; PWM_PRINT(z); PWM_PRINT_SP; PWM_PRINTLN(w); }

///////////////////////////////////////

#define PWM_LOGINFO(x)          if(_PWM_LOGLEVEL_>2) { PWM_PRINT_MARK; PWM_PRINTLN(x); }
#define PWM_LOGINFO0(x)         if(_PWM_LOGLEVEL_>2) { PWM_PRINT(x); }
#define PWM_LOGINFOLN0(x)       if(_PWM_LOGLEVEL_>2) { PWM_PRINTLN(x); }
#define PWM_LOGINFO1(x,y)       if(_PWM_LOGLEVEL_>2) { PWM_PRINT_MARK; PWM_PRINT(x); PWM_PRINT_SP; PWM_PRINTLN(y); }
#define PWM_LOGINFO2(x,y,z)     if(_PWM_LOGLEVEL_>2) { PWM_PRINT_MARK; PWM_PRINT(x); PWM_PRINT_SP; PWM_PRINT(y); PWM_PRINT_SP; PWM_PRINTLN(z); }
#define PWM_LOGINFO3(x,y,z,w)   if(_PWM_LOGLEVEL_>2) { PWM_PRINT_MARK; PWM_PRINT(x); PWM_PRINT_SP; PWM_PRINT(y); PWM_PRINT_SP; PWM_PRINT(z); PWM_PRINT_SP; PWM_PRINTLN(w); }

///////////////////////////////////////

#define PWM_LOGDEBUG(x)         if(_PWM_LOGLEVEL_>3) { PWM_PRINT_MARK; PWM_PRINTLN(x); }
#define PWM_LOGDEBUG0(x)        if(_PWM_LOGLEVEL_>3) { PWM_PRINT(x); }
#define PWM_LOGDEBUGLN0(x)      if(_PWM_LOGLEVEL_>3) { PWM_PRINTLN(x); }
#define PWM_LOGDEBUG1(x,y)      if(_PWM_LOGLEVEL_>3) { PWM_PRINT_MARK; PWM_PRINT(x); PWM_PRINT_SP; PWM_PRINTLN(y); }
#define PWM_LOGDEBUG2(x,y,z)    if(_PWM_LOGLEVEL_>3) { PWM_PRINT_MARK; PWM_PRINT(x); PWM_PRINT_SP; PWM_PRINT(y); PWM_PRINT_SP; PWM_PRINTLN(z); }
#define PWM_LOGDEBUG3(x,y,z,w)  if(_PWM_LOGLEVEL_>3) { PWM_PRINT_MARK; PWM_PRINT(x); PWM_PRINT_SP; PWM_PRINT(y); PWM_PRINT_SP; PWM_PRINT(z); PWM_PRINT_SP; PWM_PRINTLN(w); }

///////////////////////////////////////

#endif    //PWM_GENERIC_DEBUG_H
