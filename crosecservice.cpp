// crosecservice.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <Windows.h>
#include <cstdio>
#include <thread>
#include "croskbhid.h"
#include "KeyboardBacklight.h"

#include <SetupAPI.h>

BOOL isTabletConvertible;

void checkConvertible() {
    isTabletConvertible = FALSE;

    HDEVINFO hdevinfo = SetupDiGetClassDevsW(NULL, LR"(ACPI\GOOG0006)",
        NULL, DIGCF_ALLCLASSES);
    if (hdevinfo == INVALID_HANDLE_VALUE)
    {
        DWORD err = GetLastError();
        return;
    }

    SP_DEVINFO_DATA devinfo;
    devinfo.cbSize = sizeof(devinfo);
    if (!SetupDiEnumDeviceInfo(hdevinfo, 0, &devinfo))
    {
        DWORD err = GetLastError();
        return;
    }

    isTabletConvertible = TRUE;
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR     lpCmdLine, int       nShowCmd)
{
    checkConvertible();

    pcroskbhid_client client = croskbhid_alloc();
    BOOL connect = croskbhid_connect(client);
    printf("Connected? %d\n", connect);
    if (!connect) {
        return 0;
    }

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