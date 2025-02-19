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
        // Clear all time and date registers
        // TODO: add days */
        const int rtcRegisters[6] = { RTC_SECONDS, RTC_MINS, RTC_HOURS, RTC_DATE, RTC_MONTH, RTC_YEAR };

        for (int i = 0; i < 6; i++) {
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

    /* TODO: add day functionality */
    /* TODO: Ask Derek how values at 0x02 should be stored? like, if is it 10pm, it is stored as 0x70 at 0x02 (0111 0000 and bits 5 and 6 should be set). Should it be stored as 10 in bcd (0x10 at 0x02)?*/
    void DS3231::setTimeDate() {
        time_t timestamp = time(&timestamp);
        struct tm *ltm = localtime(&timestamp);

        unsigned char hourReg = readRegister(RTC_HOURS);
        int hour = ltm->tm_hour;  // This is in 24h format from ctime
        hourReg = checkIf12HFormat(hourReg, hour);

        int timeComponents[6] = { ltm->tm_sec, ltm->tm_min, ltm->tm_hour, ltm->tm_mday, ltm->tm_mon + 1, (ltm->tm_year + 1900) % 100 };

        const int rtcRegisters[6] = { RTC_SECONDS, RTC_MINS, RTC_HOURS, RTC_DATE, RTC_MONTH, RTC_YEAR };

        // Write all components except hours
        for (int i = 0; i < 6; i++) {
            if (i == 2) {  // Hours
                rtc.writeRegister(RTC_HOURS, hourReg);
            } else {
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
        // Reads a whole number from 0x11 (start) and finishes at 0x12 (a fractional part -> we need 2 bytes)
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
    // todo: ALARMS ARE TRIGGERED RANDOMLY -> when setting hours/minutes, then fine, but setting seconds does not do alarm triggering
    void DS3231::setAlarmOne(int hours, int minutes, int seconds, bool useDate, bool useDay, int date, int day) {
        /*
        // check status of 0x0F:
        cout << "[DEBUG] Checking alarm status before setting new alarm..." << endl;
        checkAlarmsStatus();

        // Clear bit 0 on A1F
        unsigned char status = readRegister(STATUS_REG);
        writeRegister(STATUS_REG, status & ~0x01);

        unsigned char secReg, minReg, hourReg, dateReg;

        // seconds = seconds % 60;
        // minutes = minutes % 60;
        // hours = hours % 24;


        //  Alarm when hours, minutes, and seconds match
        if (!useDate && !useDay) {
            secReg = decToBcd(seconds) & 0x7F;  // A1M1 = 0
            minReg = decToBcd(minutes) & 0x7F;  // A1M2 = 0
            hourReg = decToBcd(hours) & 0x7F;   // A1M3 = 0
            dateReg = 0x80;                     // A1M4 = 1
        }
        else if (useDate) {
            // Alarm when date, hours, minutes, and seconds match
            secReg = decToBcd(seconds) & 0x7F;
            minReg = decToBcd(minutes) & 0x7F;
            hourReg = decToBcd(hours) & 0x7F;
            dateReg = decToBcd(date) & 0x3F;       // A1M4 = 0, DY/DT = 0
        }
        else if (useDay) {
            // Alarm when day, hours, minutes, and seconds match
            secReg = decToBcd(seconds) & 0x7F;
            minReg = decToBcd(minutes) & 0x7F;
            hourReg = decToBcd(hours) & 0x7F;
            dateReg = (decToBcd(day) & 0x3F) | 0x40;  // A1M4 = 0, DY/DT = 1
        }

        // todo: ALARMS ARE TRIGGERED RANDOMLY -> when setting hours/minutes, then fine, but setting seconds does not do alarm triggering??
        writeRegister(ALARM1_REG_SECONDS, secReg);
        writeRegister(ALARM1_REG_MINUTES, minReg);
        writeRegister(ALARM1_REG_HOURS, hourReg);
        writeRegister(ALARM1_REG_DAY, dateReg);

        cout << "Alarm registers (hex):" << endl;
        cout << "Seconds: 0x" << hex << (int)readRegister(ALARM1_REG_SECONDS) << endl;
        cout << "Minutes: 0x" << hex << (int)readRegister(ALARM1_REG_MINUTES) << endl;
        cout << "Hours: 0x" << hex << (int)readRegister(ALARM1_REG_HOURS) << endl;
        cout << "Day/Date: 0x" << hex << (int)readRegister(ALARM1_REG_DAY) << endl;

        cout << "[DEBUG] Checking alarm status after setting alarm..." << endl;
        checkAlarmsStatus();
        */
    }

    void DS3231::readAlarmOne() {
        /*
        unsigned char sec = readRegister(ALARM1_REG_SECONDS);
        unsigned char min = readRegister(ALARM1_REG_MINUTES);
        unsigned char hour = readRegister(ALARM1_REG_HOURS);
        unsigned char day = readRegister(ALARM1_REG_DAY);

        cout << "Alarm 1 is set for: "
             << bcdToDec(hour & 0x7F) << ":"
             << bcdToDec(min & 0x7F) << ":"
             << bcdToDec(sec & 0x7F);

        if (day & 0x80) {
            // A1M4 is 1, this is a time-only alarm
            cout << " (time-only alarm)";
        } else {
            // A1M4 is 0, this is a date or day alarm
            if (day & 0x40) {
                // DY/DT is 1, this is a day alarm
                cout << " on day " << bcdToDec(day & 0x3F);
            } else {
                // DY/DT is 0, this is a date alarm
                cout << " on date " << bcdToDec(day & 0x3F);
            }
        }
        cout << endl;
        */
    }

    // check alarms statuses on 0x0F
    void DS3231::checkAlarmsStatus() {
        /*
        unsigned char status = readRegister(STATUS_REG);
        cout << "Status Register before clearing (Hex): 0x" << hex << (int)status << dec << endl;
           // Clear OSF (bit 7) while preserving other bits
        writeRegister(STATUS_REG, status & ~0x80);
        status = readRegister(STATUS_REG);
        cout << "Status Register after clearing (Hex): 0x" << hex << (int)status << dec << endl;

        bool alarmTriggered = status & 0x01;

        if (alarmTriggered) {
            cout << "Alarm 1 has been triggered!" << endl;
        }
        if (!alarmTriggered) {
            cout << "No alarms triggered." << endl;
        }

        // Clear the alarm bits
        writeRegister(STATUS_REG, status & ~0x03);
        */
    }
}

