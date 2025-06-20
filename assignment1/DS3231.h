/*
 * DS3231.h
 * Copyright (c) 2025 Derek Molloy (www.derekmolloy.ie)
 * Modified by: Arina Sofiyeva
 */

#ifndef DS3231_H_
#define DS3231_H_

#include"I2CDevice.h"
#include <string>

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

#define ALARM2_REG_MINUTES 0x0B
#define ALARM2_REG_HOURS 0x0C
#define ALARM2_REG_DATE 0x0D
#define ALARM2_REG_DAY 0x0D

#define STATUS_REG 0x0F
#define CONTROL_REG 0x0E

#define INT_SQW_PIN 17
#define LED_PIN 18

namespace een1071 {
    int bcdToDec(unsigned char);
    int decToBcd(int);

    class DS3231:public I2CDevice{
    public:
        DS3231(unsigned int bus, unsigned int device);
        void clearTimeDate();
        std::string getDayOfWeek(int);
        std::string getMonth(int);

        void setTimeFormat(bool);
        unsigned char checkIf12HFormat(unsigned char, int);
        int readHourValue(unsigned char);

        void setAMPM(bool isPM);
        void setTimeDate();

        void readRegisterYear();
        void readTemperature();
        void readTimeDate();

        void setAlarmOne();
        void readAlarmOne();

        void setAlarmTwo();
        void readAlarmTwo();

        void triggerLED();

        void enableSQW(int);
        void disableSQW();

        bool sqwStatusCheck(unsigned char, std::string, std::string);
    };

} /* namespace een1071 */

#endif
