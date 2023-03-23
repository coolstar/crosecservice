// crosecservice.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <Windows.h>
#include <cstdio>
#include <thread>
#include "croskbhid.h"

BOOL supportsKBLight;
INT keyboardBacklight = 100;
BOOL kblightEnabled = TRUE;

class KeyboardBacklight {
public:
    INT IdleTimeout = 5000;

private:
    BOOL HasKeyboardBacklight = FALSE;
    INT InternalBrightness = 100;
    INT CurrentBrightness = 100;
    INT BrightnessFadeStep = 0;
    INT BrightnessFadeTarget = 100;
    BOOL BacklightEnabled = TRUE;

    std::thread task;

    pcroskbhid_client client;

public:
    KeyboardBacklight(pcroskbhid_client client);
    ~KeyboardBacklight();

    BOOL GetBacklightEnabled();
    void SetBacklightEnabled(BOOL enabled);
    INT GetBrightness();
    void FadeSet(INT brightness);
    void BrightnessInc();
    void BrightnessDec();

private:
    void FadeSetInternal(INT brightness);
    void FadeBrightnessStep();
    void InstantlySet(INT Brightness);
};

KeyboardBacklight::KeyboardBacklight(pcroskbhid_client client) {
    this->client = client;
    this->HasKeyboardBacklight = croskblight_connect(client);

    if (this->HasKeyboardBacklight) {
        this->task = std::thread(&KeyboardBacklight::FadeBrightnessStep, this);
    }

    this->InstantlySet(this->InternalBrightness);
}

KeyboardBacklight::~KeyboardBacklight() {
    if (this->HasKeyboardBacklight) {
        this->HasKeyboardBacklight = FALSE;
        this->task.join();

        croskblight_disconnect(client);
    }
}

BOOL KeyboardBacklight::GetBacklightEnabled() {
    return BacklightEnabled;
}

void KeyboardBacklight::SetBacklightEnabled(BOOL enabled) {
    this->BacklightEnabled = enabled;
    this->InstantlySet(this->InternalBrightness);
}

INT KeyboardBacklight::GetBrightness() {
    return this->CurrentBrightness;
}

void KeyboardBacklight::FadeSetInternal(INT brightness) {
    this->BrightnessFadeStep = abs(this->InternalBrightness - brightness) / 6; //do over 6 steps[300 ms]
    this->BrightnessFadeTarget = brightness;
}

void KeyboardBacklight::FadeSet(INT brightness) {
    this->CurrentBrightness = brightness;
    this->BacklightEnabled = TRUE;
    this->FadeSetInternal(brightness);
}

void KeyboardBacklight::BrightnessInc() {
    INT DesiredBrightness = this->BrightnessFadeTarget;
    DesiredBrightness += 10;
    if (DesiredBrightness > 100) {
        DesiredBrightness = 100;
    }
    this->FadeSet(DesiredBrightness);
}

void KeyboardBacklight::BrightnessDec() {
    INT DesiredBrightness = this->BrightnessFadeTarget;
    DesiredBrightness -= 10;
    if (DesiredBrightness < 0) {
        DesiredBrightness = 0;
    }
    this->FadeSet(DesiredBrightness);
}

void KeyboardBacklight::FadeBrightnessStep() {
    while (this->HasKeyboardBacklight) {
        Sleep(50);

        if (this->InternalBrightness == this->BrightnessFadeTarget) {
            LASTINPUTINFO lastInput;
            lastInput.cbSize = sizeof(lastInput);
            GetLastInputInfo(&lastInput);

            DWORD currentTick = GetTickCount();
            if (currentTick - lastInput.dwTime > this->IdleTimeout || lastInput.dwTime > currentTick) {
                if (this->BacklightEnabled)
                    this->FadeSetInternal(0);
            }
            else if (this->CurrentBrightness != 0 && this->InternalBrightness == 0) {
                if (this->BacklightEnabled)
                    this->FadeSetInternal(this->CurrentBrightness);
            }

            continue;
        }

        INT DesiredBrightness = this->InternalBrightness;
        INT BrightnessFadeTarget = this->BrightnessFadeTarget;
        INT BrightnessFadeStep = this->BrightnessFadeStep;

        if ((DesiredBrightness < BrightnessFadeTarget) && (DesiredBrightness + BrightnessFadeStep) > BrightnessFadeTarget)
        {
            DesiredBrightness = BrightnessFadeTarget;
        }
        else if ((DesiredBrightness > BrightnessFadeTarget) && (DesiredBrightness - BrightnessFadeStep) < BrightnessFadeTarget)
        {
            DesiredBrightness = BrightnessFadeTarget;
        }
        else if (DesiredBrightness < BrightnessFadeTarget) {
            DesiredBrightness = DesiredBrightness + BrightnessFadeStep;
        }
        else if (DesiredBrightness > BrightnessFadeTarget) {
            DesiredBrightness = DesiredBrightness - BrightnessFadeStep;
        }

        this->InstantlySet(DesiredBrightness);
    }
}

void KeyboardBacklight::InstantlySet(INT brightness) {
    if (brightness > 100)
        brightness = 100;
    else if (brightness < 0)
        brightness = 0;
    this->InternalBrightness = brightness;

    CrosKBLightSettingsReport report = { 0 };
    report.ReportID = REPORTID_KBLIGHT;
    report.SetBrightness = 1;
    report.Brightness = this->BacklightEnabled ? brightness : 0;

    croskblight_write_message(client, &report);
}


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