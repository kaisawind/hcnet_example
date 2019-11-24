#include "hc_net.h"

// kPtzAbility 获取设备PTZ能力时pInBuf参数描述
const char *kPtzAbility = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                          "<!--req, 获取设备PTZ能力时pInBuf参数描述-->"
                          "<!--req, PTZAbility：花样扫描、云台守望等PTZ能力-->"
                          "<PTZAbility version= \"2.0\">"
                          "  <channelNO>"
                          "    %d<!--req,通道号-->"
                          "  </channelNO>"
                          "</PTZAbility>";

void CALLBACK fRealDataCallBack_V30(LONG lPlayHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, void *pUser) {
    auto *self = (HCNet *) pUser;

    switch (dwDataType) {
        case NET_DVR_SYSHEAD:
            spdlog::debug("系统头数据");
            break;
        case NET_DVR_STREAMDATA:
            spdlog::debug("流数据（包括复合流或音视频分开的视频流数据）");
            break;
        case NET_DVR_AUDIOSTREAMDATA:
            spdlog::debug("音频数据");
            break;
        case NET_DVR_PRIVATE_DATA:
            spdlog::debug("私有数据,包括智能信息");
            break;
        default:
            break;
    }
}

void CALLBACK fExceptionCallBack(DWORD dwType, LONG lUserID, LONG lHandle, void *pUser) {
    auto *self = (HCNet *) pUser;

    switch (dwType) {
        case EXCEPTION_RECONNECT:
            spdlog::error("预览时重连 {} {}", lUserID, dwType);
            break;
        case PREVIEW_RECONNECTSUCCESS:
            spdlog::info("预览时重连成功 {} {}", lUserID, dwType);
            break;
        case EXCEPTION_PREVIEW:
            spdlog::error("网络预览异常 {} {}", lUserID, dwType);
            break;
        case EXCEPTION_PLAYBACK:
            spdlog::error("回放异常 {} {}", lUserID, dwType);
            break;
        case NETWORK_FLOWTEST_EXCEPTION:
            spdlog::error("网络流量检测异常 {} {}", lUserID, dwType);
            break;
        case EXCEPTION_SERIAL:
            spdlog::error("透明通道异常 {} {}", lUserID, dwType);
            break;
        default:
            spdlog::error("fExceptionCallBack {} {}", lUserID, dwType);
            break;
    }
}

HCNet::HCNet() {
    this->lUserID_ = -1;
    this->lpDeviceInfo_ = new NET_DVR_DEVICEINFO_V40;
    memset(this->lpDeviceInfo_, 0, sizeof(NET_DVR_DEVICEINFO_V40));

    this->lpDeviceCfg_ = new NET_DVR_DEVICECFG_V40;
    memset(this->lpDeviceCfg_, 0, sizeof(NET_DVR_DEVICECFG_V40));

    this->lpIPParaCfg_ = new NET_DVR_IPPARACFG_V40;
    memset(this->lpIPParaCfg_, 0, sizeof(NET_DVR_IPPARACFG_V40));

    // NVR init
    this->Init();
}

