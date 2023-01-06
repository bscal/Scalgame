#pragma once

#include "Core.h"

#include <unordered_map>
#include <vector>
#include <string>

constexpr int MAX_SUGGESTIONS = 6;

enum ArgType
{
	String,
	Int,
	Float
};

template<typename T>
struct Argument
{
	T Value;
	bool IsPresent;
};

inline std::string_view GetArgString(std::string_view arg)
{
	return arg;
}

inline Argument<int> GetArgInt(int index, const std::vector<std::string_view>& args)
{
	Argument<int> arg = {};
	if (index < args.size()) return arg;
	try
	{
		arg.Value = std::stoi(args[index].data());
		arg.IsPresent = true;
	}
	catch (std::exception e)
	{
		SLOG_ERR("GetArgInt: %s", e.what());
	}
	return arg;
}

inline Argument<float> GetArgFloat(int index, const std::vector<std::string_view>& args)
{
	Argument<float> arg = {};
	if (index < args.size()) return arg;
	try
	{
		arg.Value = std::stoi(args[index].data());
		arg.IsPresent = true;
	}
	catch (std::exception e)
	{
		SLOG_ERR("GetArgFloat: %s", e.what());
	}
	return arg;
}

struct Command
{
	std::unordered_map<std::string, Command> SubCommands;

	uint8_t ReqArgs;

	int(*Execute)(const std::string_view cmd, const std::vector<std::string_view>& args);
};

struct CommandMgr
{
	std::unordered_map<std::string, Command> Commands;
	std::vector<std::string_view> CommandsNames;
	std::vector<std::string_view> Suggestions;
	std::string_view InputCommandStr;
	std::vector<std::string_view> InputArguments;

	CommandMgr();

	void RegisterCommand(std::string_view CmdName, const Command& cmd);
	void TryExecuteCommand();
	void UpdateInputCommand(const std::string_view input);

};

/// Copies err to a global error state 
/// and returns 1 (error)
int SetCmdError(const char* err);
