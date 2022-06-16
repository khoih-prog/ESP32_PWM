# ESP32_PWM Library

[![arduino-library-badge](https://www.ardu-badge.com/badge/ESP32_PWM.svg?)](https://www.ardu-badge.com/ESP32_PWM)
[![GitHub release](https://img.shields.io/github/release/khoih-prog/ESP32_PWM.svg)](https://github.com/khoih-prog/ESP32_PWM/releases)
[![GitHub](https://img.shields.io/github/license/mashape/apistatus.svg)](https://github.com/khoih-prog/ESP32_PWM/blob/master/LICENSE)
[![contributions welcome](https://img.shields.io/badge/contributions-welcome-brightgreen.svg?style=flat)](#Contributing)
[![GitHub issues](https://img.shields.io/github/issues/khoih-prog/ESP32_PWM.svg)](http://github.com/khoih-prog/ESP32_PWM/issues)

---
---

## Table of Contents

* [Changelog](#changelog)
  * [Releases v1.3.3](#releases-v133)
  * [Releases v1.3.2](#releases-v132)
  * [Releases v1.3.1](#releases-v131)
  * [Releases v1.3.0](#releases-v130)
  * [Releases v1.2.2](#releases-v122)
  * [Releases v1.2.1](#releases-v121)
  * [Releases v1.2.0](#releases-v120)
  * [Releases v1.1.1](#releases-v111)
  * [Releases v1.1.0](#releases-v110)
  * [Releases v1.0.1](#releases-v101)
  * [Releases v1.0.0](#releases-v100)

---
---

## Changelog

### Releases v1.3.3

1. Add support to new Adafruit boards such as QTPY_ESP32S2, FEATHER_ESP32S3_NOPSRAM and QTPY_ESP32S3_NOPSRAM

### Releases v1.3.2

1. Remove crashing `PIN_D24` from examples


### Releases v1.3.1

1. Fix `DutyCycle` bug. Check [float precisison of DutyCycle only sometimes working #3](https://github.com/khoih-prog/SAMD_Slow_PWM/issues/3)
2. Fix `New Period` display bug. Check [random dropouts #4](https://github.com/khoih-prog/SAMD_Slow_PWM/issues/4)
3. Update examples

### Releases v1.3.0

1. Add support to new `ESP32-S3` (ESP32S3_DEV, ESP32_S3_BOX, UM TINYS3, UM PROS3, UM FEATHERS3, etc.)
2. Modify examples accordingly

### Releases v1.2.2

1. Use `float` for `DutyCycle` and `Freq`, `uint32_t` for `period`. 
2. Optimize code by not calculation in ISR

### Releases v1.2.1

1. DutyCycle to be optionally updated at the end current PWM period instead of immediately. Check [DutyCycle to be updated at the end current PWM period #2](https://github.com/khoih-prog/ESP8266_PWM/issues/2)

### Releases v1.2.0

1. Fix `multiple-definitions` linker error. Drop `src_cpp` and `src_h` directories
2. Add support to `ESP32_C3`
3. Add example [multiFileProject](examples/multiFileProject) to demo for multiple-file project
4. Improve accuracy by using `double`, instead of `uint32_t` for `dutycycle`, `period`
5. Update examples accordingly

### Releases v1.1.1

1. Fix examples to not use GPIO1/TX0 for core v2.0.1+

### Releases v1.1.0

1. Add functions to modify PWM settings on-the-fly
2. Fix bug
3. Add example to demo how to modify PWM settings on-the-fly

### Releases v1.0.1

1. Adding PWM end-of-duty-cycle callback function.
2. Fix bug.
3. Add end-of-duty-cycle callback feature to examples

### Releases v1.0.0

1. Initial coding for ESP32, ESP32_S2 boards using [ESP32 core v2.0.0+](https://github.com/espressif/arduino-esp32/releases/tag/2.0.0)