HCNet::~HCNet() {
    spdlog::info("Destroy HCNet");

    if (this->lRealHandle_ != -1) {
        // 停止预览。
        BOOL ret = NET_DVR_StopRealPlay(this->lRealHandle_);
        if (!ret) {
            // 返回最后操作的错误码。
            DWORD lError = NET_DVR_GetLastError();
            LONG lErrorNo = LONG(lError);
            // 返回最后操作的错误码信息。
            spdlog::error("NET_DVR_StopRealPlay: {} {}", lError, NET_DVR_GetErrorMsg(&lErrorNo));
            // return;
        }
        spdlog::info("NET_DVR_StopRealPlay OK {}", this->lRealHandle_);
    }

    if (this->lUserID_ != -1) {
        // 用户注销。
        BOOL ret = NET_DVR_Logout(this->lUserID_);
        if (!ret) {
            // 返回最后操作的错误码。
            DWORD lError = NET_DVR_GetLastError();
            LONG lErrorNo = LONG(lError);
            // 返回最后操作的错误码信息。
            spdlog::error("NET_DVR_Logout: {} {}", lError, NET_DVR_GetErrorMsg(&lErrorNo));
            // return;
        }
        spdlog::info("NET_DVR_Logout OK {}", this->lUserID_);
    }
    // 释放SDK资源，在程序结束之前调用。
    BOOL ret = NET_DVR_Cleanup();
    if (!ret) {
        // 返回最后操作的错误码。
        DWORD lError = NET_DVR_GetLastError();
        LONG lErrorNo = LONG(lError);
        // 返回最后操作的错误码信息。
        spdlog::error("NET_DVR_Cleanup: {} {}", lError, NET_DVR_GetErrorMsg(&lErrorNo));
        // return;
    }
    spdlog::info("NET_DVR_Cleanup OK");

    delete this->lpDeviceCfg_;
    spdlog::info("free lpDeviceCfg_ OK");
    delete this->lpDeviceInfo_;
    spdlog::info("free lpDeviceInfo_ OK");
    delete this->lpIPParaCfg_;
    spdlog::info("free lpIPParaCfg_ OK");
    for (auto &v : this->mapChannelInfo_) {
        if (v.second != nullptr) {
            delete v.second;
            spdlog::info("free channel {} OK", v.first);
        }
    }
    spdlog::info("HC Net DVR Destroy");
}

DWORD HCNet::Init() {
    spdlog::info("HC Net DVR Init");
    // 初始化SDK，调用其他SDK函数的前提。
    BOOL ret = NET_DVR_Init();
    if (!ret) {
        // 返回最后操作的错误码。
        DWORD lError = NET_DVR_GetLastError();
        LONG lErrorNo = LONG(lError);
        // 返回最后操作的错误码信息。
        spdlog::error("NET_DVR_Init: {} {}", lError, NET_DVR_GetErrorMsg(&lErrorNo));
        return lError;
    }

    // 获取SDK的版本信息。
    DWORD dVer = NET_DVR_GetSDKVersion();
    DWORD dLowVer = dVer & 0x0000FFFF;
    DWORD dHighVer = (dVer & 0xFFFF0000) >> 16;
    spdlog::info("NET DVR SDK Version {:x} {}.{}", dVer, dHighVer, dLowVer);

    NET_DVR_SDKSTATE sdkState = {0};
    ret = NET_DVR_GetSDKState(&sdkState);
    if (!ret) {
        // 返回最后操作的错误码。
        DWORD lError = NET_DVR_GetLastError();
        LONG lErrorNo = LONG(lError);
        // 返回最后操作的错误码信息。
        spdlog::error("NET_DVR_GetSDKState: {} {}", lError, NET_DVR_GetErrorMsg(&lErrorNo));
        return lError;
    }
    spdlog::info("当前login用户数:                     {}", sdkState.dwTotalLoginNum);
    spdlog::info("当前realplay路数:                    {}", sdkState.dwTotalRealPlayNum);
    spdlog::info("当前回放或下载路数:                  {}", sdkState.dwTotalPlayBackNum);
    spdlog::info("当前建立报警通道路数:                {}", sdkState.dwTotalAlarmChanNum);
    spdlog::info("当前硬盘格式化路数:                  {}", sdkState.dwTotalFormatNum);
    spdlog::info("当前文件搜索路数:                    {}", sdkState.dwTotalFileSearchNum);
    spdlog::info("当前日志搜索路数:                    {}", sdkState.dwTotalLogSearchNum);
    spdlog::info("当前透明通道路数:                    {}", sdkState.dwTotalSerialNum);
    spdlog::info("当前升级路数:                        {}", sdkState.dwTotalUpgradeNum);
    spdlog::info("当前语音转发路数:                    {}", sdkState.dwTotalVoiceComNum);
    spdlog::info("当前语音广播路数:                    {}", sdkState.dwTotalBroadCastNum);
    spdlog::info("当前网络监听路数:                    {}", sdkState.dwTotalListenNum);
    spdlog::info("当前邮件计数路数:                    {}", sdkState.dwEmailTestNum);
    spdlog::info("当前文件备份路数:                    {}", sdkState.dwBackupNum);
    spdlog::info("当前审讯上传路数:                    {}", sdkState.dwTotalInquestUploadNum);

    NET_DVR_SDKABL adkAbl = {0};
    ret = NET_DVR_GetSDKAbility(&adkAbl);
    if (!ret) {
        // 返回最后操作的错误码。
        DWORD lError = NET_DVR_GetLastError();
        LONG lErrorNo = LONG(lError);
        // 返回最后操作的错误码信息。
        spdlog::error("NET_DVR_GetSDKAbility: {} {}", lError, NET_DVR_GetErrorMsg(&lErrorNo));
        return lError;
    }
    spdlog::info("最大login用户数      MAX_LOGIN_USERS {}", adkAbl.dwMaxLoginNum);
    spdlog::info("最大realplay路数     WATCH_NUM       {}", adkAbl.dwMaxRealPlayNum);
    spdlog::info("最大回放或下载路数   WATCH_NUM       {}", adkAbl.dwMaxPlayBackNum);
    spdlog::info("最大建立报警通道路数 ALARM_NUM       {}", adkAbl.dwMaxAlarmChanNum);
    spdlog::info("最大硬盘格式化路数   SERVER_NUM      {}", adkAbl.dwMaxFormatNum);
    spdlog::info("最大文件搜索路数     SERVER_NUM      {}", adkAbl.dwMaxFileSearchNum);
    spdlog::info("最大日志搜索路数     SERVER_NUM      {}", adkAbl.dwMaxLogSearchNum);
    spdlog::info("最大透明通道路数     SERVER_NUM      {}", adkAbl.dwMaxSerialNum);
    spdlog::info("最大升级路数         SERVER_NUM      {}", adkAbl.dwMaxUpgradeNum);
    spdlog::info("最大语音转发路数     SERVER_NUM      {}", adkAbl.dwMaxVoiceComNum);
    spdlog::info("最大语音广播路数     MAX_CASTNUM     {}", adkAbl.dwMaxBroadCastNum);
    return kOK;
}

