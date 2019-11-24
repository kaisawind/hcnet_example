
#pragma once

#include "common.h"
#include <HCNetSDK.h>
#include <map>

typedef struct ChannelInfo_tag {
    std::string sName;
    std::string sIpV4;
    BYTE byEnable;
    DWORD dwIndex;
} ChannelInfo, *LPChannelInfo;

class HCNet {
public:
    enum HCNetErrors {
        kOK = 0,
        kXmlError = 1,
    };
public:
    HCNet();

    ~HCNet();

    DWORD Login(std::string sDeviceAddress, WORD wPort, std::string sUserName, std::string sPassword);

    DWORD RealPlay(LONG lChannel);

private:
    DWORD Init();

    void GetDeviceInfo();

    DWORD GetDeviceConfig();

    DWORD GetIPParaConfig();

    DWORD GetPtzAbility(DWORD dwChannelIndex);

    DWORD ParsePtzAbility(char *pOutBuf);

    DWORD GetPictureConfig(DWORD dwChannelIndex, LPNET_DVR_PICCFG_V40 lpPicCfg);

    void DVRType(BYTE byDVRType);

private:
    LONG lUserID_{-1};
    LONG lRealHandle_{-1};
    LPNET_DVR_DEVICEINFO_V40 lpDeviceInfo_{nullptr};
    LPNET_DVR_DEVICECFG_V40 lpDeviceCfg_{nullptr};
    LPNET_DVR_IPPARACFG_V40 lpIPParaCfg_{nullptr};
    std::map<DWORD, LPChannelInfo> mapChannelInfo_;
};