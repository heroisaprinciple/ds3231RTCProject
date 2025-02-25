/*
 * Application.cpp
 * Copyright (c) 2025 Derek Molloy (www.derekmolloy.ie)
 * Modified by: Arina Sofiyeva
 */

#include <iostream>
#include "DS3231.h"
#include <pthread.h>
#include <unistd.h>

using namespace std;
using namespace een1071;

int main() {
    cout << "DS3231 RTC Code for Assignment 1 EEN1071" << endl;
    DS3231 rtc(1, RTC_ADDR);

    // Clearing registers first just in case
    cout << "\nClearing all time/date registers:" << endl;
    rtc.clearTimeDate();

    // Let user choose time format:
    int format;
    cout << "\nChoose time format (24/12): ";
    cin >> format;

    // Set format based on user choice
    if (format == 12) {
        cout << "\nSetting 12-hour mode..." << endl;
        rtc.setTimeFormat(false);
    } else {
        cout << "\nSetting 24-hour mode..." << endl;
        rtc.setTimeFormat(true);
    }

    cout << "\nSetting current time:" << endl;
    rtc.setTimeDate();
    rtc.readTimeDate();

    cout << "\nReading temperature:" << endl;
    rtc.readTemperature();

    // Trigger alarm 1

    cout << "\nTesting alarm 1"<< endl;
    rtc.setAlarmOne();

    cout << "Waiting for 60 seconds..." << endl;
    sleep(60);  // Wait for 60 seconds to trigger alarm

    cout << "\nChecking if alarm triggered:" << endl;
    unsigned char status = rtc.readRegister(STATUS_REG); // 0x89 == 10001001 will indicate that alarm1 is triggered successfully hehe
    cout << "Status Register (Hex): 0x" << hex << (int)status << dec << endl;
    if(status & 0x01) {
        cout << "Alarm 1 triggered!" << endl;
        cout << "Current time: ";
        rtc.readTimeDate();

        // Clear the alarm flag
        rtc.writeRegister(STATUS_REG, status & ~0x01);
    } else {
        cout << "Alarm 1 did not trigger as expected" << endl;
    }

    // Trigger alarm 2

    cout << "\nTesting alarm 2"<< endl;
    rtc.setAlarmTwo();

    cout << "Waiting for another 60 seconds..." << endl;
    sleep(60);  // Wait for 120 seconds to trigger alarm

    cout << "\nChecking if alarm triggered:" << endl;
    status = rtc.readRegister(STATUS_REG);
    cout << "Status Register (Hex): 0x" << hex << (int)status << dec << endl; // 0x8a == 10001010 will indicate that alarm2 is triggered successfully

    if(status & 0x02) {
        cout << "Alarm 2 triggered!" << endl;
        cout << "Current time: ";
        rtc.readTimeDate();

        // Clear the alarm flag
        rtc.writeRegister(STATUS_REG, status & ~0x02);
    } else {
        cout << "Alarm 2 did not trigger as expected" << endl;
    }

    return 0;
}