DWORD HCNet::Login(std::string sDeviceAddress, WORD wPort, std::string sUserName, std::string sPassword) {
    NET_DVR_USER_LOGIN_INFO struLoginInfo = {0 };
    strcpy(struLoginInfo.sDeviceAddress, sDeviceAddress.c_str());   // sDeviceAddress 设备地址，IP 或者普通域名
    struLoginInfo.byUseTransport = 1;                               // byUseTransport 是否启用能力集透传：0- 不启用透传，默认；1- 启用透传
    struLoginInfo.wPort = wPort;                                    // wPort		  设备端口号，例如：8000
    strcpy(struLoginInfo.sUserName, sUserName.c_str());             // sUserName	  登录用户名，例如：admin
    strcpy(struLoginInfo.sPassword, sPassword.c_str());             // sPassword	  登录密码，例如：12345
    struLoginInfo.cbLoginResult = nullptr;                          // cbLoginResult  登录状态回调函数，bUseAsynLogin 为1时有效
    struLoginInfo.pUser = nullptr;                                  // pUser		  用户数据
    struLoginInfo.bUseAsynLogin = FALSE;                            // bUseAsynLogin  是否异步登录：0- 否，1- 是
    LONG lUserID = NET_DVR_Login_V40(&struLoginInfo, this->lpDeviceInfo_);
    if (lUserID == -1) {
        // 返回最后操作的错误码。
        DWORD lError = NET_DVR_GetLastError();
        LONG lErrorNo = LONG(lError);
        // 返回最后操作的错误码信息。
        spdlog::error("NET_DVR_Login_V40: {} {}", lError, NET_DVR_GetErrorMsg(&lErrorNo));
        return lError;
    }
    this->lUserID_ = lUserID;
    this->GetDeviceInfo();

    spdlog::info("NET_DVR_Login_V40 OK {}", lUserID);

    BOOL ret = NET_DVR_SetExceptionCallBack_V30(0, nullptr, fExceptionCallBack, this);
    if (!ret) {
        // 返回最后操作的错误码。
        DWORD lError = NET_DVR_GetLastError();
        LONG lErrorNo = LONG(lError);
        // 返回最后操作的错误码信息。
        spdlog::error("NET_DVR_SetExceptionCallBack_V30: {} {}", lError, NET_DVR_GetErrorMsg(&lErrorNo));
        return lError;
    }

    DWORD error = this->GetDeviceConfig();
    if (error) {
        spdlog::error("Get Device Config error {}", error);
        return error;
    }
    error = this->GetIPParaConfig();
    if (error) {
        spdlog::error("Get IP Para error {}", error);
        return error;
    }
    return kOK;
}

