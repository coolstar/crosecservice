// crosecservice.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <Windows.h>
#include <cstdio>
#include <thread>
#include "croskbhid.h"
#include "KeyboardBacklight.h"

int main()
{
    pcroskbhid_client client = croskbhid_alloc();
    BOOL connect = croskbhid_connect(client);
    printf("Connected? %d\n", connect);

    KeyboardBacklight backlight(client);

    CrosKBHIDRemapperMediaReport report = { 0 };

    while (TRUE) {
        croskbhid_read_keyboard(client, &report);

        if (report.ReportID == REPORTID_MEDIA) {
            if (report.ControlCode == 0)
                continue;

            if (report.ControlCode & CROSKBHID_KBLT_UP) {
                backlight.BrightnessInc();
            }
            if (report.ControlCode & CROSKBHID_KBLT_DN) {
                backlight.BrightnessDec();
            }
            if (report.ControlCode & CROSKBHID_KBLT_TOGGLE) {
                backlight.SetBacklightEnabled(!backlight.GetBacklightEnabled());
            }
        }
    }

    croskbhid_disconnect(client);
}