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

void interruptCallback(int gpio, int level, uint32_t tick, void * userData) {
    DS3231 *rtc = (DS3231*)userData;
    // Trigger LED when interrupts happen
    rtc->triggerLED();
}

void blinkLED(DS3231 &rtc) {
    cout << "\nDemonstrating Square Wave Functionality using PWM:" << endl;

    // pigpio functitons
    gpioSetMode(LED_PIN, PI_OUTPUT);
    gpioSetPWMrange(LED_PIN, 100);
    gpioSetPWMfrequency(LED_PIN, 800);

    // 1Hz - LED flickering with 100% duty cycle
    cout << "\n1Hz - LED at high brightness" << endl;
    rtc.enableSQW(1);
    gpioPWM(LED_PIN, 100);
    sleep(5);

    // 1.024kHz - LED flickering with 75% duty cycle
    cout << "\n1.024kHz - LED at medium brightness" << endl;
    rtc.enableSQW(1024);
    gpioPWM(LED_PIN, 75);
    sleep(5);

    // 4.096kHz - LED flickering with 50% duty cycle
    cout << "\n4.096kHz - LED dimmer" << endl;
    rtc.enableSQW(4096);
    gpioPWM(LED_PIN, 50);
    sleep(5);

    // 8.192kHz - LED flickering with 25% duty cycle
    cout << "\n8.192kHz - LED very dim" << endl;
    rtc.enableSQW(8192);
    gpioPWM(LED_PIN, 25);
    sleep(5);

    // Turn off PWM
    gpioPWM(LED_PIN, 0);

    rtc.disableSQW();
}

int main() {
    DS3231 rtc(1, RTC_ADDR);
    unsigned char status;
    unsigned char control;

    if (gpioInitialise() < 0) {
        perror("Can't initialize pigpio.");
        return 1;
    }

    gpioSetMode(INT_SQW_PIN, PI_INPUT);
    gpioSetMode(LED_PIN, PI_OUTPUT);
    // Enable interrupts, pigpio needs callback
    gpioSetAlertFuncEx(INT_SQW_PIN, interruptCallback, &rtc);

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

    // Disable interrupts to enable SQW; otherwise interrupts and SQW will be at conflict
    gpioSetAlertFuncEx(INT_SQW_PIN, NULL, NULL);

    // LED flickering via SQW and PWM
    blinkLED(rtc);

    // Return to interrupts!
    gpioSetAlertFuncEx(INT_SQW_PIN, interruptCallback, &rtc);

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