DWORD HCNet::GetPictureConfig(DWORD dwChannelIndex, LPNET_DVR_PICCFG_V40 lpPicCfg) {
    // 获取图像参数
    DWORD dwReturned = 0;
    BOOL ret = NET_DVR_GetDVRConfig(this->lUserID_, NET_DVR_GET_PICCFG_V40, dwChannelIndex, lpPicCfg,
                                    sizeof(NET_DVR_PICCFG_V40), &dwReturned);
    if (!ret) {
        // 返回最后操作的错误码。
        DWORD lError = NET_DVR_GetLastError();
        LONG lErrorNo = LONG(lError);
        // 返回最后操作的错误码信息。
        spdlog::error("NET_DVR_GetDVRConfig: NET_DVR_GET_PICCFG {} {}", lError, NET_DVR_GetErrorMsg(&lErrorNo));
        return lError;
    }
    spdlog::info("NET_DVR_GetDVRConfig NET_DVR_GET_PICCFG {} {}", dwChannelIndex, lpPicCfg->sChanName);
    return kOK;
}

DWORD HCNet::ParsePtzAbility(char *pOutBuf) {
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError error = doc.Parse(pOutBuf);
    if (!error) {
        spdlog::error("Ptz Ability xml parser error", tinyxml2::XMLDocument::ErrorIDToName(error));
        return kXmlError;
    }
    const char *ability = doc.RootElement()->Value();
    if (strncmp(ability, "PTZAbility", sizeof("PTZAbility")) != 0) {
        spdlog::warn("Not PTZ Ability");
        return kXmlError;
    }
    const char *controlTypeOpt = doc.FirstChildElement("PTZAbility")->FirstChildElement(
            "PTZControl")->FirstChildElement("controlType")->Attribute("opt");
    std::string sOpt(controlTypeOpt);
    // sOpt.find("");
    spdlog::info("control type opt {}", sOpt);
    return kOK;
}

DWORD HCNet::GetPtzAbility(DWORD dwChannelIndex) {
    char pInBuf[512] = "";
    char pOutBuf[1024 * 1024] = "";
    sprintf(pInBuf, kPtzAbility, dwChannelIndex);
    BOOL ret = NET_DVR_GetDeviceAbility(this->lUserID_, DEVICE_ABILITY_INFO, pInBuf, sizeof(pInBuf), pOutBuf,
                                        sizeof(pOutBuf));
    if (!ret) {
        // 返回最后操作的错误码。
        DWORD lError = NET_DVR_GetLastError();
        LONG lErrorNo = LONG(lError);
        // 返回最后操作的错误码信息。
        spdlog::error("{} NET_DVR_GetDeviceAbility PTZ_ABILITY: {} {}", dwChannelIndex, lError,
                      NET_DVR_GetErrorMsg(&lErrorNo));
        return lError;
    }
    DWORD error = this->ParsePtzAbility(pOutBuf);
    if (!error) {
        return error;
    }
    return kOK;
}

