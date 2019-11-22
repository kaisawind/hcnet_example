
#pragma once
#include "common.h"
#include <HCNetSDK.h>

class HCNet {
public:
	HCNet();
	~HCNet();
	void Login();

private:
	DWORD Init();
	void DeviceInfo();
    void DeviceInfoDVRType(BYTE byDVRType);

private:
    LONG lUserID_{-1};
	LPNET_DVR_DEVICEINFO_V40 lpDeviceInfo_{nullptr};
};