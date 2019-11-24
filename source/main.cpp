// hcnet2webrtc.cpp: 定义应用程序的入口点。
//

#include "common.h"
#include "hc_net.h"

using namespace std;

int main() {
    cout << "Hello CMake." << endl;
    spdlog::set_level(spdlog::level::debug);
    spdlog::info("Welcome to spdlog!");

    auto hcNet = new HCNet();
    hcNet->Login("123.185.223.20", WORD(8000), "admin", "1qaz2wsx");
    hcNet->RealPlay(33);
    system("pause");
    delete hcNet;
    return 0;
}
