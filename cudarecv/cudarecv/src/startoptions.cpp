#include <iostream>
#include <getopt.h>
#include <cstring>
#include <stdint.h>
#include <cstdlib>
#include "startoptions.h"
#include "auxil.h"

using namespace std;

#define OPT_STR    "f:"

// 打印程序的使用方式。
void StartOptions::Usage(char* program) {
    cout << "Usage: " << program << "[-i] [-f filename]" << endl; // 打印出程序的使用方法，其中 program 是程序的名称，-i 和 -f filename 是示例选项。
    cout << "       " << "--no-console" << endl; // 显示 --no-console 选项，该选项可能用于指示程序不以控制台模式运行。
    cout << "       " << "--filename filename" << endl; // 显示 --filename 选项，后面需要跟随一个参数（filename），用于指定输入文件的名称。
    cout << "       " << "--samplerate 2000000" << endl; // 显示 --samplerate 选项，后面需要跟随一个参数，用于设置采样率，示例中为 2000000 Hz。
    cout << "       " << "--carrierfrequency 0" << endl; // 显示 --carrierfrequency 选项，后面需要跟随一个参数，用于设置载波频率，示例中为 0 Hz。
    cout << "       " << "--skip 0" << endl; // 显示 --skip 选项，后面需要跟随一个参数，用于指定跳过的样本数，示例中为 0。
    cout << "       " << "--fileformat 0" << endl; // 显示 --fileformat 选项，后面需要跟随一个参数，用于设置文件格式，示例中为 0。
}

// StartOptions 类的 ParseOptions 成员函数的实现，它用于解析命令行参数
void StartOptions::ParseOptions(int argc, char* argv[]) {
    int c; // 用于存储 getopt_long 返回的值

    // 无限循环，用于持续解析命令行选项，直到没有更多选项
    while (1) {
        // 定义一个静态数组，存储长选项的信息
        static struct option longOptions[] = {
            // 每个选项包括名称、是否有参数、标志和对应的值
            {"no-console", no_argument, 0, '5'}, // 不带参数的选项
            {"filename", required_argument, 0, 'f'}, // 需要参数的选项
            {"samplerate", required_argument, 0, '1'}, // 需要参数的选项
            {"carrierfrequency", required_argument, 0, '2'}, // 需要参数的选项
            {"skip", required_argument, 0, '3'}, // 需要参数的选项
            {"fileformat", required_argument, 0, '4'}, // 需要参数的选项
            {0, 0, 0, 0} // 选项列表结束标志
        };

        int optionIndex = 0; // 用于跟踪当前处理的长选项索引

        // 解析长选项和短选项，optarg 存储当前选项的参数值
        c = getopt_long(argc, argv, OPT_STR, longOptions, &optionIndex);
        // 如果没有更多选项或遇到问题，则 getopt_long 返回 -1
        if (c == -1) break;

        // 根据当前选项的值，执行相应的操作
        switch (c) {
        case '5': // 如果是 '--no-console' 选项
            console = false; // 设置 console 为 false
            break;
        case 'f': // 如果是 '--filename' 选项
            fromFile = true; // 设置 fromFile 为 true
            filename = optarg; // 存储参数值到 filename
            //cout << "from file: " << filename << endl; // 调试信息，已注释
            break;
        case '1': // 如果是 '--samplerate' 选项
            // 解析采样率参数，并存储在 sampleRate 中
            sampleRate = ParseFreq(optarg);
            //cout << "sample rate: " << sampleRate << endl; // 调试信息，已注释
            break;
        case '2': // 如果是 '--carrierfrequency' 选项
            // 解析载波频率参数，并存储在 carrierFreq 中
            carrierFreq = ParseFreq(optarg);
            //cout << "carrier frequency: " << carrierFreq << endl; // 调试信息，已注释
            break;
        case '3': // 如果是 '--skip' 选项
            // 此处尚未实现解析逻辑，仅打印参数值
            //cout << "skip: " << optarg << endl; // 调试信息，已注释
            break;
        case '4': // 如果是 '--fileformat' 选项
            // 此处尚未实现解析逻辑，仅打印参数值
            //cout << "file format: " << optarg << endl; // 调试信息，已注释
            break;
        case '?': // 如果 getopt_long 遇到未知选项或错误
            // getopt_long 已经打印了错误消息
            error = true; // 设置错误标志为 true
            Usage(argv[0]); // 打印使用说明
            return; // 返回函数
            break;
        default: // 如果遇到无法识别的选项
            // 打印错误信息，并退出函数
            cerr << "Error: option " << c
                << " not recognized. Argument: " << optarg << endl;
            error = true; // 设置错误标志为 true
            Usage(argv[0]); // 打印使用说明
            return;
        }
    }

    // 此处的调试信息被注释掉了，如果取消注释，将打印 console 的值
    //cout << "console: " << console << endl;
}

// StartOptions 构造函数，初始化默认值并解析命令行参数
StartOptions::StartOptions(int argc, char* argv[]) {
    InitDefaults(); // 初始化默认值
    ParseOptions(argc, argv); // 解析命令行参数
}

// 初始化默认值
void StartOptions::InitDefaults() {
    error = false; // 错误标志
    console = true; // 控制台模式
    fromFile = false; // 是否从文件读取
    sampleRate = 2000000; // 采样率，单位 Hz
    carrierFreq = 0; // 载波频率，单位 Hz
    skip = 0; // 跳过的样本数
    fileFormat = 0; // 文件格式
    filename.clear(); // 清空文件名
}

// 解析频率字符串为整数，返回 Hz 值
int StartOptions::ParseFreq(char* arg) {
    double val = 0.0; // 频率值
    string units; // 单位
    SplitArgument(arg, val, units); // 分割参数为值和单位
    if (units.length() > 0){
        if (((auxil::strcmpi(units, string("MHz")) == 0) && (units.at(0) == 'M')) ||
                (units.compare("M") == 0))
            val *= 1000000.0; // MHz 转换为 Hz
        else if ((auxil::strcmpi(units, string("GHz")) == 0) ||
                (auxil::strcmpi(units, string("G")) == 0))
            val *= 1000000000.0; // GHz 转换为 Hz
        else if ((auxil::strcmpi(units, string("kHz")) == 0) ||
                (auxil::strcmpi(units, string("k")) == 0))
            val *= 1000.0; // kHz 转换为 Hz
        else
            return -1; // 无效单位，返回错误
    }
    val += 0.5; // 四舍五入
    return (int)val; // 返回整数值
}

// 分割输入字符串为值和单位
int StartOptions::SplitArgument(char* arg, double &val, string &units) {
    string str = arg; // 转换为字符串
    auxil::trimWhiteSpace(str); // 去除空白字符
    size_t split = str.find_first_not_of("1234567890."); // 查找第一个非数字字符的位置
    string num; // 数字部分
    if (split >= str.length()){
        units.clear(); // 没有单位
        num = str; // 全部为数字
    } else {
        num = str.substr(0,split); // 提取数字部分
        str = str.substr(split, string::npos); // 提取单位部分
        auxil::trimWhiteSpace(str); // 去除空白字符
        units = str; // 设置单位
    }
    val = atof(num.c_str()); // 转换为浮点数
    return 0; // 返回成功
}
