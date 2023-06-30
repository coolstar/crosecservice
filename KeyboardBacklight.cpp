#include <Windows.h>
#include <cstdio>
#include <thread>
#include "croskbhid.h"
#include "KeyboardBacklight.h"

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
    INT TimerAdjustment = 0;
    while (this->HasKeyboardBacklight) {
        if ((50 - TimerAdjustment) > 0) {
            Sleep(50 - TimerAdjustment);
        }

        ULONGLONG Start = GetTickCount64();

        if (this->InternalBrightness == this->BrightnessFadeTarget &&
            this->IsLaptopMode == CheckIsLaptopMode()) {
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

            TimerAdjustment = (INT)(GetTickCount64() - Start);

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

        TimerAdjustment = (INT)(GetTickCount64() - Start);
    }
}

extern BOOL isTabletConvertible;

BOOL KeyboardBacklight::CheckIsLaptopMode() {
    if (!isTabletConvertible) {
        return TRUE;
    }
    return (GetSystemMetrics(SM_CONVERTIBLESLATEMODE) != 0);
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
    
    this->IsLaptopMode = CheckIsLaptopMode();
    if (!this->IsLaptopMode) {
        report.Brightness = 0;
    }

    croskblight_write_message(client, &report);
}