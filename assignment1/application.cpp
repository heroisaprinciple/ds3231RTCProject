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
 
     // Your application code here
     cout << "DS3231 RTC Code for Assignment 1 EEN1071" << endl;
     DS3231 rtc(1, RTC_ADDR);
 
     cout << "Setting time!" << endl;
     rtc.setTimeDate();
 
     cout << "Reading time! ";
     rtc.readTimeDate();
 
     cout << "Setting temperature! " << endl;
     rtc.readTemperature();
 
     cout << "Setting alarms! " << endl;
     rtc.setAlarmOne(16, 52, 00, false, false, 1, 1);

     cout << "Reading alarms!" << endl;
     rtc.readAlarmOne();
 
     cout << "Checking alarm statuses on Status register..." << endl;
     rtc.checkAlarmsStatus();
 
     cout << "end" << endl;
     return 0;
 }
 
 