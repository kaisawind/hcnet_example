
#pragma once
#include "common.h"
#include <HCNetSDK.h>

class HCNet {
public:
	HCNet();
	~HCNet();
	void Login();
	void LoginResult(LONG lUserID, DWORD dwResult, LPNET_DVR_DEVICEINFO_V30 lpDeviceInfo);

private:
	DWORD Init();

private:
	LONG lUserID_;
	LPNET_DVR_DEVICEINFO_V30 lpDeviceInfo_;
};