/*
 * DS3231.cpp
 * Copyright (c) 2025 Derek Molloy (www.derekmolloy.ie)
 * Modified by: Arina Sofiyeva
 */

 #include "DS3231.h"
 #include <iostream>
 #include <unistd.h>
 #include <math.h>
 #include <stdio.h>
 #include <ctime>
 #include <string>
 #include <pigpio.h>

using namespace std;

namespace een1071 {
    int bcdToDec(unsigned char bcd) {
        return ((bcd >> 4) * 10) + (bcd & 0x0F);
    }

    int decToBcd(int dec) {
        return ((dec / 10) << 4) | (dec % 10);
    }

    // constructor is made
    DS3231::DS3231(unsigned int bus, unsigned int device) : I2CDevice(bus, device) {}
    DS3231 rtc(1, RTC_ADDR);

    void DS3231::clearTimeDate() {
        const int rtcRegisters[7] = { RTC_SECONDS, RTC_MINS, RTC_DAYS, RTC_HOURS, RTC_DATE, RTC_MONTH, RTC_YEAR };

        for (int i = 0; i < 7; i++) {
            cout << "Register 0x" << hex << rtcRegisters[i]
                 << " before clearing: 0x" << (int)readRegister(rtcRegisters[i]) << dec << endl;

            writeRegister(rtcRegisters[i], 0x00);

            cout << "Register 0x" << hex << rtcRegisters[i]
                 << " after clearing: 0x" << (int)readRegister(rtcRegisters[i]) << dec << endl;
        }
    }

    unsigned char DS3231::checkIf12HFormat(unsigned char hourReg, int hour) {
        bool is12Hour = hourReg & (1 << HOUR_MODE_BIT);

        if (is12Hour) {
            // Converting 24-hour to 12-hour format
            int hour12;
            if (hour == 0) hour12 = 12;        // 00:00 -> 12 AM
            else if (hour > 12) hour12 = hour - 12;  // 13-23 -> 1-11 PM
            else hour12 = hour;                 // 1-12 -> 1-12 AM/PM

            // Convert to BCD
            hourReg = decToBcd(hour12);
            hourReg |= (1 << HOUR_MODE_BIT);  // Set 12h mode bit

            // Set PM bit
            if (hour >= 12) {
                hourReg |= (1 << AM_PM_BIT);
            }
        } else {
            hourReg = decToBcd(hour);
        }

        return hourReg;
    }

    void DS3231::setTimeDate() {
        time_t timestamp = time(&timestamp);
        struct tm *ltm = localtime(&timestamp);

        unsigned char hourReg = readRegister(RTC_HOURS);
        int hour = ltm->tm_hour;  // This is in 24h format from ctime
        hourReg = checkIf12HFormat(hourReg, hour);

        int timeComponents[7] = { ltm->tm_sec, ltm->tm_min, ltm->tm_hour, ltm->tm_wday, ltm->tm_mday, ltm->tm_mon + 1, (ltm->tm_year + 1900) % 100 };

        const int rtcRegisters[7] = { RTC_SECONDS, RTC_MINS, RTC_HOURS, RTC_DAYS, RTC_DATE, RTC_MONTH, RTC_YEAR };

        // Write all components
        for (int i = 0; i < 7; i++) {
            if (i == 2) {  // Hours
                rtc.writeRegister(RTC_HOURS, hourReg);
            }
            else if (i == 3) { // Day of a week
                rtc.writeRegister(RTC_DAYS, decToBcd(timeComponents[3] + 1));
            }
            else {
                rtc.writeRegister(rtcRegisters[i], decToBcd(timeComponents[i]));
            }
        }
    }

    void DS3231::setTimeFormat(bool is24Hour) {
        unsigned char hourReg = readRegister(RTC_HOURS);

        if (!is24Hour) {
            hourReg |= (1 << HOUR_MODE_BIT);  // Set bit 6 for 12h mode
            // Current hour from ctime will determine AM/PM (bit 5)
            time_t now = time(nullptr);
            struct tm *ltm = localtime(&now);
            if (ltm->tm_hour >= 12) {
                hourReg |= (1 << AM_PM_BIT);  // Set bit 5 for PM
            }
        }

        writeRegister(RTC_HOURS, hourReg);
    }

