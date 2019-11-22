#include "hc_net.h"
#include "common.h"
#include <functional>

static void CbLoginResult(LONG lUserID, DWORD dwResult, LPNET_DVR_DEVICEINFO_V30 lpDeviceInfo, void* pUser) {
	HCNet* self = static_cast<HCNet*>(pUser);
	self->LoginResult(lUserID, dwResult, lpDeviceInfo);
}

HCNet::HCNet() {
	this->lUserID_ = -1;
	this->lpDeviceInfo_ = NULL;

	// NVR init
	this->Init();
}

HCNet::~HCNet() {
	spdlog::info("HC Net DVR Cleanup");
	// 释放SDK资源，在程序结束之前调用。
	BOOL ret = NET_DVR_Cleanup();
	if (!ret) {
		// 返回最后操作的错误码。
		DWORD lError = NET_DVR_GetLastError();
		LONG lErrorNo = LONG(lError);
		// 返回最后操作的错误码信息。
		spdlog::error("NET_DVR_Init: {} {}", lError, NET_DVR_GetErrorMsg(&lErrorNo));
		return;
	}
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
	DWORD dHighVer = dVer & 0xFFFF0000 >> 16;
	spdlog::info("NET DVR SDK Version {}.{}", dHighVer, dLowVer);

	NET_DVR_SDKSTATE sdkState = { 0 };
	ret = NET_DVR_GetSDKState(&sdkState);
	if (!ret) {
		// 返回最后操作的错误码。
		DWORD lError = NET_DVR_GetLastError();
		LONG lErrorNo = LONG(lError);
		// 返回最后操作的错误码信息。
		spdlog::error("NET_DVR_GetSDKState: {} {}", lError, NET_DVR_GetErrorMsg(&lErrorNo));
		return lError;
	}
	spdlog::info("当前login用户数:       {}", sdkState.dwTotalLoginNum);
	spdlog::info("当前realplay路数:      {}", sdkState.dwTotalRealPlayNum);
	spdlog::info("当前回放或下载路数:    {}", sdkState.dwTotalPlayBackNum);
	spdlog::info("当前建立报警通道路数:  {}", sdkState.dwTotalAlarmChanNum);
	spdlog::info("当前硬盘格式化路数:    {}", sdkState.dwTotalFormatNum);
	spdlog::info("当前文件搜索路数:      {}", sdkState.dwTotalFileSearchNum);
	spdlog::info("当前日志搜索路数:      {}", sdkState.dwTotalLogSearchNum);
	spdlog::info("当前透明通道路数:      {}", sdkState.dwTotalSerialNum);
	spdlog::info("当前升级路数:          {}", sdkState.dwTotalUpgradeNum);
	spdlog::info("当前语音转发路数:      {}", sdkState.dwTotalVoiceComNum);
	spdlog::info("当前语音广播路数:      {}", sdkState.dwTotalBroadCastNum);
	spdlog::info("当前网络监听路数:      {}", sdkState.dwTotalListenNum);
	spdlog::info("当前邮件计数路数:      {}", sdkState.dwEmailTestNum);
	spdlog::info("当前文件备份路数:      {}", sdkState.dwBackupNum);
	spdlog::info("当前审讯上传路数:      {}", sdkState.dwTotalInquestUploadNum);

	NET_DVR_SDKABL adkAbl = { 0 };
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
	return NET_DVR_NOERROR;
}

void HCNet::Login() {

	NET_DVR_USER_LOGIN_INFO loginInfo = {
		"192.168.1.91",			// sDeviceAddress 设备地址，IP 或者普通域名 
		0,						// byUseTransport 是否启用能力集透传：0- 不启用透传，默认；1- 启用透传
		8000,					// wPort		  设备端口号，例如：8000 
		"admin",				// sUserName	  登录用户名，例如：admin 
		"1qaz2wsx",				// sPassword	  登录密码，例如：12345 
		CbLoginResult,			// cbLoginResult  登录状态回调函数，bUseAsynLogin 为1时有效
		this,					// pUser		  用户数据
		TRUE,					// bUseAsynLogin  是否异步登录：0- 否，1- 是
		0,						// byProxyType    代理服务器类型：0- 不使用代理，1- 使用标准代理，2- 使用EHome代理
		0,						// byUseUTCTime   是否使用UTC时间：0- 不进行转换，默认；1- 输入输出UTC时间，SDK进行与设备时区的转换；2- 输入输出平台本地时间，SDK进行与设备时区的转换
		0,						// byLoginMode    登录模式(不同模式具体含义详见“Remarks”说明)：0- SDK私有协议，1- ISAPI协议，2- 自适应（设备支持协议类型未知时使用，一般不建议）
		0,						// byHttps        ISAPI协议登录时是否启用HTTPS(byLoginMode为1时有效)：0- 不启用，1- 启用，2- 自适应（设备支持协议类型未知时使用，一般不建议）
		0,						// iProxyID       代理服务器序号，添加代理服务器信息时相对应的服务器数组下表值
		0,						// byVerifyMode
	};
	NET_DVR_DEVICEINFO_V40 deviceInfo = { 0 };
	NET_DVR_Login_V40(&loginInfo, &deviceInfo);
}


