/****************************************************************************************************************************
  ISR_16_PWMs_Array_Complex.ino
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
*****************************************************************************************************************************/

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
  irqCallback   irqCallbackStartFunc;
  irqCallback   irqCallbackStopFunc;

#if USING_PWM_FREQUENCY  
  uint32_t      PWM_Freq;
#else  
  uint32_t      PWM_Period;
#endif
  
  uint32_t      PWM_DutyCycle;
  
  uint64_t      deltaMicrosStart;
  uint64_t      previousMicrosStart;

  uint64_t      deltaMicrosStop;
  uint64_t      previousMicrosStop;
  
} ISR_PWM_Data;

// In NRF52, avoid doing something fancy in ISR, for example Serial.print()
// The pure simple Serial.prints here are just for demonstration and testing. Must be eliminate in working environment
// Or you can get this run-time error / crash

void doingSomethingStart(int index);

void doingSomethingStop(int index);

#else   // #if USE_COMPLEX_STRUCT

volatile unsigned long deltaMicrosStart    [NUMBER_ISR_PWMS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
volatile unsigned long previousMicrosStart [NUMBER_ISR_PWMS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

volatile unsigned long deltaMicrosStop     [NUMBER_ISR_PWMS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
volatile unsigned long previousMicrosStop  [NUMBER_ISR_PWMS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

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
double PWM_Freq[NUMBER_ISR_PWMS] =
{
  1.0f,  2.0f,  3.0f,  4.0f,  5.0f,  6.0f,  7.0f,  8.0f,
  9.0f, 10.0f, 15.0f, 20.0f, 25.0f, 30.0f, 40.0f, 50.0f
};

// You can assign any interval for any timer here, in milliseconds
uint32_t PWM_DutyCycle[NUMBER_ISR_PWMS] =
{
   5, 10, 20, 30, 40, 45, 50, 55,
  60, 65, 70, 75, 80, 85, 90, 95
};

void doingSomethingStart(int index)
{
  unsigned long currentMicros  = micros();

  deltaMicrosStart[index]    = currentMicros - previousMicrosStart[index];
  previousMicrosStart[index] = currentMicros;
}

void doingSomethingStop(int index)
{
  unsigned long currentMicros  = micros();

  // Count from start to stop PWM pulse
  deltaMicrosStop[index]    = currentMicros - previousMicrosStart[index];
  previousMicrosStop[index] = currentMicros;
}

#endif    // #if USE_COMPLEX_STRUCT

////////////////////////////////////
// Shared
////////////////////////////////////

void doingSomethingStart0()
{
  doingSomethingStart(0);
}

void doingSomethingStart1()
{
  doingSomethingStart(1);
}

void doingSomethingStart2()
{
  doingSomethingStart(2);
}

void doingSomethingStart3()
{
  doingSomethingStart(3);
}

void doingSomethingStart4()
{
  doingSomethingStart(4);
}

void doingSomethingStart5()
{
  doingSomethingStart(5);
}

void doingSomethingStart6()
{
  doingSomethingStart(6);
}

void doingSomethingStart7()
{
  doingSomethingStart(7);
}

void doingSomethingStart8()
{
  doingSomethingStart(8);
}

void doingSomethingStart9()
{
  doingSomethingStart(9);
}

void doingSomethingStart10()
{
  doingSomethingStart(10);
}

void doingSomethingStart11()
{
  doingSomethingStart(11);
}

void doingSomethingStart12()
{
  doingSomethingStart(12);
}

void doingSomethingStart13()
{
  doingSomethingStart(13);
}

void doingSomethingStart14()
{
  doingSomethingStart(14);
}

void doingSomethingStart15()
{
  doingSomethingStart(15);
}

//////////////////////////////////////////////////////

void doingSomethingStop0()
{
  doingSomethingStop(0);
}

void doingSomethingStop1()
{
  doingSomethingStop(1);
}

void doingSomethingStop2()
{
  doingSomethingStop(2);
}

void doingSomethingStop3()
{
  doingSomethingStop(3);
}

void doingSomethingStop4()
{
  doingSomethingStop(4);
}

void doingSomethingStop5()
{
  doingSomethingStop(5);
}

void doingSomethingStop6()
{
  doingSomethingStop(6);
}

void doingSomethingStop7()
{
  doingSomethingStop(7);
}

void doingSomethingStop8()
{
  doingSomethingStop(8);
}

void doingSomethingStop9()
{
  doingSomethingStop(9);
}

void doingSomethingStop10()
{
  doingSomethingStop(10);
}

void doingSomethingStop11()
{
  doingSomethingStop(11);
}

void doingSomethingStop12()
{
  doingSomethingStop(12);
}

void doingSomethingStop13()
{
  doingSomethingStop(13);
}

void doingSomethingStop14()
{
  doingSomethingStop(14);
}

void doingSomethingStop15()
{
  doingSomethingStop(15);
}

//////////////////////////////////////////////////////

#if USE_COMPLEX_STRUCT

  #if USING_PWM_FREQUENCY
  
  ISR_PWM_Data curISR_PWM_Data[NUMBER_ISR_PWMS] =
  {
    //irqCallbackStartFunc, PWM_Period, deltaMicrosStart, previousMicrosStart
    { PIN_D1,       doingSomethingStart0,    doingSomethingStop0,    1,   5, 0, 0, 0, 0 },
    { LED_BUILTIN,  doingSomethingStart1,    doingSomethingStop1,    2,  10, 0, 0, 0, 0 },
    { PIN_D3,       doingSomethingStart2,    doingSomethingStop2,    3,  20, 0, 0, 0, 0 },
    { PIN_D4,       doingSomethingStart3,    doingSomethingStop3,    4,  30, 0, 0, 0, 0 },
    { PIN_D5,       doingSomethingStart4,    doingSomethingStop4,    5,  40, 0, 0, 0, 0 },
    { PIN_D12,      doingSomethingStart5,    doingSomethingStop5,    6,  45, 0, 0, 0, 0 },
    { PIN_D13,      doingSomethingStart6,    doingSomethingStop6,    7,  50, 0, 0, 0, 0 },
    { PIN_D14,      doingSomethingStart7,    doingSomethingStop7,    8,  55, 0, 0, 0, 0 },
    { PIN_D15,      doingSomethingStart8,    doingSomethingStop8,    9,  60, 0, 0, 0, 0 },
    { PIN_D16,      doingSomethingStart9,    doingSomethingStop9,   10,  65, 0, 0, 0, 0 },
    { PIN_D17,      doingSomethingStart10,   doingSomethingStop10,  15,  70, 0, 0, 0, 0 },
    { PIN_D18,      doingSomethingStart11,   doingSomethingStop11,  20,  75, 0, 0, 0, 0 },
    { PIN_D19,      doingSomethingStart12,   doingSomethingStop12,  25,  80, 0, 0, 0, 0 },
    { PIN_D21,      doingSomethingStart13,   doingSomethingStop13,  30,  85, 0, 0, 0, 0 },
    { PIN_D22,      doingSomethingStart14,   doingSomethingStop14,  40,  90, 0, 0, 0, 0 },
    { PIN_D23,      doingSomethingStart15,   doingSomethingStop15,  50,  95, 0, 0, 0, 0 }
  };
  
  #else   // #if USING_PWM_FREQUENCY
  
  ISR_PWM_Data curISR_PWM_Data[NUMBER_ISR_PWMS] =
  {
    //irqCallbackStartFunc, PWM_Period, deltaMicrosStart, previousMicrosStart
    { PIN_D1,       doingSomethingStart0,     doingSomethingStop0,   1000000L,  5, 0, 0, 0, 0 },
    { LED_BUILTIN,  doingSomethingStart1,     doingSomethingStop1,    500000L, 10, 0, 0, 0, 0 },
    { PIN_D3,       doingSomethingStart2,     doingSomethingStop2,    333333L, 20, 0, 0, 0, 0 },
    { PIN_D4,       doingSomethingStart3,     doingSomethingStop3,    250000L, 30, 0, 0, 0, 0 },
    { PIN_D5,       doingSomethingStart4,     doingSomethingStop4,    200000L, 40, 0, 0, 0, 0 },
    { PIN_D12,      doingSomethingStart5,     doingSomethingStop5,    166667L, 45, 0, 0, 0, 0 },
    { PIN_D13,      doingSomethingStart6,     doingSomethingStop6,    142857L, 50, 0, 0, 0, 0 },
    { PIN_D14,      doingSomethingStart7,     doingSomethingStop7,    125000L, 55, 0, 0, 0, 0 },
    { PIN_D15,      doingSomethingStart8,     doingSomethingStop8,    111111L, 60, 0, 0, 0, 0 },
    { PIN_D16,      doingSomethingStart9,     doingSomethingStop9,    100000L, 65, 0, 0, 0, 0 },
    { PIN_D17,      doingSomethingStart10,    doingSomethingStop10,    66667L, 70, 0, 0, 0, 0 },
    { PIN_D18,      doingSomethingStart11,    doingSomethingStop11,    50000L, 75, 0, 0, 0, 0 },
    { PIN_D19,      doingSomethingStart12,    doingSomethingStop12,    40000L, 80, 0, 0, 0, 0 },
    { PIN_D21,      doingSomethingStart13,    doingSomethingStop13,    33333L, 85, 0, 0, 0, 0 },
    { PIN_D22,      doingSomethingStart14,    doingSomethingStop14,    25000L, 90, 0, 0, 0, 0 },
    { PIN_D23,      doingSomethingStart15,    doingSomethingStop15,    20000L, 95, 0, 0, 0, 0 }
  };
  
  #endif  // #if USING_PWM_FREQUENCY

void doingSomethingStart(int index)
{
  unsigned long currentMicros  = micros();

  curISR_PWM_Data[index].deltaMicrosStart    = currentMicros - curISR_PWM_Data[index].previousMicrosStart;
  curISR_PWM_Data[index].previousMicrosStart = currentMicros;
}

void doingSomethingStop(int index)
{
  unsigned long currentMicros  = micros();

  //curISR_PWM_Data[index].deltaMicrosStop     = currentMicros - curISR_PWM_Data[index].previousMicrosStop;
  // Count from start to stop PWM pulse
  curISR_PWM_Data[index].deltaMicrosStop     = currentMicros - curISR_PWM_Data[index].previousMicrosStart;
  curISR_PWM_Data[index].previousMicrosStop  = currentMicros;
}

#else   // #if USE_COMPLEX_STRUCT

irqCallback irqCallbackStartFunc[NUMBER_ISR_PWMS] =
{
  doingSomethingStart0,  doingSomethingStart1,  doingSomethingStart2,  doingSomethingStart3,
  doingSomethingStart4,  doingSomethingStart5,  doingSomethingStart6,  doingSomethingStart7,
  doingSomethingStart8,  doingSomethingStart9,  doingSomethingStart10, doingSomethingStart11,
  doingSomethingStart12, doingSomethingStart13, doingSomethingStart14, doingSomethingStart15
};

irqCallback irqCallbackStopFunc[NUMBER_ISR_PWMS] =
{
  doingSomethingStop0,  doingSomethingStop1,  doingSomethingStop2,  doingSomethingStop3,
  doingSomethingStop4,  doingSomethingStop5,  doingSomethingStop6,  doingSomethingStop7,
  doingSomethingStop8,  doingSomethingStop9,  doingSomethingStop10, doingSomethingStop11,
  doingSomethingStop12, doingSomethingStop13, doingSomethingStop14, doingSomethingStop15
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
  static unsigned long previousMicrosStart = startMicros;

  unsigned long currMicros = micros();

  Serial.print(F("SimpleTimer (ms): ")); Serial.print(SIMPLE_TIMER_MS);
  Serial.print(F(", us : ")); Serial.print(currMicros);
  Serial.print(F(", Dus : ")); Serial.println(currMicros - previousMicrosStart);

  for (uint16_t i = 0; i < NUMBER_ISR_PWMS; i++)
  {
#if USE_COMPLEX_STRUCT
    Serial.print(F("PWM Channel : ")); Serial.print(i);
    Serial.print(F(", programmed Period (us): "));

  #if USING_PWM_FREQUENCY
    Serial.print(1000000 / curISR_PWM_Data[i].PWM_Freq);
  #else
    Serial.print(curISR_PWM_Data[i].PWM_Period);
  #endif
    
    Serial.print(F(", actual : ")); Serial.print(curISR_PWM_Data[i].deltaMicrosStart);
    
    Serial.print(F(", programmed DutyCycle : ")); 

    Serial.print(curISR_PWM_Data[i].PWM_DutyCycle);
    
    Serial.print(F(", actual : ")); Serial.println((float) curISR_PWM_Data[i].deltaMicrosStop * 100.0f / curISR_PWM_Data[i].deltaMicrosStart);
    
#else

    Serial.print(F("PWM Channel : ")); Serial.print(i);
    
  #if USING_PWM_FREQUENCY
    Serial.print(1000000 / PWM_Freq[i]);
  #else
    Serial.print(PWM_Period[i]);
  #endif
  
    Serial.print(F(", programmed Period (us): ")); Serial.print(PWM_Period[i]);
    Serial.print(F(", actual : ")); Serial.print(deltaMicrosStart[i]);

    Serial.print(F(", programmed DutyCycle : ")); 
  
    Serial.print(PWM_DutyCycle[i]);
      
    Serial.print(F(", actual : ")); Serial.println( (float) deltaMicrosStop[i] * 100.0f / deltaMicrosStart[i]);
#endif
  }

  previousMicrosStart = currMicros;
}

void setup()
{
  Serial.begin(115200);
  while (!Serial);

  delay(2000);

  Serial.print(F("\nStarting ISR_16_PWMs_Array_Complex on ")); Serial.println(ARDUINO_BOARD);
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

  startMicros = micros();

  // Just to demonstrate, don't use too many ISR Timers if not absolutely necessary
  // You can use up to 16 timer for each ISR_PWM
  
  for (uint16_t i = 0; i < NUMBER_ISR_PWMS; i++)
  {
#if USE_COMPLEX_STRUCT
    curISR_PWM_Data[i].previousMicrosStart = startMicros;
    //ISR_PWM.setInterval(curISR_PWM_Data[i].PWM_Period, curISR_PWM_Data[i].irqCallbackStartFunc);

    //void setPWM(uint32_t pin, uint32_t frequency, uint32_t dutycycle
    // , timer_callback_p StartCallback = nullptr, timer_callback_p StopCallback = nullptr)

  #if USING_PWM_FREQUENCY
    // You can use this with PWM_Freq in Hz
    ISR_PWM.setPWM(curISR_PWM_Data[i].PWM_Pin, curISR_PWM_Data[i].PWM_Freq, curISR_PWM_Data[i].PWM_DutyCycle, 
                   curISR_PWM_Data[i].irqCallbackStartFunc, curISR_PWM_Data[i].irqCallbackStopFunc);
  #else
    // Or You can use this with PWM_Period in us
    ISR_PWM.setPWM_Period(curISR_PWM_Data[i].PWM_Pin, curISR_PWM_Data[i].PWM_Period, curISR_PWM_Data[i].PWM_DutyCycle, 
                          curISR_PWM_Data[i].irqCallbackStartFunc, curISR_PWM_Data[i].irqCallbackStopFunc);
  #endif
  
#else
    previousMicrosStart[i] = micros();
    
  #if USING_PWM_FREQUENCY
    // You can use this with PWM_Freq in Hz
    ISR_PWM.setPWM(PWM_Pin[i], PWM_Freq[i], PWM_DutyCycle[i], irqCallbackStartFunc[i], irqCallbackStopFunc[i]);
  #else
    // Or You can use this with PWM_Period in us
    ISR_PWM.setPWM_Period(PWM_Pin[i], PWM_Period[i], PWM_DutyCycle[i], irqCallbackStartFunc[i], irqCallbackStopFunc[i]);
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