    int DS3231::readHourValue(unsigned char hourReg) {
        bool is12Hour = hourReg & (1 << HOUR_MODE_BIT);
        int hour;

        if (is12Hour) {
            hour = bcdToDec(hourReg & 0x1F);  // 0x1F mask for 12h mode
        } else {
            hour = bcdToDec(hourReg & 0x3F);  // 0x3F mask for 24h mode
        }

        return hour;
    }

    std::string DS3231::getDayOfWeek(int day) {
        const char* dayNames[] = {"", "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
        return dayNames[day];
    }

    void DS3231::readTimeDate() {
        int timeDateVal[7];
        unsigned char * dataList = rtc.readRegisters(7, RTC_SECONDS);
        if (!dataList) {
            perror("Sorry, no timedate data was found.\n");
            return;
        }

        cout << "Hour register value: 0x" << hex << (int)dataList[2] << dec << endl;
        bool is12Hour = dataList[2] & (1 << HOUR_MODE_BIT);
        int hour = readHourValue(dataList[2]);

        for (int i = 0; i < 7; i++) {
            if (i == 2) {  // Hours
                if (is12Hour) {
                    bool isPM = dataList[2] & (1 << AM_PM_BIT);
                    if (hour == 0) hour = 12;
                    else if (hour > 12) hour = hour - 12;
                    timeDateVal[i] = hour;
                } else {
                    timeDateVal[i] = hour;
                }
            } else {
                timeDateVal[i] = bcdToDec(dataList[i]);
            }
        }

        printf("The current time is: %02d:%02d:%02d %02d/%02d/%02d",
               timeDateVal[2], timeDateVal[1], timeDateVal[0],
               timeDateVal[4], timeDateVal[5], timeDateVal[6]);

        if (is12Hour) {
            bool isPM = dataList[2] & (1 << AM_PM_BIT);
            printf(" %s", isPM ? "PM" : "AM");
        }
        printf("\n");
    }

    void DS3231::readTemperature() {
        // Reads a whole number from 0x11 (start) and finishes at 0x12 (a fractional part -> 2 bytes)
        unsigned char * tempList = rtc.readRegisters(2, RTC_TEMP);

        if (!tempList) {
            perror("Error: Failed to read temperature registers!\n");
            return;
        }

        // Convert temperature
        int wholePart = tempList[0];  // MSB
        float fractionalPart = (tempList[1] >> 6) * 0.25;  // Extract top 2 bits, multiply by 0.25
        float temperature = wholePart + fractionalPart;

        cout << "Temperature: " << temperature << "C" << endl;
    }

    /* TODO: add implementation for user to set an alarm time */
    // This alarm will be triggered when seconds, mins, hours and day (current day of week) are matched! */
    void DS3231::setAlarmOne() {
        time_t timestamp = time(&timestamp);
        struct tm *ltm = localtime(&timestamp);
        unsigned char statusBefore = readRegister(STATUS_REG);
        unsigned char hourReg = readRegister(RTC_HOURS);
        bool is12Hour = hourReg & (1 << HOUR_MODE_BIT);

        // Add 1 minute to current time
        int minutes = ltm->tm_min + 1;
        if(minutes >= 60) {
            minutes = 0;
            ltm->tm_hour++;
        }

        // Clear alarms status before setting new alarm
        writeRegister(STATUS_REG, statusBefore & ~0x03);

        // Set alarm registers
        writeRegister(ALARM1_REG_SECONDS, decToBcd(ltm->tm_sec) & 0x7F);  // A1M1 = 0
        writeRegister(ALARM1_REG_MINUTES, decToBcd(minutes) & 0x7F);      // A1M2 = 0

        if(is12Hour) {
            unsigned char alarmHourReg = checkIf12HFormat(hourReg, ltm->tm_hour);
            writeRegister(ALARM1_REG_HOURS, alarmHourReg & 0x7F);   // A1M3 = 0; bit 6 is 1 and bit 5 is 0/1 depending on am/pm
        } else {
            writeRegister(ALARM1_REG_HOURS, decToBcd(ltm->tm_hour) & 0x7F);   // A1M3 = 0
        }

        // Day alarm (RTC starts at 0 == Sunday; bit DYDT is set to 1, but A1M4 is 0 to indicate usage of date/day field)
        writeRegister(ALARM1_REG_DAY, 0x40 | decToBcd(ltm->tm_wday + 1));
        // Enable Alarm 1 interrupt
        writeRegister(CONTROL_REG, 0x05);  // Set INTCN and A1|E
        readAlarmOne();
    }

    void DS3231::readAlarmOne() {
        unsigned char sec = readRegister(ALARM1_REG_SECONDS);
        unsigned char min = readRegister(ALARM1_REG_MINUTES);
        unsigned char hour = readRegister(ALARM1_REG_HOURS);
        unsigned char day = readRegister(ALARM1_REG_DAY);
        bool is12Hour = hour & (1 << 6); // Checking bit 6 to understand if 12h mode or 24h

        if (is12Hour) {
            bool isPM = hour & (1 << 5);
            int hour12 = bcdToDec(hour & 0x1F);

            cout << "Alarm 1 is set for: "
                 << hour12 << ":"
                 << bcdToDec(min & 0x7F) << ":"
                 << bcdToDec(sec & 0x7F) << " "
                 << (isPM ? "PM" : "AM");
        } else {
            cout << "Alarm 1 is set for: "
                 << bcdToDec(hour & 0x7F) << ":"
                 << bcdToDec(min & 0x7F) << ":"
                 << bcdToDec(sec & 0x7F);
        }

        cout << " on day " << getDayOfWeek(bcdToDec(day & 0x3F)) << endl;
    }

    // This alarm will be triggered when mins, hours and date (today) are matched! */
    // TODO: add months: Feb = 02 (like on getDayOfWeek) for output to be 'alarm 2 is set on February, 25'
    void DS3231::setAlarmTwo() {
        time_t timestamp = time(&timestamp);
        struct tm *ltm = localtime(&timestamp);
        unsigned char statusBefore = readRegister(STATUS_REG);
        unsigned char hourReg = readRegister(RTC_HOURS);
        bool is12Hour = hourReg & (1 << HOUR_MODE_BIT);

        // Add 1 minute to current time
        int minutes = ltm->tm_min + 1;
        if(minutes >= 60) {
            minutes = 0;
            ltm->tm_hour++;
        }

        // Clear alarms status before setting new alarm
        writeRegister(STATUS_REG, statusBefore & ~0x03);

        // Set alarm registers
        writeRegister(ALARM2_REG_MINUTES, decToBcd(minutes) & 0x7F); // A2M2 = 0

        if(is12Hour) {
            unsigned char alarmHourReg = checkIf12HFormat(hourReg, ltm->tm_hour);
            writeRegister(ALARM2_REG_HOURS, alarmHourReg & 0x7F);   // A2M3 = 0; bit 6 is 1 and bit 5 is 0/1 depending on am/pm
        } else {
            writeRegister(ALARM2_REG_HOURS, decToBcd(ltm->tm_hour) & 0x7F);   // A2M3 = 0
        }

        writeRegister(ALARM2_REG_DAY, decToBcd(ltm->tm_mday) & 0x3F);
        // Enable Alarm 2 interrupt
        writeRegister(CONTROL_REG, 0x06);  // Set INTCN and A2|E
        readAlarmTwo();
    }

    void DS3231::readAlarmTwo() {
        unsigned char min = readRegister(ALARM2_REG_MINUTES);
        unsigned char hour = readRegister(ALARM2_REG_HOURS);
        unsigned char date = readRegister(ALARM2_REG_DATE);
        bool is12Hour = hour & (1 << 6); // Checking bit 6 to understand if 12h mode or 24h

        if (is12Hour) {
            bool isPM = hour & (1 << 5);
            int hour12 = bcdToDec(hour & 0x1F);

            cout << "Alarm 2 is set for: "
                 << hour12 << ":"
                 << bcdToDec(min & 0x7F) << ":"
                 << (isPM ? "PM" : "AM");
        } else {
            cout << "Alarm 2 is set for: "
                 << bcdToDec(hour & 0x7F) << ":"
                 << bcdToDec(min & 0x7F);
        }

        cout << " on date " << bcdToDec(date & 0x3F) << endl;
    }

    void DS3231::triggerLED() {
        unsigned char status = readRegister(STATUS_REG);
        if (status & 0x01) {
            cout << "Yay, alarm 1 triggered!" << endl;
            gpioWrite(LED_PIN, 1);
            gpioDelay(1000000);
            gpioWrite(LED_PIN, 0);
        }

        if (status & 0x02) {
            cout << "Yay, alarm 2 triggered!" << endl;
            gpioWrite(LED_PIN, 1);
            gpioDelay(1000000);
            gpioWrite(LED_PIN, 0);
        }
    }
}


