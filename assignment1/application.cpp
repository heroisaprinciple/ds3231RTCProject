/*
 * Application.cpp
 * Copyright (c) 2025 Derek Molloy (www.derekmolloy.ie)
 * Modified by: Arina Sofiyeva
 */

#include <iostream>
#include "DS3231.h"
#include <unistd.h>
#include <pthread.h>

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

    // cout << "\nChecking alarm status:" << endl;
    // rtc.checkAlarmsStatus();

    return 0;
}