DWORD HCNet::GetDeviceConfig() {
    // 获取设备的配置信息。
    DWORD dwReturned = 0;
    BOOL ret = NET_DVR_GetDVRConfig(this->lUserID_, NET_DVR_GET_DEVICECFG_V40, 0xFFFFFFFF, this->lpDeviceCfg_,
                                    sizeof(NET_DVR_DEVICECFG_V40), &dwReturned);
    if (!ret) {
        // 返回最后操作的错误码。
        DWORD lError = NET_DVR_GetLastError();
        LONG lErrorNo = LONG(lError);
        // 返回最后操作的错误码信息。
        spdlog::error("NET_DVR_GetDVRConfig: NET_DVR_GET_DEVICECFG_V40 {} {}", lError, NET_DVR_GetErrorMsg(&lErrorNo));
        return lError;
    }
    LPNET_DVR_DEVICECFG_V40 lpDeviceCfg = this->lpDeviceCfg_;

    spdlog::info("设备名称       {}", lpDeviceCfg->sDVRName);
    spdlog::info("设备ID号       {}", lpDeviceCfg->dwDVRID);
    spdlog::info("设备名称        {}", lpDeviceCfg->sSerialNumber);
    spdlog::info("设备232串口个数  {}", lpDeviceCfg->byRS232Num);
    spdlog::info("设备485串口个数  {}", lpDeviceCfg->byRS485Num);
    spdlog::info("网络口个数       {}", lpDeviceCfg->byNetworkPortNum);
    spdlog::info("硬盘控制器个数    {}", lpDeviceCfg->byDiskCtrlNum);
    spdlog::info("硬盘个数          {}", lpDeviceCfg->byDiskNum);
    this->DVRType(lpDeviceCfg->byDVRType);
    spdlog::info("设备模拟通道个数    {}", lpDeviceCfg->byChanNum);
    spdlog::info("模拟通道的起始通道号 {}", lpDeviceCfg->byStartChan);

    spdlog::info("NET_DVR_GetDVRConfig NET_DVR_GET_DEVICECFG_V40 OK {}", dwReturned);
    return kOK;
}

DWORD HCNet::GetIPParaConfig() {
    // 获取IP接入配置参数
    DWORD dwReturned = 0;
    BOOL ret = NET_DVR_GetDVRConfig(this->lUserID_, NET_DVR_GET_IPPARACFG_V40, 0, this->lpIPParaCfg_,
                                    sizeof(NET_DVR_IPPARACFG_V40), &dwReturned);
    if (!ret) {
        // 返回最后操作的错误码。
        DWORD lError = NET_DVR_GetLastError();
        LONG lErrorNo = LONG(lError);
        // 返回最后操作的错误码信息。
        spdlog::error("NET_DVR_GetDVRConfig: NET_DVR_GET_IPPARACFG_V40 {} {}", lError, NET_DVR_GetErrorMsg(&lErrorNo));
        return lError;
    }
    LPNET_DVR_IPPARACFG_V40 lpIPParaCfg = this->lpIPParaCfg_;
    spdlog::info("设备支持的总组数 {}", lpIPParaCfg->dwGroupNum);
    spdlog::info("最大模拟通道个数 {}", lpIPParaCfg->dwAChanNum);
    spdlog::info("数字通道个数    {}", lpIPParaCfg->dwDChanNum);
    spdlog::info("起始数字通道    {}", lpIPParaCfg->dwStartDChan);
    // IP设备
    for (DWORD index = 0; index < lpIPParaCfg->dwDChanNum; index++) {
        LPNET_DVR_IPDEVINFO_V31 lpIPDevInfo = &lpIPParaCfg->struIPDevInfo[index];
        if (!lpIPDevInfo->byEnable) {
            continue;
        }
        DWORD dwChannelIndex = lpIPParaCfg->dwStartDChan + index;
        spdlog::info("---------------------------------");
        switch (lpIPDevInfo->byProType) {
            case 0:
                spdlog::info("协议类型 {}-{}", lpIPDevInfo->byProType, "私有协议");
                break;
            case 1:
                spdlog::info("协议类型 {}-{}", lpIPDevInfo->byProType, "松下协议");
                break;
            case 2:
                spdlog::info("协议类型 {}-{}", lpIPDevInfo->byProType, "索尼");
                break;
            default:
                spdlog::info("协议类型 {}", lpIPDevInfo->byProType);
                break;
        }
        spdlog::info("用户名   {}", lpIPDevInfo->sUserName);
        spdlog::info("密码     {}", lpIPDevInfo->sPassword);
        spdlog::info("设备域名  {}", lpIPDevInfo->byDomain);
        spdlog::info("IPV4地址 {}", lpIPDevInfo->struIP.sIpV4);
        spdlog::info("IPV6地址 {}", lpIPDevInfo->struIP.byIPv6);
        spdlog::info("端口号   {}", lpIPDevInfo->wDVRPort);
        spdlog::info("设备ID   {}", lpIPDevInfo->szDeviceID);
        LPNET_DVR_STREAM_MODE lpStreamMode = &lpIPParaCfg->struStreamMode[index];
        switch (lpStreamMode->byGetStreamType) {
            case 0: {
                spdlog::info("{} 直接从设备取流，对应联合体中结构NET_DVR_IPCHANINFO", dwChannelIndex);
                DWORD error = this->GetPtzAbility(dwChannelIndex);
                if (error) {
                    spdlog::warn("Get Ptz Ability error {}", error);
                }
                NET_DVR_PICCFG_V40 struPicCfg = {0};
                error = this->GetPictureConfig(dwChannelIndex, &struPicCfg);
                if (error) {
                    spdlog::warn("Get Picture Config error {}", error);
                }
                LPChannelInfo lpChannelInfo = new ChannelInfo;
                if (lpStreamMode->uGetStream.struChanInfo.byEnable) {
                    lpChannelInfo->byEnable = 1;
                }
                lpChannelInfo->sIpV4 = (char *) lpIPDevInfo->struIP.sIpV4;
                lpChannelInfo->dwIndex = dwChannelIndex;
                lpChannelInfo->sName = (char *) struPicCfg.sChanName;
                this->mapChannelInfo_[dwChannelIndex] = lpChannelInfo;
            }
                break;
            case 1:
                spdlog::info("{} 从流媒体取流，对应联合体中结构NET_DVR_IPSERVER_STREAM", dwChannelIndex);
                break;
            case 2:
                spdlog::info("{} 通过IPServer获得IP地址后取流，对应联合体中结构NET_DVR_PU_STREAM_CFG", dwChannelIndex);
                break;
            case 3:
                spdlog::info("{} 通过IPServer找到设备，再通过流媒体取设备的流，对应联合体中结构NET_DVR_DDNS_STREAM_CFG", dwChannelIndex);
                break;
            case 4:
                spdlog::info("{} 通过流媒体由URL去取流，对应联合体中结构NET_DVR_PU_STREAM_URL", dwChannelIndex);
                break;
            case 5:
                spdlog::info("{} 通过hiDDNS域名连接设备然后从设备取流，对应联合体中结构NET_DVR_HKDDNS_STREAM", dwChannelIndex);
                break;
            case 6:
                spdlog::info("{} 直接从设备取流(扩展)，对应联合体中结构NET_DVR_IPCHANINFO_V40", dwChannelIndex);
                break;
            default:
                spdlog::info("{} Stream Type {}", dwChannelIndex, lpStreamMode->byGetStreamType);
                break;
        }
    }
    spdlog::info("---------------------------------");
    spdlog::info("NET_DVR_GetDVRConfig NET_DVR_GET_IPPARACFG_V40 OK {}", dwReturned);
    return kOK;
}

