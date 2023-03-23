#if !defined(_CROSKBHIDREMAPPER_COMMON_H_)
#define _CROSKBHIDREMAPPER_COMMON_H_

typedef struct _croskbhid_client_t* pcroskbhid_client;


//
//These are the device attributes returned by vmulti in response
// to IOCTL_HID_GET_DEVICE_ATTRIBUTES.
//

#define CROSKBHIDREMAPPER_PID              0x0303
#define CROSKBHIDREMAPPER_VID              0x18D1
#define CROSKBHIDREMAPPER_VERSION          0x0004

#define CROSKBLIGHT_PID              0x0002
#define CROSKBLIGHT_VID              0x18D1
#define CROSKBLIGHT_VERSION          0x0001

//
// These are the report ids
//

#define REPORTID_KBLIGHT       0x01

#define REPORTID_KEYBOARD       0x07
#define REPORTID_MEDIA          0x08
#define REPORTID_SETTINGS		0x09

//
// Keyboard specific report infomation
//

#define CROSKBHID_BRIGHTNESS_UP 0x01
#define CROSKBHID_BRIGHTNESS_DN 0x02
#define CROSKBHID_KBLT_UP       0x04
#define CROSKBHID_KBLT_DN       0x08
#define CROSKBHID_KBLT_TOGGLE   0x10

#pragma pack(1)
typedef struct _CROSKBHIDREMAPPER_MEDIA_REPORT
{

	BYTE      ReportID;

	BYTE	  ControlCode;

	BYTE	  Reserved;

} CrosKBHIDRemapperMediaReport;

#pragma pack()

#pragma pack()

//
// Feature report infomation
//

#define DEVICE_MODE_MOUSE        0x00
#define DEVICE_MODE_SINGLE_INPUT 0x01
#define DEVICE_MODE_MULTI_INPUT  0x02

#pragma pack(1)
typedef struct _CROSKBHIDREMAPPER_FEATURE_REPORT
{

	BYTE      ReportID;

	BYTE      DeviceMode;

	BYTE      DeviceIdentifier;

} CrosKBHIDRemapperFeatureReport;

#pragma pack(1)
typedef struct _CROSKBHIDREMAPPER_SETTINGS_REPORT
{

	BYTE        ReportID;

	BYTE		SettingsRegister;

	BYTE		SettingsValue;

} CrosKBHIDRemapperSettingsReport;
#pragma pack()

#pragma pack(1)
typedef struct _CROSKBLIGHT_GETLIGHT_REPORT
{

	BYTE        ReportID;

	BYTE		Brightness;

} CrosKBLightGetLightReport;
#pragma pack()

#pragma pack(1)
typedef struct _CROSKBLIGHT_SETTINGS_REPORT
{

	BYTE        ReportID;

	BYTE		SetBrightness;

	BYTE		Brightness;

} CrosKBLightSettingsReport;
#pragma pack()

pcroskbhid_client croskbhid_alloc(void);

void croskbhid_free(pcroskbhid_client vmulti);

BOOL croskbhid_connect(pcroskbhid_client vmulti);

void croskbhid_disconnect(pcroskbhid_client vmulti);

BOOL croskblight_connect(pcroskbhid_client croskbhid);
void croskblight_disconnect(pcroskbhid_client croskbhid);

BOOL croskbhid_read_keyboard(pcroskbhid_client vmulti, CrosKBHIDRemapperMediaReport* pReport);

BOOL croskbhid_write_keyboard(pcroskbhid_client vmulti, CrosKBHIDRemapperMediaReport* pReport);

BOOL croskblight_read_message(pcroskbhid_client vmulti, CrosKBLightSettingsReport* pReport);

BOOL croskblight_write_message(pcroskbhid_client vmulti, CrosKBLightSettingsReport* pReport);

#endif
#pragma once
