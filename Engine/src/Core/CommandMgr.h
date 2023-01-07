#pragma once

#include "Core.h"

#include <unordered_map>
#include <vector>
#include <string>

constexpr int MAX_SUGGESTIONS = 6;

struct Command
{
	int(*Execute)(const std::string_view cmd, const std::vector<std::string_view>& args);
};

struct CommandMgr
{
	std::unordered_map<std::string, Command> Commands;
	std::vector<std::string_view> CommandsNames;
	std::vector<std::string_view> Suggestions;
	std::vector<std::string_view> InputArgs;
	int Length;
	int LastLength;
	char TextInputMemory[96];

	CommandMgr();

	void RegisterCommand(std::string_view CmdName, const Command& cmd);
	void TryExecuteCommand(const std::string_view input);
	void PopulateSuggestions(const std::string_view input);
};

/// Copies err to a global error state 
/// and returns 1 (error)
int SetCmdError(const char* err);

template<typename T>
struct Argument
{
	T Value;
	bool IsPresent;
};

/// NOTE: This returns a string view from the command input string!
/// This means that the string is temporary and the null 
/// terminator is at the end of the command not the string!
Argument<std::string_view>
GetArgString(int index, const std::vector<std::string_view>& args);

Argument<int>
GetArgInt(int index, const std::vector<std::string_view>& args);

Argument<float>
GetArgFloat(int index, const std::vector<std::string_view>& args);

Argument<Vector2>
GetArgVec2(int index, const std::vector<std::string_view>& args);