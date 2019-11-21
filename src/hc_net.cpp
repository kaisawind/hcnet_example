#include "hc_net.h"
#include "common.h"
#include <HCNetSDK.h>

HCNet::HCNet() {
	spdlog::info("HC Net DVR Init");
	// ��ʼ��SDK����������SDK������ǰ�ᡣ
	BOOL ret = NET_DVR_Init();
	if (!ret) {
		// �����������Ĵ����롣
		DWORD lError = NET_DVR_GetLastError();
		LONG lErrorNo = LONG(lError);
		// �����������Ĵ�������Ϣ��
		spdlog::error("NET_DVR_Init: {} {}", lError, NET_DVR_GetErrorMsg(&lErrorNo));
		return;
	}

	// ��ȡSDK�İ汾��Ϣ��
	DWORD dVer = NET_DVR_GetSDKVersion();
	DWORD dLowVer = dVer & 0x0000FFFF;
	DWORD dHighVer = dVer & 0xFFFF0000 >> 16;
	spdlog::info("NET DVR SDK Version {}.{}", dHighVer, dLowVer);

	NET_DVR_SDKSTATE sdkState = { 0 };
	ret = NET_DVR_GetSDKState(&sdkState);
	if (!ret) {
		// �����������Ĵ����롣
		DWORD lError = NET_DVR_GetLastError();
		LONG lErrorNo = LONG(lError);
		// �����������Ĵ�������Ϣ��
		spdlog::error("NET_DVR_Init: {} {}", lError, NET_DVR_GetErrorMsg(&lErrorNo));
		return;
	}
	spdlog::info("��ǰlogin�û���:       {}", sdkState.dwTotalLoginNum);
	spdlog::info("��ǰrealplay·��:      {}", sdkState.dwTotalRealPlayNum);
	spdlog::info("��ǰ�طŻ�����·��:    {}", sdkState.dwTotalPlayBackNum);
	spdlog::info("��ǰ��������ͨ��·��:  {}", sdkState.dwTotalAlarmChanNum);
	spdlog::info("��ǰӲ�̸�ʽ��·��:	 {}", sdkState.dwTotalFormatNum);
	spdlog::info("��ǰ�ļ�����·��:      {}", sdkState.dwTotalFileSearchNum);
	spdlog::info("��ǰ��־����·��:      {}", sdkState.dwTotalLogSearchNum);
	spdlog::info("��ǰ͸��ͨ��·��:      {}", sdkState.dwTotalSerialNum);
	spdlog::info("��ǰ����·��:          {}", sdkState.dwTotalUpgradeNum);
	spdlog::info("��ǰ����ת��·��:      {}", sdkState.dwTotalVoiceComNum);
	spdlog::info("��ǰ�����㲥·��:      {}", sdkState.dwTotalBroadCastNum);
	spdlog::info("��ǰ�������·��:      {}", sdkState.dwTotalListenNum);
	spdlog::info("��ǰ�ʼ�����·��:      {}", sdkState.dwEmailTestNum);
	spdlog::info("��ǰ�ļ�����·��:      {}", sdkState.dwBackupNum);
	spdlog::info("��ǰ��Ѷ�ϴ�·��:      {}", sdkState.dwTotalInquestUploadNum);
}

HCNet::~HCNet() {
	spdlog::info("HC Net DVR Cleanup");
	// �ͷ�SDK��Դ���ڳ������֮ǰ���á�
	BOOL ret = NET_DVR_Cleanup();
	if (!ret) {
		// �����������Ĵ����롣
		DWORD lError = NET_DVR_GetLastError();
		LONG lErrorNo = LONG(lError);
		// �����������Ĵ�������Ϣ��
		spdlog::error("NET_DVR_Init: {} {}", lError, NET_DVR_GetErrorMsg(&lErrorNo));
		return;
	}
}