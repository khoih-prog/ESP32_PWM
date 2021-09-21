# ESP32_PWM Library

[![arduino-library-badge](https://www.ardu-badge.com/badge/ESP32_PWM.svg?)](https://www.ardu-badge.com/ESP32_PWM)
[![GitHub release](https://img.shields.io/github/release/khoih-prog/ESP32_PWM.svg)](https://github.com/khoih-prog/ESP32_PWM/releases)
[![GitHub](https://img.shields.io/github/license/mashape/apistatus.svg)](https://github.com/khoih-prog/ESP32_PWM/blob/master/LICENSE)
[![contributions welcome](https://img.shields.io/badge/contributions-welcome-brightgreen.svg?style=flat)](#Contributing)
[![GitHub issues](https://img.shields.io/github/issues/khoih-prog/ESP32_PWM.svg)](http://github.com/khoih-prog/ESP32_PWM/issues)

---
---

## Table of Contents

* [Why do we need this ESP32_PWM library](#why-do-we-need-this-ESP32_PWM-library)
  * [Features](#features)
  * [Why using ISR-based PWM-channels is better](#Why-using-ISR-based-PWM-channels-is-better)
  * [Currently supported Boards](#currently-supported-boards)
  * [Important Notes about ISR](#important-notes-about-isr)
* [Prerequisites](#prerequisites)
* [Installation](#installation)
  * [Use Arduino Library Manager](#use-arduino-library-manager)
  * [Manual Install](#manual-install)
  * [VS Code & PlatformIO](#vs-code--platformio)
* [HOWTO Fix `Multiple Definitions` Linker Error](#howto-fix-multiple-definitions-linker-error)
* [HOWTO Use analogRead() with ESP32 running WiFi and/or BlueTooth (BT/BLE)](#howto-use-analogread-with-esp32-running-wifi-andor-bluetooth-btble)
  * [1. ESP32 has 2 ADCs, named ADC1 and ADC2](#1--esp32-has-2-adcs-named-adc1-and-adc2)
  * [2. ESP32 ADCs functions](#2-esp32-adcs-functions)
  * [3. ESP32 WiFi uses ADC2 for WiFi functions](#3-esp32-wifi-uses-adc2-for-wifi-functions)
* [More useful Information](#more-useful-information)
* [How to use](#how-to-use)
* [Examples](#examples)
  * [  1. ISR_16_PWMs_Array](examples/ISR_16_PWMs_Array)
  * [  2. ISR_16_PWMs_Array_Complex](examples/ISR_16_PWMs_Array_Complex)
  * [  3. ISR_16_PWMs_Array_Simple](examples/ISR_16_PWMs_Array_Simple)
* [Example ISR_16_PWMs_Array_Complex](#Example-ISR_16_PWMs_Array_Complex)
* [Debug Terminal Output Samples](#debug-terminal-output-samples)
  * [1. ISR_16_PWMs_Array_Complex on ESP32_DEV](#1-ISR_16_PWMs_Array_Complex-on-ESP32_DEV)
  * [2. ISR_16_PWMs_Array on ESP32_DEV](#2-ISR_16_PWMs_Array-on-ESP32_DEV)
  * [3. ISR_16_PWMs_Array_Simple on ESP32_DEV](#3-ISR_16_PWMs_Array_Simple-on-ESP32_DEV)
* [Debug](#debug)
* [Troubleshooting](#troubleshooting)
* [Issues](#issues)
* [TO DO](#to-do)
* [DONE](#done)
* [Contributions and Thanks](#contributions-and-thanks)
* [Contributing](#contributing)
* [License](#license)
* [Copyright](#copyright)

---
---

### Why do we need this [ESP32_PWM library](https://github.com/khoih-prog/ESP32_PWM)

## Features

This library enables you to use Interrupt from Hardware Timers on an ESP32, ESP32_S2-based board to create and output PWM to pins.

---

This library enables you to use Interrupt from Hardware Timers on an ESP32, ESP32_S2 or ESP32_C3-based board to create and output PWM to pins. It now supports 16 ISR-based synchronized PWM channels, while consuming only 1 Hardware Timer. PWM interval can be very long (uint32_t millisecs). The most important feature is they're ISR-based PWM channels. Therefore, their executions are not blocked by bad-behaving functions or tasks. This important feature is absolutely necessary for mission-critical tasks. These hardware PWM channels, using interrupt, still work even if other functions are blocking. Moreover, they are much more precise (certainly depending on clock frequency accuracy) than other software timers using millis() or micros(). That's necessary if you need to measure some data requiring better accuracy.

As **Hardware Timers are rare, and very precious assets** of any board, this library now enables you to use up to **16 ISR-based synchronized PWM channels, while consuming only 1 Hardware Timer**. Timers' interval is very long (**ulong millisecs**).

Now with these new **16 ISR-based timers**, the maximum interval is **practically unlimited** (limited only by unsigned long miliseconds) while **the accuracy is nearly perfect** compared to software timers. 

The most important feature is they're ISR-based PWM channels. Therefore, their executions are **not blocked by bad-behaving functions / tasks**. This important feature is absolutely necessary for mission-critical tasks. 

The [**ISR_16_PWMs_Array_Complex**](examples/ISR_16_PWMs_Array_Complex) example will demonstrate the nearly perfect accuracy compared to software timers by printing the actual elapsed millisecs of each type of PWM channels.

Being ISR-based timers, their executions are not blocked by bad-behaving functions / tasks, such as connecting to WiFi, Internet and Blynk services. You can also have many `(up to 16)` timers to use.

This non-being-blocked important feature is absolutely necessary for mission-critical tasks.


### Why using ISR-based PWM-channels is better

Imagine you have a system with a **mission-critical** function, measuring water level and control the sump pump or doing something much more important. You normally use a software timer to poll, or even place the function in loop(). But what if another function is **blocking** the loop() or setup().

So your function **might not be executed, and the result would be disastrous.**

You'd prefer to have your function called, no matter what happening with other functions (busy loop, bug, etc.).

The correct choice is to use a Hardware PWM-channels with **Interrupt** to call your function.

These hardware timers, using interrupt, still work even if other functions are blocking. Moreover, they are much more **precise** (certainly depending on clock frequency accuracy) than other software timers using millis() or micros(). That's necessary if you need to measure some data requiring better accuracy.

Functions using normal software timers, relying on loop() and calling millis(), won't work if the loop() or setup() is blocked by certain operation. For example, certain function is blocking while it's connecting to WiFi or some services.

The catch is **your function is now part of an ISR (Interrupt Service Routine), and must be lean / mean, and follow certain rules.** More to read on:

[**HOWTO Attach Interrupt**](https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/)

---

### Currently supported Boards

1. ESP32 boards, such as ESP32_DEV, etc.
2. ESP32S2-based boards, such as ESP32S2_DEV, ESP32_S2 Saola, etc.
3. ESP32C3-based boards, such as ESP32C3_DEV, etc. **Not yet supported yet**

---

### Important Notes about ISR

1. Inside the attached function, **delay() won’t work and the value returned by millis() will not increment.** Serial data received while in the function may be lost. You should declare as **volatile any variables that you modify within the attached function.**

2. Typically global variables are used to pass data between an ISR and the main program. To make sure variables shared between an ISR and the main program are updated correctly, declare them as volatile.


---
---

## Prerequisites

1. [`Arduino IDE 1.8.16+` for Arduino](https://www.arduino.cc/en/Main/Software)
2. [`ESP32 Core 2.0.0+`](https://github.com/espressif/arduino-esp32) for ESP32-based boards. [![Latest release](https://img.shields.io/github/release/espressif/arduino-esp32.svg)](https://github.com/espressif/arduino-esp32/releases/latest/)
---
---

## Installation

### Use Arduino Library Manager

The best and easiest way is to use `Arduino Library Manager`. Search for [**ESP32_PWM**](https://github.com/khoih-prog/ESP32_PWM), then select / install the latest version.
You can also use this link [![arduino-library-badge](https://www.ardu-badge.com/badge/ESP32_PWM.svg?)](https://www.ardu-badge.com/ESP32_PWM) for more detailed instructions.

### Manual Install

Another way to install is to:

1. Navigate to [**ESP32_PWM**](https://github.com/khoih-prog/ESP32_PWM) page.
2. Download the latest release `ESP32_PWM-master.zip`.
3. Extract the zip file to `ESP32_PWM-master` directory 
4. Copy whole `ESP32_PWM-master` folder to Arduino libraries' directory such as `~/Arduino/libraries/`.

### VS Code & PlatformIO

1. Install [VS Code](https://code.visualstudio.com/)
2. Install [PlatformIO](https://platformio.org/platformio-ide)
3. Install [**ESP32_PWM** library](https://platformio.org/lib/show/12846/ESP32_PWM) by using [Library Manager](https://platformio.org/lib/show/12846/ESP32_PWM/installation). Search for **ESP32_PWM** in [Platform.io Author's Libraries](https://platformio.org/lib/search?query=author:%22Khoi%20Hoang%22)
4. Use included [platformio.ini](platformio/platformio.ini) file from examples to ensure that all dependent libraries will installed automatically. Please visit documentation for the other options and examples at [Project Configuration File](https://docs.platformio.org/page/projectconf.html)


---
---


### HOWTO Fix `Multiple Definitions` Linker Error

The current library implementation, using **xyz-Impl.h instead of standard xyz.cpp**, possibly creates certain `Multiple Definitions` Linker error in certain use cases. Although it's simple to just modify several lines of code, either in the library or in the application, the library is adding 2 more source directories

1. **scr_h** for new h-only files
2. **src_cpp** for standard h/cpp files

besides the standard **src** directory.

To use the **old standard cpp** way, locate this library' directory, then just 

1. **Delete the all the files in src directory.**
2. **Copy all the files in src_cpp directory into src.**
3. Close then reopen the application code in Arduino IDE, etc. to recompile from scratch.

To re-use the **new h-only** way, just 

1. **Delete the all the files in src directory.**
2. **Copy the files in src_h directory into src.**
3. Close then reopen the application code in Arduino IDE, etc. to recompile from scratch.

---
---

### HOWTO Use analogRead() with ESP32 running WiFi and/or BlueTooth (BT/BLE)

Please have a look at [**ESP_WiFiManager Issue 39: Not able to read analog port when using the autoconnect example**](https://github.com/khoih-prog/ESP_WiFiManager/issues/39) to have more detailed description and solution of the issue.

#### 1.  ESP32 has 2 ADCs, named ADC1 and ADC2

#### 2. ESP32 ADCs functions

- ADC1 controls ADC function for pins **GPIO32-GPIO39**
- ADC2 controls ADC function for pins **GPIO0, 2, 4, 12-15, 25-27**

#### 3.. ESP32 WiFi uses ADC2 for WiFi functions

Look in file [**adc_common.c**](https://github.com/espressif/esp-idf/blob/master/components/driver/adc_common.c#L61)

> In ADC2, there're two locks used for different cases:
> 1. lock shared with app and Wi-Fi:
>    ESP32:
>         When Wi-Fi using the ADC2, we assume it will never stop, so app checks the lock and returns immediately if failed.
>    ESP32S2:
>         The controller's control over the ADC is determined by the arbiter. There is no need to control by lock.
> 
> 2. lock shared between tasks:
>    when several tasks sharing the ADC2, we want to guarantee
>    all the requests will be handled.
>    Since conversions are short (about 31us), app returns the lock very soon,
>    we use a spinlock to stand there waiting to do conversions one by one.
> 
> adc2_spinlock should be acquired first, then adc2_wifi_lock or rtc_spinlock.


- In order to use ADC2 for other functions, we have to **acquire complicated firmware locks and very difficult to do**
- So, it's not advisable to use ADC2 with WiFi/BlueTooth (BT/BLE).
- Use ADC1, and pins GPIO32-GPIO39
- If somehow it's a must to use those pins serviced by ADC2 (**GPIO0, 2, 4, 12, 13, 14, 15, 25, 26 and 27**), use the **fix mentioned at the end** of [**ESP_WiFiManager Issue 39: Not able to read analog port when using the autoconnect example**](https://github.com/khoih-prog/ESP_WiFiManager/issues/39) to work with ESP32 WiFi/BlueTooth (BT/BLE).

---
---

## More useful Information

### ESP32 Hardware Timers

  - **The ESP32 and ESP32_S2 has two timer groups, each one with two general purpose hardware timers.**
  - **The ESP32_C3 has two timer groups, each one with only one general purpose hardware timer.**
  - All the timers are based on **64-bit counters and 16-bit prescalers.**
  - The timer counters can be configured to count up or down and support automatic reload and software reload.
  - They can also generate alarms when they reach a specific value, defined by the software. 
  - The value of the counter can be read by the software program.

---

Now with these new `16 ISR-based PWM-channels` (while consuming only **1 hardware timer**), the maximum interval is practically unlimited (limited only by unsigned long miliseconds). The accuracy is nearly perfect compared to software PWM-channels. The most important feature is they're ISR-based PWM-channels Therefore, their executions are not blocked by bad-behaving functions / tasks.
This important feature is absolutely necessary for mission-critical tasks. 

The [**ISR_16_PWMs_Array_Complex**](examples/ISR_16_PWMs_Array_Complex) example will demonstrate the nearly perfect accuracy compared to software-based PWM-channels by printing the actual elapsed `microsecs / millisecs` of each type of PWM-channels.
Being ISR-based PWM-channels, their executions are not blocked by bad-behaving functions / tasks, such as connecting to WiFi, Internet and Blynk services. You can also have many `(up to 16)` synchronized PWM-channels to use.
This non-being-blocked important feature is absolutely necessary for mission-critical tasks. 
You'll see `SimpleTimer` is blocked while system is connecting to WiFi / Internet / Blynk, as well as by blocking task 
in loop(), using delay() function as an example. The elapsed time then is very unaccurate

---
---

## How to use

Before using any Timer, you have to make sure the Timer has not been used by any other purpose.

`Timer0, Timer1, Timer2 and Timer3` are supported for ESP32

---
---

### Examples: 

 1. [ISR_16_PWMs_Array](examples/ISR_16_PWMs_Array)
 2. [ISR_16_PWMs_Array_Complex](examples/ISR_16_PWMs_Array_Complex)
 3. [ISR_16_PWMs_Array_Simple](examples/ISR_16_PWMs_Array_Simple) 

---
---

### Example [ISR_16_PWMs_Array_Complex](examples/ISR_16_PWMs_Array_Complex)

```
#if !defined( ESP32 )
  #error This code is designed to run on ESP32 platform, not Arduino nor ESP8266! Please check your Tools->Board setting.
#elif ( ARDUINO_ESP32C3_DEV )
  #error This code is not designed to run on ESP32-C3 platform! Please check your Tools->Board setting.     
#endif

// These define's must be placed at the beginning before #include "ESP32_PWM.h"
// _PWM_LOGLEVEL_ from 0 to 4
// Don't define _PWM_LOGLEVEL_ > 0. Only for special ISR debugging only. Can hang the system.
#define _PWM_LOGLEVEL_      4

#define USING_MICROS_RESOLUTION       true    //false 

#include "ESP32_PWM.h"

#include <SimpleTimer.h>              // https://github.com/jfturcot/SimpleTimer

#ifndef LED_BUILTIN
  #define LED_BUILTIN       2
#endif

#ifndef LED_BLUE
  #define LED_BLUE          25
#endif

#ifndef LED_RED
  #define LED_RED           27
#endif

#define HW_TIMER_INTERVAL_US      20L

volatile uint32_t startMillis = 0;

// Init ESP32 timer 1
ESP32Timer ITimer(1);

// Init ESP32_ISR_PWM
ESP32_PWM ISR_PWM;

bool IRAM_ATTR TimerHandler(void * timerNo)
{ 
  ISR_PWM.run();
 
  return true;
}

/////////////////////////////////////////////////

#define NUMBER_ISR_PWMS         16

#define PIN_D0            0         // Pin D0 mapped to pin GPIO0/BOOT/ADC11/TOUCH1 of ESP32
#define PIN_D1            1         // Pin D1 mapped to pin GPIO1/TX0 of ESP32
#define PIN_D2            2         // Pin D2 mapped to pin GPIO2/ADC12/TOUCH2 of ESP32
#define PIN_D3            3         // Pin D3 mapped to pin GPIO3/RX0 of ESP32
#define PIN_D4            4         // Pin D4 mapped to pin GPIO4/ADC10/TOUCH0 of ESP32
#define PIN_D5            5         // Pin D5 mapped to pin GPIO5/SPISS/VSPI_SS of ESP32
#define PIN_D12           12        // Pin D12 mapped to pin GPIO12/HSPI_MISO/ADC15/TOUCH5/TDI of ESP32
#define PIN_D13           13        // Pin D13 mapped to pin GPIO13/HSPI_MOSI/ADC14/TOUCH4/TCK of ESP32
#define PIN_D14           14        // Pin D14 mapped to pin GPIO14/HSPI_SCK/ADC16/TOUCH6/TMS of ESP32
#define PIN_D15           15        // Pin D15 mapped to pin GPIO15/HSPI_SS/ADC13/TOUCH3/TDO of ESP32
#define PIN_D16           16        // Pin D16 mapped to pin GPIO16/TX2 of ESP32
#define PIN_D17           17        // Pin D17 mapped to pin GPIO17/RX2 of ESP32     
#define PIN_D18           18        // Pin D18 mapped to pin GPIO18/VSPI_SCK of ESP32
#define PIN_D19           19        // Pin D19 mapped to pin GPIO19/VSPI_MISO of ESP32

#define PIN_D21           21        // Pin D21 mapped to pin GPIO21/SDA of ESP32
#define PIN_D22           22        // Pin D22 mapped to pin GPIO22/SCL of ESP32
#define PIN_D23           23        // Pin D23 mapped to pin GPIO23/VSPI_MOSI of ESP32
#define PIN_D24           24        // Pin D24 mapped to pin GPIO24 of ESP32
#define PIN_D25           25        // Pin D25 mapped to pin GPIO25/ADC18/DAC1 of ESP32
#define PIN_D26           26        // Pin D26 mapped to pin GPIO26/ADC19/DAC2 of ESP32
#define PIN_D27           27        // Pin D27 mapped to pin GPIO27/ADC17/TOUCH7 of ESP32   

typedef void (*irqCallback)  ();

//////////////////////////////////////////////////////

#define USE_COMPLEX_STRUCT      true

#define USING_PWM_FREQUENCY     false //true

//////////////////////////////////////////////////////

#if USE_COMPLEX_STRUCT

typedef struct
{
  uint32_t      PWM_Pin;
  irqCallback   irqCallbackFunc;

#if USING_PWM_FREQUENCY  
  uint32_t      PWM_Freq;
#else  
  uint32_t      PWM_Period;
#endif
  
  uint32_t      PWM_DutyCycle;
  unsigned long deltaMillis;
  unsigned long previousMillis;
} ISR_PWM_Data;

// In NRF52, avoid doing something fancy in ISR, for example Serial.print()
// The pure simple Serial.prints here are just for demonstration and testing. Must be eliminate in working environment
// Or you can get this run-time error / crash

void doingSomething(int index);

#else   // #if USE_COMPLEX_STRUCT

volatile unsigned long deltaMillis    [NUMBER_ISR_PWMS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
volatile unsigned long previousMillis [NUMBER_ISR_PWMS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

// You can assign pins here. Be carefull to select good pin to use or crash, e.g pin 6-11
uint32_t PWM_Pin[NUMBER_ISR_PWMS] =
{
   PIN_D1,  LED_BUILTIN,  PIN_D3,  PIN_D4,  PIN_D5,  PIN_D12, PIN_D13, PIN_D14,
   PIN_D15, PIN_D16,      PIN_D17, PIN_D18, PIN_D19, PIN_D21, PIN_D22, PIN_D23
};

// You can assign any interval for any timer here, in microseconds
uint32_t PWM_Period[NUMBER_ISR_PWMS] =
{
  1000000L,   500000L,   333333L,   250000L,   200000L,   166667L,   142857L,   125000L,
   111111L,   100000L,    66667L,    50000L,    40000L,   33333L,     25000L,    20000L
};


// You can assign any interval for any timer here, in Hz
uint32_t PWM_Freq[NUMBER_ISR_PWMS] =
{
  1,  2,  3,  4,  5,  6,  7,  8,
  9, 10, 15, 20, 25, 30, 40, 50
};

// You can assign any interval for any timer here, in milliseconds
uint32_t PWM_DutyCycle[NUMBER_ISR_PWMS] =
{
   5, 10, 20, 30, 40, 45, 50, 55,
  60, 65, 70, 75, 80, 85, 90, 95
};

void doingSomething(int index)
{
  unsigned long currentMillis  = millis();

  deltaMillis[index]    = currentMillis - previousMillis[index];
  previousMillis[index] = currentMillis;
}

#endif    // #if USE_COMPLEX_STRUCT

////////////////////////////////////
// Shared
////////////////////////////////////

void doingSomething0()
{
  doingSomething(0);
}

void doingSomething1()
{
  doingSomething(1);
}

void doingSomething2()
{
  doingSomething(2);
}

void doingSomething3()
{
  doingSomething(3);
}

void doingSomething4()
{
  doingSomething(4);
}

void doingSomething5()
{
  doingSomething(5);
}

void doingSomething6()
{
  doingSomething(6);
}

void doingSomething7()
{
  doingSomething(7);
}

void doingSomething8()
{
  doingSomething(8);
}

void doingSomething9()
{
  doingSomething(9);
}

void doingSomething10()
{
  doingSomething(10);
}

void doingSomething11()
{
  doingSomething(11);
}

void doingSomething12()
{
  doingSomething(12);
}

void doingSomething13()
{
  doingSomething(13);
}

void doingSomething14()
{
  doingSomething(14);
}

void doingSomething15()
{
  doingSomething(15);
}

//////////////////////////////////////////////////////

#if USE_COMPLEX_STRUCT

  #if USING_PWM_FREQUENCY
  
  ISR_PWM_Data curISR_PWM_Data[NUMBER_ISR_PWMS] =
  {
    //irqCallbackFunc, PWM_Period, deltaMillis, previousMillis
    { PIN_D1,       doingSomething0,   1,   5, 0, 0 },
    { LED_BUILTIN,  doingSomething1,   2,  10, 0, 0 },
    { PIN_D3,       doingSomething2,   3,  20, 0, 0 },
    { PIN_D4,       doingSomething3,   4,  30, 0, 0 },
    { PIN_D5,       doingSomething4,   5,  40, 0, 0 },
    { PIN_D12,      doingSomething5,   6,  45, 0, 0 },
    { PIN_D13,      doingSomething6,   7,  50, 0, 0 },
    { PIN_D14,      doingSomething7,   8,  55, 0, 0 },
    { PIN_D15,      doingSomething8,   9,  60, 0, 0 },
    { PIN_D16,      doingSomething9,   10, 65, 0, 0 },
    { PIN_D17,      doingSomething10,  15, 70, 0, 0 },
    { PIN_D18,      doingSomething11,  20, 75, 0, 0 },
    { PIN_D19,      doingSomething12,  25, 80, 0, 0 },
    { PIN_D21,      doingSomething13,  30, 85, 0, 0 },
    { PIN_D22,      doingSomething14,  40, 90, 0, 0 },
    { PIN_D23,      doingSomething15,  50, 95, 0, 0 }
  };
  
  #else   // #if USING_PWM_FREQUENCY
  
  ISR_PWM_Data curISR_PWM_Data[NUMBER_ISR_PWMS] =
  {
    //irqCallbackFunc, PWM_Period, deltaMillis, previousMillis
    { PIN_D1,       doingSomething0,   1000000L,  5, 0, 0 },
    { LED_BUILTIN,  doingSomething1,    500000L, 10, 0, 0 },
    { PIN_D3,       doingSomething2,    333333L, 20, 0, 0 },
    { PIN_D4,       doingSomething3,    250000L, 30, 0, 0 },
    { PIN_D5,       doingSomething4,    200000L, 40, 0, 0 },
    { PIN_D12,      doingSomething5,    166667L, 45, 0, 0 },
    { PIN_D13,      doingSomething6,    142857L, 50, 0, 0 },
    { PIN_D14,      doingSomething7,    125000L, 55, 0, 0 },
    { PIN_D15,      doingSomething8,    111111L, 60, 0, 0 },
    { PIN_D16,      doingSomething9,    100000L, 65, 0, 0 },
    { PIN_D17,      doingSomething10,    66667L, 70, 0, 0 },
    { PIN_D18,      doingSomething11,    50000L, 75, 0, 0 },
    { PIN_D19,      doingSomething12,    40000L, 80, 0, 0 },
    { PIN_D21,      doingSomething13,    33333L, 85, 0, 0 },
    { PIN_D22,      doingSomething14,    25000L, 90, 0, 0 },
    { PIN_D23,      doingSomething15,    20000L, 95, 0, 0 }
  };
  
  #endif  // #if USING_PWM_FREQUENCY

void doingSomething(int index)
{
  unsigned long currentMillis  = millis();

  curISR_PWM_Data[index].deltaMillis    = currentMillis - curISR_PWM_Data[index].previousMillis;
  curISR_PWM_Data[index].previousMillis = currentMillis;
}

#else   // #if USE_COMPLEX_STRUCT

irqCallback irqCallbackFunc[NUMBER_ISR_PWMS] =
{
  doingSomething0,  doingSomething1,  doingSomething2,  doingSomething3,
  doingSomething4,  doingSomething5,  doingSomething6,  doingSomething7,
  doingSomething8,  doingSomething9,  doingSomething10, doingSomething11,
  doingSomething12, doingSomething13, doingSomething14, doingSomething15
};

#endif    // #if USE_COMPLEX_STRUCT

//////////////////////////////////////////////////////

#define SIMPLE_TIMER_MS        2000L

// Init SimpleTimer
SimpleTimer simpleTimer;

// Here is software Timer, you can do somewhat fancy stuffs without many issues.
// But always avoid
// 1. Long delay() it just doing nothing and pain-without-gain wasting CPU power.Plan and design your code / strategy ahead
// 2. Very long "do", "while", "for" loops without predetermined exit time.
void simpleTimerDoingSomething2s()
{
  static unsigned long previousMillis = startMillis;

  unsigned long currMillis = millis();

  Serial.print(F("SimpleTimer (ms): ")); Serial.print(SIMPLE_TIMER_MS);
  Serial.print(F(", ms : ")); Serial.print(currMillis);
  Serial.print(F(", Dms : ")); Serial.println(currMillis - previousMillis);

  for (uint16_t i = 0; i < NUMBER_ISR_PWMS; i++)
  {
#if USE_COMPLEX_STRUCT
    Serial.print(F("PWM Channel : ")); Serial.print(i);
    Serial.print(F(", programmed : ")); 

  #if USING_PWM_FREQUENCY
    Serial.print(1000 / curISR_PWM_Data[i].PWM_Freq);
  #else
    Serial.print(curISR_PWM_Data[i].PWM_Period);
  #endif
    
    Serial.print(F(", actual : ")); Serial.println(curISR_PWM_Data[i].deltaMillis);
    
#else
    Serial.print(F("PWM Channel : ")); Serial.print(i);
    
  #if USING_PWM_FREQUENCY
    Serial.print(1000 / PWM_Freq[i]);
  #else
    Serial.print(curISR_PWM_Data[i].PWM_Period);
  #endif
  
    Serial.print(F(", programmed : ")); Serial.print(PWM_Period[i]);
    Serial.print(F(", actual : ")); Serial.println(deltaMillis[i]);
#endif
  }

  previousMillis = currMillis;
}

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);
  while (!Serial);

  delay(2000);

  Serial.print(F("\nStarting ISR_16_PWMs_Array_Complex on ")); Serial.println(ARDUINO_BOARD);
  Serial.println(ESP32_PWM_VERSION);
  Serial.print(F("CPU Frequency = ")); Serial.print(F_CPU / 1000000); Serial.println(F(" MHz"));

  // Interval in microsecs
  if (ITimer.attachInterruptInterval(HW_TIMER_INTERVAL_US, TimerHandler))
  {
    startMillis = millis();
    Serial.print(F("Starting ITimer OK, millis() = ")); Serial.println(startMillis);
  }
  else
    Serial.println(F("Can't set ITimer. Select another freq. or timer"));

  startMillis = millis();

  // Just to demonstrate, don't use too many ISR Timers if not absolutely necessary
  // You can use up to 16 timer for each ISR_PWM
  
  for (uint16_t i = 0; i < NUMBER_ISR_PWMS; i++)
  {
#if USE_COMPLEX_STRUCT
    curISR_PWM_Data[i].previousMillis = startMillis;
    //ISR_PWM.setInterval(curISR_PWM_Data[i].PWM_Period, curISR_PWM_Data[i].irqCallbackFunc);

    //void setPWM(uint32_t pin, uint32_t frequency, uint32_t dutycycle
    // , timer_callback_p StartCallback = nullptr, timer_callback_p StopCallback = nullptr)

  #if USING_PWM_FREQUENCY
    // You can use this with PWM_Freq in Hz
    ISR_PWM.setPWM(curISR_PWM_Data[i].PWM_Pin, curISR_PWM_Data[i].PWM_Freq, curISR_PWM_Data[i].PWM_DutyCycle, curISR_PWM_Data[i].irqCallbackFunc);
  #else
    // Or You can use this with PWM_Period in us
    ISR_PWM.setPWM_Period(curISR_PWM_Data[i].PWM_Pin, curISR_PWM_Data[i].PWM_Period, curISR_PWM_Data[i].PWM_DutyCycle, curISR_PWM_Data[i].irqCallbackFunc);
  #endif
  
#else
    previousMillis[i] = millis();
    
  #if USING_PWM_FREQUENCY
    // You can use this with PWM_Freq in Hz
    ISR_PWM.setPWM(PWM_Pin[i], PWM_Freq[i], PWM_DutyCycle[i], irqCallbackFunc[i]);
  #else
    // Or You can use this with PWM_Period in us
    ISR_PWM.setPWM_Period(PWM_Pin[i], PWM_Period[i], PWM_DutyCycle[i], irqCallbackFunc[i]);
  #endif 
   
#endif
  }

  // You need this timer for non-critical tasks. Avoid abusing ISR if not absolutely necessary.
  simpleTimer.setInterval(SIMPLE_TIMER_MS, simpleTimerDoingSomething2s);
}

#define BLOCKING_TIME_MS      10000L

void loop()
{
  // This unadvised blocking task is used to demonstrate the blocking effects onto the execution and accuracy to Software timer
  // You see the time elapse of ISR_PWM still accurate, whereas very unaccurate for Software Timer
  // The time elapse for 2000ms software timer now becomes 3000ms (BLOCKING_TIME_MS)
  // While that of ISR_PWM is still prefect.
  delay(BLOCKING_TIME_MS);

  // You need this Software timer for non-critical tasks. Avoid abusing ISR if not absolutely necessary
  // You don't need to and never call ISR_PWM.run() here in the loop(). It's already handled by ISR timer.
  simpleTimer.run();
}
```
---
---

### Debug Terminal Output Samples

### 1. ISR_16_PWMs_Array_Complex on ESP32_DEV

The following is the sample terminal output when running example [ISR_16_PWMs_Array_Complex](examples/ISR_16_PWMs_Array_Complex) to demonstrate the accuracy of ISR Hardware PWM-channels, **especially when system is very busy**.  The ISR PWM-channels is **running exactly according to corresponding programmed periods**


```
Starting ISR_16_PWMs_Array_Complex on ESP32_DEV
ESP32_PWM v1.0.0
CPU Frequency = 240 MHz
[PWM] ESP32_TimerInterrupt: _timerNo = 1 , _fre = 1000000
[PWM] TIMER_BASE_CLK = 80000000 , TIMER_DIVIDER = 80
[PWM] _timerIndex = 1 , _timerGroup = 0
[PWM] _count = 0 - 20
[PWM] timer_set_alarm_value = 20.00
Starting ITimer OK, millis() = 2054
Channel : 0	Period : 1000000		OnTime : 50000	Start_Time : 2054323
Channel : 1	Period : 500000		OnTime : 50000	Start_Time : 2054323
Channel : 2	Period : 333333		OnTime : 66666	Start_Time : 2054323
Channel : 3	Period : 250000		OnTime : 75000	Start_Time : 2054323
Channel : 4	Period : 200000		OnTime : 80000	Start_Time : 2054323
Channel : 5	Period : 166666		OnTime : 74999	Start_Time : 2054323
Channel : 6	Period : 142857		OnTime : 71428	Start_Time : 2054323
Channel : 7	Period : 125000		OnTime : 68750	Start_Time : 2054323
Channel : 8	Period : 111111		OnTime : 66666	Start_Time : 2054323
Channel : 9	Period : 100000		OnTime : 65000	Start_Time : 2054323
Channel : 10	Period : 66666		OnTime : 46666	Start_Time : 2054323
Channel : 11	Period : 50000		OnTime : 37500	Start_Time : 2054323
Channel : 12	Period : 40000		OnTime : 32000	Start_Time : 2054323
Channel : 13	Period : 33333		OnTime : 28333	Start_Time : 2054323
Channel : 14	Period : 25000		OnTime : 22500	Start_Time : 2054323
Channel : 15	Period : 20000		OnTime : 19000	Start_Time : 2054323
SimpleTimer (ms): 2000, ms : 12150, Dms : 10096
PWM Channel : 0, programmed : 1000, actual : 1000
PWM Channel : 1, programmed : 500, actual : 500
PWM Channel : 2, programmed : 333, actual : 333
PWM Channel : 3, programmed : 250, actual : 250
PWM Channel : 4, programmed : 200, actual : 200
PWM Channel : 5, programmed : 166, actual : 167
PWM Channel : 6, programmed : 142, actual : 143
PWM Channel : 7, programmed : 125, actual : 125
PWM Channel : 8, programmed : 111, actual : 111
PWM Channel : 9, programmed : 100, actual : 100
PWM Channel : 10, programmed : 66, actual : 66
PWM Channel : 11, programmed : 50, actual : 50
PWM Channel : 12, programmed : 40, actual : 40
PWM Channel : 13, programmed : 33, actual : 33
PWM Channel : 14, programmed : 25, actual : 25
PWM Channel : 15, programmed : 20, actual : 20
```

---

### 2. ISR_16_PWMs_Array on ESP32_DEV

The following is the sample terminal output when running example [ISR_16_PWMs_Array](examples/ISR_16_PWMs_Array) to demonstrate how to use multiple Hardware PWM channels.

```
Starting ISR_16_PWMs_Array on ESP32_DEV
ESP32_PWM v1.0.0
CPU Frequency = 240 MHz
[PWM] ESP32_TimerInterrupt: _timerNo = 1 , _fre = 1000000
[PWM] TIMER_BASE_CLK = 80000000 , TIMER_DIVIDER = 80
[PWM] _timerIndex = 1 , _timerGroup = 0
[PWM] _count = 0 - 1000
[PWM] timer_set_alarm_value = 1000.00
Starting ITimer OK, millis() = 2054
Channel : 0	Period : 1000000		OnTime : 50000	Start_Time : 2054267
Channel : 1	Period : 200000		OnTime : 20000	Start_Time : 2054267
Channel : 2	Period : 333333		OnTime : 66666	Start_Time : 2054267
Channel : 3	Period : 250000		OnTime : 75000	Start_Time : 2054267
Channel : 4	Period : 200000		OnTime : 80000	Start_Time : 2054267
Channel : 5	Period : 166666		OnTime : 74999	Start_Time : 2054267
Channel : 6	Period : 142857		OnTime : 71428	Start_Time : 2054267
Channel : 7	Period : 125000		OnTime : 68750	Start_Time : 2054267
Channel : 8	Period : 111111		OnTime : 66666	Start_Time : 2054267
Channel : 9	Period : 100000		OnTime : 65000	Start_Time : 2054267
Channel : 10	Period : 66666		OnTime : 46666	Start_Time : 2054267
Channel : 11	Period : 50000		OnTime : 37500	Start_Time : 2054267
Channel : 12	Period : 40000		OnTime : 32000	Start_Time : 2054267
Channel : 13	Period : 33333		OnTime : 28333	Start_Time : 2054267
Channel : 14	Period : 25000		OnTime : 22500	Start_Time : 2054267
Channel : 15	Period : 20000		OnTime : 19000	Start_Time : 2054267
```

---


### 3. ISR_16_PWMs_Array_Simple on ESP32_DEV

The following is the sample terminal output when running example [ISR_16_PWMs_Array_Simple](examples/ISR_16_PWMs_Array_Simple) to demonstrate how to use multiple Hardware PWM channels.

```
Starting ISR_16_PWMs_Array_Simple on ESP32_DEV
ESP32_PWM v1.0.0
CPU Frequency = 240 MHz
[PWM] ESP32_TimerInterrupt: _timerNo = 1 , _fre = 1000000
[PWM] TIMER_BASE_CLK = 80000000 , TIMER_DIVIDER = 80
[PWM] _timerIndex = 1 , _timerGroup = 0
[PWM] _count = 0 - 20
[PWM] timer_set_alarm_value = 20.00
Starting ITimer OK, millis() = 2054
Channel : 0	Period : 1000000		OnTime : 50000	Start_Time : 2054360
Channel : 1	Period : 500000		OnTime : 50000	Start_Time : 2054360
Channel : 2	Period : 333333		OnTime : 66666	Start_Time : 2054360
Channel : 3	Period : 250000		OnTime : 75000	Start_Time : 2054360
Channel : 4	Period : 200000		OnTime : 80000	Start_Time : 2054360
Channel : 5	Period : 166666		OnTime : 74999	Start_Time : 2054360
Channel : 6	Period : 142857		OnTime : 71428	Start_Time : 2054360
Channel : 7	Period : 125000		OnTime : 68750	Start_Time : 2054360
Channel : 8	Period : 111111		OnTime : 66666	Start_Time : 2054360
Channel : 9	Period : 100000		OnTime : 65000	Start_Time : 2054360
Channel : 10	Period : 66666		OnTime : 46666	Start_Time : 2054360
Channel : 11	Period : 50000		OnTime : 37500	Start_Time : 2054360
Channel : 12	Period : 40000		OnTime : 32000	Start_Time : 2054360
Channel : 13	Period : 33333		OnTime : 28333	Start_Time : 2054360
Channel : 14	Period : 25000		OnTime : 22500	Start_Time : 2054360
Channel : 15	Period : 20000		OnTime : 19000	Start_Time : 2054360
```

---
---

### Debug

Debug is enabled by default on Serial.

You can also change the debugging level (_TIMERINTERRUPT_LOGLEVEL_) from 0 to 4

```cpp
// These define's must be placed at the beginning before #include "ESP32_PWM.h"
// _PWM_LOGLEVEL_ from 0 to 4
// Don't define _PWM_LOGLEVEL_ > 0. Only for special ISR debugging only. Can hang the system.
#define _PWM_LOGLEVEL_      4
```

---

### Troubleshooting

If you get compilation errors, more often than not, you may need to install a newer version of the core for Arduino boards.

Sometimes, the library will only work if you update the board core to the latest version because I am using newly added functions.


---
---

### Issues

Submit issues to: [ESP32_PWM issues](https://github.com/khoih-prog/ESP32_PWM/issues)

---

## TO DO

1. Search for bug and improvement.
2. Similar features for remaining Arduino boards such as SAMD21, SAMD51, SAM-DUE, nRF52, ESP8266, STM32, Portenta_H7, RP2040, etc.



## DONE

1. Basic hardware PWM-channels for ESP32 and ESP32-S2 for [ESP32 core v2.0.0+](https://github.com/espressif/arduino-esp32/releases/tag/2.0.0)
2. Longer time interval
3. Add complex examples.

---
---

### Contributions and Thanks

Many thanks for everyone for bug reporting, new feature suggesting, testing and contributing to the development of this library.


---

## Contributing

If you want to contribute to this project:
- Report bugs and errors
- Ask for enhancements
- Create issues and pull requests
- Tell other people about this library

---

### License

- The library is licensed under [MIT](https://github.com/khoih-prog/ESP32_PWM/blob/master/LICENSE)

---

## Copyright

Copyright 2021- Khoi Hoang