void HCNet::LoginResult(LONG lUserID, DWORD dwResult, LPNET_DVR_DEVICEINFO_V30 lpDeviceInfo) {
	// 异步登录失败
	if (dwResult == 0) {
		// 返回最后操作的错误码。
		DWORD lError = NET_DVR_GetLastError();
		LONG lErrorNo = LONG(lError);
		// 返回最后操作的错误码信息。
		spdlog::error("NET_DVR_Init: {} {}", lError, NET_DVR_GetErrorMsg(&lErrorNo));
		return;
	}
	this->lpDeviceInfo_ = lpDeviceInfo;
	this->lUserID_ = lUserID;
	spdlog::info("序列号 {}", lpDeviceInfo->sSerialNumber);
	spdlog::info("报警输入个数 {}", lpDeviceInfo->byAlarmInPortNum);
	spdlog::info("报警输出个数 {}", lpDeviceInfo->byAlarmOutPortNum);
	spdlog::info("硬盘个数 {}", lpDeviceInfo->byDiskNum);
	switch (lpDeviceInfo -> byDVRType) {
	case 1:
		spdlog::info("设备类型 {}:{}", lpDeviceInfo->byDVRType, "DVR");
		break;
	case 2:
		spdlog::info("设备类型 {}:{}", lpDeviceInfo->byDVRType, "ATM DVR");
		break;
	case 3:
		spdlog::info("设备类型 {}:{}", lpDeviceInfo->byDVRType, "DVS");
		break;
	default:
		spdlog::info("设备类型 {}:{}", lpDeviceInfo->byDVRType, "unknown");
		break;
	}
	spdlog::info("模拟通道个数 {}", lpDeviceInfo->byChanNum);
	spdlog::info("起始通道号 {}", lpDeviceInfo->byStartChan);
	spdlog::info("语音通道数 {}", lpDeviceInfo->byAudioChanNum);
	spdlog::info("最大数字通道个数,低位 {}", lpDeviceInfo->byIPChanNum);
	spdlog::info("零通道编码个数 {}", lpDeviceInfo->byZeroChanNum);

	// 主码流传输协议类型
	switch (lpDeviceInfo->byMainProto) {
	case 0:
		spdlog::info("主码流传输协议类型 {}-{}", lpDeviceInfo->byMainProto, "private");
		break;
	case 1:
		spdlog::info("主码流传输协议类型 {}-{}", lpDeviceInfo->byMainProto, "rtsp");
		break;
	case 2:
		spdlog::info("主码流传输协议类型 {}-{}", lpDeviceInfo->byMainProto, "同时支持private和rtsp");
		break;
	default:
		spdlog::info("主码流传输协议类型 {}-{}", lpDeviceInfo->byMainProto, "unknown");
		break;
	}
	// 子码流传输协议类型
	switch (lpDeviceInfo->bySubProto) {
	case 0:
		spdlog::info("子码流传输协议类型 {}-{}", lpDeviceInfo->byMainProto, "private");
		break;
	case 1:
		spdlog::info("子码流传输协议类型 {}-{}", lpDeviceInfo->byMainProto, "rtsp");
		break;
	case 2:
		spdlog::info("子码流传输协议类型 {}-{}", lpDeviceInfo->byMainProto, "同时支持private和rtsp");
		break;
	default:
		spdlog::info("子码流传输协议类型 {}-{}", lpDeviceInfo->byMainProto, "unknown");
		break;
	}
	spdlog::info("是否支持智能搜索 {}", lpDeviceInfo->bySupport & 0x1);
	spdlog::info("是否支持备份 {}", lpDeviceInfo->bySupport & 0x2);
	spdlog::info("是否支持压缩参数能力获取 {}", lpDeviceInfo->bySupport & 0x3);
	spdlog::info("是否支持多网卡 {}", lpDeviceInfo->bySupport & 0x4);
	spdlog::info("是否支持远程SADP {}", lpDeviceInfo->bySupport & 0x10);
	spdlog::info("是否支持Raid卡功能 {}", lpDeviceInfo->bySupport & 0x20);
	spdlog::info("是否支持IPSAN 目录查找 {}", lpDeviceInfo->bySupport & 0x40);
	spdlog::info("是否支持rtp over rtsp {}", lpDeviceInfo->bySupport & 0x80);
}