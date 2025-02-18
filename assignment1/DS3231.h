/*
 * DS3231.h
 * Copyright (c) 2025 Derek Molloy (www.derekmolloy.ie)
 * Modified by: Arina Sofiyeva
 */

#ifndef DS3231_H_
#define DS3231_H_

#include"I2CDevice.h"

#define RTC_ADDR 0x68

#define RTC_SECONDS 0x00
#define RTC_MINS 0x01
#define RTC_HOURS 0x02
#define RTC_DAYS 0x03
#define RTC_DATE 0x04
#define RTC_MONTH 0x05
#define RTC_YEAR 0x06

#define HOUR_MODE_BIT 6
#define AM_PM_BIT 5

#define RTC_TEMP 0x11

#define ALARM1_REG_SECONDS 0x07
#define ALARM1_REG_MINUTES 0x08
#define ALARM1_REG_HOURS 0x09
#define ALARM1_REG_DAY 0x0A
#define ALARM1_REG_DATE 0x0A

#define STATUS_REG 0x0F

namespace een1071 {
    int bcdToDec(unsigned char);
    int decToBcd(int);

    class DS3231:public I2CDevice{
    public:
        DS3231(unsigned int bus, unsigned int device);
        void clearTimeDate();

        void setTimeFormat(bool);
        void setAMPM(bool isPM);
        void setTimeDate();

        void readRegisterYear();
        void readTemperature();
        void readTimeDate();

        void setAlarmOne(int, int, int, bool, bool, int, int);
        void readAlarmOne();
        void checkAlarmsStatus();
    };

} /* namespace een1071 */

#endif
