
#ifndef INC__STARTOPTIONS_H_
#define INC__STARTOPTIONS_H_

#include <string>

//using namespace std;

class StartOptions {

    public:

        StartOptions(int argc, char* argv[]);

        bool console;                   /**< console mode *///指示程序是否以控制台模式运行
        bool fromFile;                  /**< samples from file *///指示样本数据是否来自文件。
        unsigned int sampleRate;        /**< in Hz *///采样率，单位是赫兹（Hz）。
        unsigned int carrierFreq;       /**< in Hz *///载波频率，单位是赫兹（Hz）。
        unsigned long skip;             /**< in number of samples *///跳过的样本数。
        unsigned char fileFormat;//文件格式。
        std::string filename;//文件名。

        bool error;//指示是否发生错误。

    private:

        /** \brief Initializes all options to defaults. *///初始化所有选项为默认值。
        void InitDefaults(void);

        /** \brief Parses all of argv[] into options. *///解析命令行参数，填充成员变量。
        void ParseOptions(int argc, char* argv[]);

        /** \brief Prints usage instructions. *///打印使用说明，通常在命令行参数错误时调用。
        void Usage(char* program);

        /** \brief Parses frequency string into integer.
         *
         *  Parses possible strings such as 12.3GHz into integer of value Hz.
         *  Possible units are GHz, MHz, kHz.
         *  return frequency in Hz. -1 on error.
         *///解析频率字符串（如 "12.3GHz"）为整数赫兹值。在出错时返回 -1。
        int ParseFreq(char* arg);

        /** \brief Splits an input string into its value and units. *///将输入字符串分割为数值和单位，返回解析状态。
        int SplitArgument(char* arg, double &val, std::string &units);

};

#endif