void HCNet::GetDeviceInfo() {
    LPNET_DVR_DEVICEINFO_V40 lpDeviceInfo = this->lpDeviceInfo_;
    NET_DVR_DEVICEINFO_V30 struDeviceV30 = lpDeviceInfo->struDeviceV30;
    spdlog::info("序列号                               {}", struDeviceV30.sSerialNumber);
    spdlog::info("报警输入个数                         {}", struDeviceV30.byAlarmInPortNum);
    spdlog::info("报警输出个数                         {}", struDeviceV30.byAlarmOutPortNum);
    spdlog::info("硬盘个数                             {}", struDeviceV30.byDiskNum);
    this->DVRType(struDeviceV30.byDVRType);
    spdlog::info("模拟通道个数                         {}", struDeviceV30.byChanNum);
    spdlog::info("起始通道号                           {}", struDeviceV30.byStartChan);
    spdlog::info("语音通道数                           {}", struDeviceV30.byAudioChanNum);
    spdlog::info("最大数字通道个数,低位                 {}", struDeviceV30.byIPChanNum);
    spdlog::info("零通道编码个数                       {}", struDeviceV30.byZeroChanNum);

    // 主码流传输协议类型
    switch (struDeviceV30.byMainProto) {
        case 0:
            spdlog::info("主码流传输协议类型                   {}-{}", struDeviceV30.byMainProto, "private");
            break;
        case 1:
            spdlog::info("主码流传输协议类型                   {}-{}", struDeviceV30.byMainProto, "rtsp");
            break;
        case 2:
            spdlog::info("主码流传输协议类型                   {}-{}", struDeviceV30.byMainProto, "同时支持private和rtsp");
            break;
        default:
            spdlog::info("主码流传输协议类型                   {}-{}", struDeviceV30.byMainProto, "unknown");
            break;
    }
    // 子码流传输协议类型
    switch (struDeviceV30.bySubProto) {
        case 0:
            spdlog::info("子码流传输协议类型                   {}-{}", struDeviceV30.byMainProto, "private");
            break;
        case 1:
            spdlog::info("子码流传输协议类型                   {}-{}", struDeviceV30.byMainProto, "rtsp");
            break;
        case 2:
            spdlog::info("子码流传输协议类型                   {}-{}", struDeviceV30.byMainProto, "同时支持private和rtsp");
            break;
        default:
            spdlog::info("子码流传输协议类型                   {}-{}", struDeviceV30.byMainProto, "unknown");
            break;
    }
    // 能力，位与结果为0表示不支持，1表示支持，
    spdlog::info("是否支持智能搜索                     {:x}", struDeviceV30.bySupport & 0x1);
    spdlog::info("是否支持备份                         {:x}", (struDeviceV30.bySupport & 0x2) >> 1);
    spdlog::info("是否支持压缩参数能力获取              {:x}", (struDeviceV30.bySupport & 0x3) >> 2);
    spdlog::info("是否支持多网卡                       {:x}", (struDeviceV30.bySupport & 0x4) >> 3);
    spdlog::info("是否支持远程SADP                     {:x}", (struDeviceV30.bySupport & 0x10) >> 4);
    spdlog::info("是否支持Raid卡功能                   {:x}", (struDeviceV30.bySupport & 0x20) >> 5);
    spdlog::info("是否支持IPSAN 目录查找               {:x}", (struDeviceV30.bySupport & 0x40) >> 6);
    spdlog::info("是否支持rtp over rtsp                {:x}", (struDeviceV30.bySupport & 0x80) >> 7);
}

