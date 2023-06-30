class KeyboardBacklight {
public:
    INT IdleTimeout = 5000;

private:
    BOOL HasKeyboardBacklight = FALSE;
    BOOL IsLaptopMode = TRUE;
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

    BOOL CheckIsLaptopMode();
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