/*
 * Application.cpp
 * Copyright (c) 2025 Derek Molloy (www.derekmolloy.ie)
 * Modified by: Arina Sofiyeva
 */

#include <iostream>
#include "DS3231.h"
#include <pthread.h>
#include <unistd.h>
#include <pigpio.h>

using namespace std;
using namespace een1071;

// TODO: remove a global ptr
DS3231 *glbRTC;

// TODO: EACH ALARM IS BEING TRIGGERED TWICE

void interruptCallback(int gpio, int level, uint32_t tick, void * userData) {
    // DS3231 *rtc = (DS3231*)userData;
    glbRTC->triggerLED();
}

int main() {
    DS3231 rtc(1, RTC_ADDR);
    unsigned char status;
    unsigned char control;

    // sending a pointer to rtc to a global var
    glbRTC = &rtc;

    if (gpioInitialise() < 0) {
        perror("Can't initialize pigpio.");
        return 1;
    }

    gpioSetMode(INT_SQW_PIN, PI_INPUT);
    gpioSetMode(LED_PIN, PI_OUTPUT);
    gpioSetAlertFuncEx(INT_SQW_PIN, interruptCallback, NULL);

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

    status = rtc.readRegister(STATUS_REG); // 0x1 == 00000001 will indicate that alarm1 is triggered successfully hehe
    control = rtc.readRegister(CONTROL_REG);

    cout << "Status Register (Hex): 0x" << hex << (int)status << dec << endl;
    cout << "Control Register (Hex): 0x" << hex << (int)control << dec << endl;

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
    sleep(60);  // Wait for another 60 seconds to trigger alarm

    cout << "\nChecking if alarm triggered:" << endl;

    status = rtc.readRegister(STATUS_REG);
    control = rtc.readRegister(CONTROL_REG);

    cout << "Status Register (Hex): 0x" << hex << (int)status << dec << endl; // 0x2 == 0000010 will indicate that alarm2 is triggered successfully
    cout << "Control Register (Hex): 0x" << hex << (int)control << dec << endl;

    if(status & 0x02) {
        cout << "Alarm 2 triggered!" << endl;
        cout << "Current time: ";
        rtc.readTimeDate();

        // Clear the alarm flag
        rtc.writeRegister(STATUS_REG, status & ~0x02);
    } else {
        cout << "Alarm 2 did not trigger as expected" << endl;
    }

    gpioTerminate();

    return 0;
}