void HCNet::DVRType(BYTE byDVRType) {
    switch (byDVRType) {
        case DVR:
            spdlog::info("设备类型                             {}:{}", byDVRType, "DVR");
            break;
        case ATMDVR:
            spdlog::info("设备类型                             {}:{}", byDVRType, "ATM DVR");
            break;
        case DVS:
            spdlog::info("设备类型                             {}:{}", byDVRType, "DVS");
            break;
        case DS90XX_HF_S:
            spdlog::info("设备类型                             {}:{}", byDVRType, "DS90XX_HF_S");
            break;
        default:
            spdlog::info("设备类型                             {}:{}", byDVRType, "unknown");
            break;
    }
}

DWORD HCNet::RealPlay(LONG lChannel) {
    // 实时预览（支持多码流）。
    NET_DVR_PREVIEWINFO struPreviewInfo = {
            lChannel: lChannel,
            dwStreamType: 1,
            dwLinkMode: 0,
            bBlocked: 1,
    };
    this->lRealHandle_ = NET_DVR_RealPlay_V40(this->lUserID_, &struPreviewInfo, nullptr, nullptr);
    if (this->lRealHandle_ == -1) {
        // 返回最后操作的错误码。
        DWORD lError = NET_DVR_GetLastError();
        LONG lErrorNo = LONG(lError);
        // 返回最后操作的错误码信息。
        spdlog::error("NET_DVR_RealPlay_V40: {} {}", lError, NET_DVR_GetErrorMsg(&lErrorNo));
        return lError;
    }
    spdlog::info("NET_DVR_RealPlay_V40 OK");

    // 注册回调函数，捕获实时码流数据。
    BOOL ret = NET_DVR_SetRealDataCallBackEx(this->lRealHandle_, fRealDataCallBack_V30, this);
    if (!ret) {
        // 返回最后操作的错误码。
        DWORD lError = NET_DVR_GetLastError();
        LONG lErrorNo = LONG(lError);
        // 返回最后操作的错误码信息。
        spdlog::error("NET_DVR_SetRealDataCallBackEx: {} {}", lError, NET_DVR_GetErrorMsg(&lErrorNo));
        return lError;
    }
    spdlog::info("NET_DVR_SetRealDataCallBackEx OK");
    return kOK;
}