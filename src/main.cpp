// hcnet2webrtc.cpp: 定义应用程序的入口点。
//

#include <HCNetSDK.h>
#include "common.h"
#include "hc_net.h"

using namespace std;

int main()
{
	cout << "Hello CMake." << endl;
	spdlog::info("Welcome to spdlog!");
	HCNet* hcNet = new HCNet();
	hcNet->Login();
    delete hcNet;
	return 0;
}
