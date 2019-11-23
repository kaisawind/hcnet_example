
#pragma once
#include "common.h"
#include <HCNetSDK.h>

class HCNet {
public:
    enum HCNetErrors {
        kOK = 0,
        kXmlError = 1,
    };
public:
	HCNet();
	~HCNet();
    DWORD Login();
    DWORD RealPlay();

private:
	DWORD Init();
	void GetDeviceInfo();
    DWORD GetDeviceConfig();
    DWORD GetIPParaConfig();
    DWORD GetPtzAbility();
    DWORD ParsePtzAbility(char* pOutBuf);
    void DVRType(BYTE byDVRType);

private:
    LONG lUserID_{ -1 };
    LONG lRealHandle_{ -1 };
	LPNET_DVR_DEVICEINFO_V40 lpDeviceInfo_{ nullptr };
    LPNET_DVR_DEVICECFG_V40 lpDeviceCfg_{ nullptr };
    LPNET_DVR_IPPARACFG_V40 lpIPParaCfg_{ nullptr };
};