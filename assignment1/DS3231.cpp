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
    /* TODO: should I open connection to my I2c here? */
    int bcdToDec(unsigned char bcd) {
        return ((bcd >> 4) * 10) + (bcd & 0x0F);
    }

    int decToBcd(int dec) {
        return ((dec / 10) << 4) | (dec % 10);
    }

    // constructor is made
    DS3231::DS3231(unsigned int bus, unsigned int device) : I2CDevice(bus, device) {}
    DS3231 rtc(1, RTC_ADDR);

    void DS3231::setTimeDate() {
        time_t timestamp = time(&timestamp);
        struct tm *ltm = localtime(&timestamp);

        /* Using ctime functions to utilize setting current time */
        int timeComponents[6] = { ltm->tm_sec, ltm->tm_min, ltm->tm_hour,
                                  ltm->tm_mday, ltm->tm_mon + 1, (ltm->tm_year + 1900) % 100 };

        /* RTC registers: seconds, mins, hours, date, month, year (no date for now) */
        const int rtcRegisters[6] = { RTC_SECONDS, RTC_MINS, RTC_HOURS, RTC_DATE, RTC_MONTH, RTC_YEAR };

        for (int i = 0; i < 6; i++) {
            rtc.writeRegister(rtcRegisters[i], decToBcd(timeComponents[i]));
        }
    }

    void DS3231::readRegisterYear() {
        unsigned char value = rtc.readRegister(0x06);
        cout << "Value at 0x68 0x06: 0x" << hex << (int)value << dec << endl;
    }

    void DS3231::readTimeDate() {
        int timeDateVal[7];
        unsigned char * dataList = rtc.readRegisters(7, RTC_SECONDS);
        // todo: why errno is still here on the error -> Error: Success ?
        if (!dataList) {
            perror("Sorry, no timedate data was found.\n");
            return;
        }

        /* TODO: add functionality to add days = 01 == Sunday */
        for (int i = 0; i < 7; i++) {
            timeDateVal[i] = bcdToDec(dataList[i]);
        }
        printf("The current time is: %02d:%02d:%02d %02d/%02d/%02d\n", timeDateVal[2], timeDateVal[1],
               timeDateVal[0], timeDateVal[4], timeDateVal[5], timeDateVal[6]);
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
    
        // todo: ALARMS ARE TRIGGERED RANDOMLY -> when setting hours/minutes, then fine, but setting seconds does not do alarm triggering
        writeRegister(ALARM1_REG_SECONDS, secReg);
        writeRegister(ALARM1_REG_MINUTES, minReg);
        writeRegister(ALARM1_REG_HOURS, hourReg);
        writeRegister(ALARM1_REG_DAY, dateReg);
    
        // Debug output
        cout << "Alarm registers (hex):" << endl;
        cout << "Seconds: 0x" << hex << (int)readRegister(ALARM1_REG_SECONDS) << endl;
        cout << "Minutes: 0x" << hex << (int)readRegister(ALARM1_REG_MINUTES) << endl;
        cout << "Hours: 0x" << hex << (int)readRegister(ALARM1_REG_HOURS) << endl;
        cout << "Day/Date: 0x" << hex << (int)readRegister(ALARM1_REG_DAY) << endl;

        cout << "[DEBUG] Checking alarm status after setting alarm..." << endl;
        checkAlarmsStatus();  // Verify if alarm has been set properly
    }

    void DS3231::readAlarmOne() {
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
    }

    // check alarms statuses on 0x0F 
    void DS3231::checkAlarmsStatus() {
        unsigned char status = readRegister(STATUS_REG);
        cout << "Status Register (Hex): 0x" << hex << (int)status << dec << endl;

        bool alarmTriggered = status & 0x01;

        if (alarmTriggered) {
            cout << "Alarm 1 has been triggered!" << endl;
        }
        if (!alarmTriggered) {
            cout << "No alarms triggered." << endl;
        }

        // Clear the alarm bits
        writeRegister(STATUS_REG, status & ~0x03);
    }
    }

