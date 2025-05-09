#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include "startoptions.h"
#include "console.h"
#include "cmdParser.h"
#include "flowmgr.h"
#include "dsp.h"
//#include "acquisition.h"
#include "dpeflow.h"

// For math functions used by initialization shift
// 用于初始化偏移的数学函数
#include <math.h>
#include <stdlib.h>

using namespace std;

// 全局指针，指向命令解析器和流管理器对象
console::CmdParser* cmdMgr = NULL;
static dsp::FlowMgr* fm = nullptr;

// 全局变量，用于控制程序的运行状态
volatile char KeepRunning = 1;

// 信号处理函数，用于处理 SIGINT 信号（通常是 Ctrl+C）
void sigHandler(int sig) {
    signal(sig, SIG_IGN);
    /*if (fm && fm->EmergencyStop()){
        std::cout << "[SIGINT] Press Ctrl+C again to quit the program." << std::endl;
    } else {*/
    cout << "[SIGINT] Signal Caught... exiting" << endl;
    KeepRunning = 0;
    //}
}

/** \brief  Main entry to program.
 *  程序的主要入口点
 */
int main(int argc, char** argv) {

    // 打印程序版本信息
    std::cout << "GNSS Receiver by Gao Research Group" << endl;
    std::cout << "Version 0.0 Alpha" << endl;

    // 创建 StartOptions 对象来解析命令行参数
    StartOptions options(argc, argv);

    // 如果解析出错，则返回 -1 退出程序
    if (options.error) return -1;

    // 设置 POSIX 信号处理，以便程序可以处理 Ctrl+C 信号
    KeepRunning = 1;

    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = sigHandler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);

    /* Set main thread as the lowest priority in
     * real time round robin policy.
     * 设置主线程的调度优先级为实时轮转策略中的最低优先级
     */
    pthread_t mainThread = pthread_self();
    struct sched_param param;
    param.sched_priority = sched_get_priority_min(SCHED_RR);
    pthread_setschedparam(mainThread, SCHED_RR, &param);

    // Primary way of running CUDARecv -- ask the user what to do
    // Follow the README for instructions on how to interact with the program
    // 主要的运行方式 -- 询问用户要做什么
    // 按照 README 中的说明与程序交互
    if (options.console) {
        // 创建命令解析器和流管理器对象
        cmdMgr = new console::CmdParser("\033[01;32mgnss\033[00m$ ");
        fm = new dsp::FlowMgr;
        console::initCommonCmd(cmdMgr);
        console::initFlowCmd(cmdMgr, fm);

        // 进入命令执行循环，直到 KeepRunning 变为 0
        while ((cmdMgr->execOneCmd()) && KeepRunning);

        // 删除命令解析器和流管理器对象
        delete fm;
        delete cmdMgr;
    }
    else
        return 0;

    // Automated method 2: load and start -- don't bother interacting with the user, just launch DPEFlow
    // 自动方法 2：加载并启动 -- 不与用户交互，只启动 DPEFlow
    /*
    dsp::DPEFlow myFlow;
    int i = myFlow.LoadFlow(NULL);
    if (i == 0) myFlow.Start();
    //while ((cmdMgr->execOneCmd()) && KeepRunning);
    while (!myFlow.CheckFlowState()) {
        sleep(1);
    }
    myFlow.Stop();
    cout << "[MAIN] Stopped flow." << endl;
    */

    // Automated method 2: random initial receiver state guess -- perturb the initial state and run repeatedly, indexing the output files
    // 自动方法 2：随机初始接收器状态猜测 -- 扰动初始状态并重复运行，索引输出文件
    /*
    // Rotate the offset into ECEF frame (note: this only works for the usrp 6 simulated datasets!)
    // 将偏移旋转到 ECEF 框架（注意：这仅适用于 usrp 6 模拟数据集！）
    double r_enu2ecef[9] = {0.9995, -0.0199, 0.0237, 0.0309, 0.6440, -0.7644, 0, 0.7648, 0.6443}; // Receiver in Urbana-Champaign, IL, USA

    // Name the output file from the current date and time
    // 从当前日期和时间命名输出文件
    time_t rawtime;
    struct tm * timeinfo;
    char buffer[80];

    // Set 1
    // 设置 1
    time (&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer,sizeof(buffer),"%d-%m-%Y_%H-%M-%S",timeinfo);
    std::string fnameTime(buffer);
    // Helpers for the name of the output file
    // 输出文件名的辅助工具
    std::string XFileNamePrefix = "/home/ubuntu/output/test/";
    std::string XFileNameSuffix = "-XFile.csv";
    std::string XFileName;

    cmdMgr = new console::CmdParser("\033[01;32mgnss\033[00m$ ");
    console::initCommonCmd(cmdMgr);
    console::initFlowCmd(cmdMgr,fm);

    srand(time(0));
    ofstream shiftFile;
    std::string shiftFileName;
    shiftFileName = XFileNamePrefix + "shiftFile_" + fnameTime + ".csv";
    shiftFile.open(shiftFileName);

    for (int idx = 0; idx < 100; idx++) {

        dsp::DPEFlow myFlow;
        int i = myFlow.LoadFlow(NULL);

        // Specify how much spread to use
        // 指定使用多少扩展
        //float shiftMin = 50.0;
        //float shiftMax = 80.0;

        float shiftRange = 30.0;
        //float shiftRange = 25.0;
        float shiftBottom = 50.0;
        //float shiftBottom = 25.0;

        // (New) Horizontal shift
        // （新）水平偏移
        float magShift = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX/(2*shiftRange)))-shiftRange;
        if (magShift >= 0) {
            magShift += shiftBottom;
        }
        else {
            magShift -= shiftBottom;
        }
        float thetaShift = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX/(3.1415926535*2)));
        float yShiftENU = magShift * sin(thetaShift);
        float xShiftENU = magShift * cos(thetaShift);

        // Vertical shift
        // 垂直偏移
        float zShiftENU = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX/(2*shiftRange)))-shiftRange;

        if (zShiftENU >= 0) {
            zShiftENU += shiftBottom;
        }
        else {
            zShiftENU -= shiftBottom;
        }

        // Time shift
        // 时间偏移
        float tShift = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX/(2*shiftRange)))-shiftRange;

        if (tShift >= 0) {
            tShift += shiftBottom;
        }
        else {
            tShift -= shiftBottom;
        }

        float xShift = r_enu2ecef[0] * xShiftENU + r_enu2ecef[1] * yShiftENU + r_enu2ecef[2] * zShiftENU;
        float yShift = r_enu2ecef[3] * xShiftENU + r_enu2ecef[4] * yShiftENU + r_enu2ecef[5] * zShiftENU;
        float zShift = r_enu2ecef[6] * xShiftENU + r_enu2ecef[7] * yShiftENU + r_enu2ecef[8] * zShiftENU;

        //xShift = 0;
        //yShift = 0;
        //zShift = 0;
        tShift = 0;

        myFlow.SetModParam("DPInit", "InitDeltaX", xShift);
        myFlow.SetModParam("DPInit", "InitDeltaY", yShift);
        myFlow.SetModParam("DPInit", "InitDeltaZ", zShift);
        myFlow.SetModParam("DPInit", "InitDeltaT", tShift);
        myFlow.SetModParam("BatchCorrManifold", "LoadPosGrid", (bool)false);

        shiftFile << idx << ", " << xShift << ", " << yShift << ", " << zShift << ", " << tShift << std::endl;

        XFileName = XFileNamePrefix + fnameTime + "_idx" + std::to_string(idx) + XFileNameSuffix;

        myFlow.SetModParam("XECEFLogger", "Filename", XFileName.c_str());

        if (i == 0) myFlow.Start();
        //while ((cmdMgr->execOneCmd()) && KeepRunning);
        while (!myFlow.CheckFlowState()) {
            sleep(1);
        }
        myFlow.Stop();
        cout << "[MAIN] Stopped flow." << endl;
    }

    shiftFile.close();
    */

    // Automated method 3: resizable grids -- change grid spacing and run repeatedly, indexing the output files
    // 自动方法 3：可调整大小的网格 -- 改变网格间距并重复运行，索引输出文件
    /*
    // Name the output file from the current date and time
    // 从当前日期和时间命名输出文件
    time_t rawtime;
    struct tm * timeinfo;
    char buffer[80];

    time (&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer,sizeof(buffer),"%d-%m-%Y_%H-%M-%S",timeinfo);
    std::string fnameTime(buffer);
    // Helpers for the name of the output file
    // 输出文件名的辅助工具
    std::string XFileNamePrefix = "/home/ubuntu/output/bulk/";
    std::string XFileNameSuffix = "-XFile.csv";
    std::string XFileName;

    cmdMgr = new console::CmdParser("\033[01;32mgnss\033[00m$ ");
    console::initCommonCmd(cmdMgr);
    console::initFlowCmd(cmdMgr,fm);

    srand(static_cast<unsigned> (time(0)));
    ofstream shiftFile;
    std::string shiftFileName;
    shiftFileName = XFileNamePrefix + "shiftFile_" + fnameTime + ".csv";
    shiftFile.open(shiftFileName);

    for (int idx = 0; idx < 7; idx++) {

        dsp::DPEFlow myFlow;
        int i = myFlow.LoadFlow(NULL);

        XFileName = XFileNamePrefix + fnameTime + "_idx" + std::to_string(idx) + XFileNameSuffix;

        myFlow.SetModParam("XECEFLogger", "Filename", XFileName.c_str());

        myFlow.SetModParam("BatchCorrManifold", "GridDimSpacing", (float)((idx+1)*0.5 + 6.5));
        //myFlow.SetModParam("BatchCorrManifold", "GridDimSpacing", (float)((idx+1)+7));

        if (i == 0) myFlow.Start();
        //while ((cmdMgr->execOneCmd()) && KeepRunning);
        while (!myFlow.CheckFlowState()) {
            sleep(1);
        }
        myFlow.Stop();
        cout << "[MAIN] Stopped flow." << endl;
    }

    shiftFile.close();
    */

    return 0;
}
