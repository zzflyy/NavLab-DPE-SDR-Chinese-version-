#include <cstdlib>
#include <cctype>

#include "auxil.h"
#include "cmdFlow.h"

#include "console.h"
#include "cmdExec.h"
#include "cmdParser.h"

// new flow, start flow, stop flow, destroy flow, list flow.
using namespace console;

// 初始化流管理命令
bool console::initFlowCmd(CmdParser* const cm, dsp::FlowMgr* const fm) {
    // 第二个参数是命令中的大写字母数量
    // 这用于决定哪些字符转换为大写，哪些转换为小写
    // 因此，任何命令都应以大写字母开头，以小写字母结尾
    // (永远不要是大写、小写、大写的顺序)
    if (cm->regCmd("NEWFlow", 4, new NewFlow(cm, fm)) &&
        cm->regCmd("DELFlow", 4, new DelFlow(cm, fm)) &&
        cm->regCmd("STARTFlow", 6, new StartFlow(cm, fm)) &&
        cm->regCmd("STOPFlow", 5, new StopFlow(cm, fm)) &&
        cm->regCmd("LOADFlow", 5, new LoadFlow(cm, fm)) &&
        cm->regCmd("ADDAlias", 4, new AddAlias(cm, fm)) &&
        cm->regCmd("ACTAlias", 4, new ActiveAlias(cm, fm)) &&
        cm->regCmd("ACTFlow", 4, new ActiveFlow(cm, fm)) &&
        cm->regCmd("SETParam", 4, new SetParam(cm, fm)) &&
        cm->regCmd("LSFlow", 3, new FlowType(cm, fm)) &&
        cm->regCmd("PRINTport", 5, new PrintPort(cm, fm))
        ) return true;
    std::cerr << "Flow command registration failed." << std::endl;
    return false;
}

// NewFlow 命令的执行函数
CmdExecStatus NewFlow::exec(const std::string& option) {
    std::vector<std::string> tokens;
    if (!CmdExec::lexOptions(option, tokens)) return CMD_EXEC_ERROR;

    int val = -1;
    switch (tokens.size()) {
    case 0:
        return errorOption(CMD_OPT_MISSING, "");
    case 1:
        val = _fmgr->createFlow(tokens[0]);
        break;
    case 2:
        val = _fmgr->createFlow(tokens[0], tokens[1]);
        break;
    default:
        return errorOption(CMD_OPT_EXTRA, tokens[2]);
    }
    return (val == 0) ? CMD_EXEC_DONE : CMD_EXEC_ERROR;
}

// DelFlow 命令的执行函数
CmdExecStatus DelFlow::exec(const std::string& option) {
    std::vector<std::string> tokens;
    if (!CmdExec::lexOptions(option, tokens)) return CMD_EXEC_ERROR;

    int state = 0;
    for (auto const& toki : tokens)
        state |= _fmgr->destroyFlow(toki);
    return (state == 0) ? CMD_EXEC_DONE : CMD_EXEC_ERROR;
}

// StartFlow 命令的执行函数
CmdExecStatus StartFlow::exec(const std::string& option) {
    std::vector<std::string> tokens;
    if (!CmdExec::lexOptions(option, tokens)) return CMD_EXEC_ERROR;

    int state = 0;
    for (auto const& toki : tokens)
        state |= _fmgr->startFlow(toki);
    return (state == 0) ? CMD_EXEC_DONE : CMD_EXEC_ERROR;
}

// StopFlow 命令的执行函数
CmdExecStatus StopFlow::exec(const std::string& option) {
    std::vector<std::string> tokens;
    if (!CmdExec::lexOptions(option, tokens)) return CMD_EXEC_ERROR;

    int state = 0;
    for (auto const& toki : tokens)
        state |= _fmgr->stopFlow(toki);
    return (state == 0) ? CMD_EXEC_DONE : CMD_EXEC_ERROR;
}

// LoadFlow 命令的执行函数
CmdExecStatus LoadFlow::exec(const std::string& option) {
    std::vector<std::string> tokens;
    if (!CmdExec::lexOptions(option, tokens)) return CMD_EXEC_ERROR;
    int val = -1;
    switch (tokens.size()) {
    case 0:
        return errorOption(CMD_OPT_MISSING, "");
    case 1:
        val = _fmgr->loadFlow(tokens[0]);
        break;
    case 2:
        val = _fmgr->loadFlow(tokens[0], tokens[1].c_str());
        break;
    default:
        return errorOption(CMD_OPT_EXTRA, tokens[2]);
    }
    return (val == 0) ? CMD_EXEC_DONE : CMD_EXEC_ERROR;
}

// AddAlias 命令的执行函数
CmdExecStatus AddAlias::exec(const std::string& option) {
    std::vector<std::string> tokens;
    if (!CmdExec::lexOptions(option, tokens)) return CMD_EXEC_ERROR;
    if (tokens.size() < 2) return errorOption(CMD_OPT_MISSING, "ident & alias");

    const size_t idx = _fmgr->getFlowIdx(tokens[0]);
    if (idx == NPOS) {
        std::cerr << "[FlowMgr] Flow identifier " << tokens[0] << " invalid." << std::endl;
        return errorOption(CMD_OPT_ILLEGAL, tokens[0]);
    }

    for (size_t i = 1; i < tokens.size(); i++)
        _fmgr->addAlias(tokens[i], idx);
    return CMD_EXEC_DONE;
}

// ActiveAlias 命令的执行函数
CmdExecStatus ActiveAlias::exec(const std::string& option) {
    std::vector<std::string> tokens;
    if (!CmdExec::lexOptions(option, tokens)) return CMD_EXEC_ERROR;
    if (!tokens.empty()) return errorOption(CMD_OPT_EXTRA, tokens[0]);
    _fmgr->listAlias();
    return CMD_EXEC_DONE;
}

// ActiveFlow 命令的执行函数
CmdExecStatus ActiveFlow::exec(const std::string& option) {
    std::vector<std::string> tokens;
    if (!CmdExec::lexOptions(option, tokens)) return CMD_EXEC_ERROR;
    if (!tokens.empty()) return errorOption(CMD_OPT_EXTRA, tokens[0]);

    _fmgr->listFlow();
    return CMD_EXEC_DONE;
}

// FlowType 命令的执行函数
CmdExecStatus FlowType::exec(const std::string& option) {
    std::vector<std::string> tokens;
    if (!CmdExec::lexOptions(option, tokens)) return CMD_EXEC_ERROR;
    if (!tokens.empty()) return errorOption(CMD_OPT_EXTRA, tokens[0]);
    _fmgr->flowType();
    return CMD_EXEC_DONE;
}

// SetParam 命令的执行函数
CmdExecStatus SetParam::exec(const std::string& option) {
    std::vector<std::string> tokens;
    if (!CmdExec::lexOptions(option, tokens, 4)) return CMD_EXEC_ERROR;
    return (_fmgr->setParam(tokens[0], tokens[1], tokens[2], tokens[3]) == 0) ? CMD_EXEC_DONE : CMD_EXEC_ERROR;
}

// PrintPort 命令的执行函数
CmdExecStatus PrintPort::exec(const std::string& option) {
    std::vector<std::string> tokens;
    if (!CmdExec::lexOptions(option, tokens, 3)) return CMD_EXEC_ERROR;
    return (_fmgr->listOutput(tokens[0], tokens[1], tokens[2]) == 0) ? CMD_EXEC_DONE : CMD_EXEC_ERROR;
}
