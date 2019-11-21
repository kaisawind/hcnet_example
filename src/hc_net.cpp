#include "hc_net.h"
#include "common.h"
#include <HCNetSDK.h>

HCNet::HCNet() {
	spdlog::info("HC Net DVR Init");
	// 初始化SDK，调用其他SDK函数的前提。
	BOOL ret = NET_DVR_Init();
	if (!ret) {
		// 返回最后操作的错误码。
		DWORD lError = NET_DVR_GetLastError();
		LONG lErrorNo = LONG(lError);
		// 返回最后操作的错误码信息。
		spdlog::error("NET_DVR_Init: {} {}", lError, NET_DVR_GetErrorMsg(&lErrorNo));
		return;
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
		spdlog::error("NET_DVR_Init: {} {}", lError, NET_DVR_GetErrorMsg(&lErrorNo));
		return;
	}
	spdlog::info("当前login用户数:       {}", sdkState.dwTotalLoginNum);
	spdlog::info("当前realplay路数:      {}", sdkState.dwTotalRealPlayNum);
	spdlog::info("当前回放或下载路数:    {}", sdkState.dwTotalPlayBackNum);
	spdlog::info("当前建立报警通道路数:  {}", sdkState.dwTotalAlarmChanNum);
	spdlog::info("当前硬盘格式化路数:	 {}", sdkState.dwTotalFormatNum);
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