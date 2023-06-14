#pragma once

#include "Core.h"

#include "SString.h"
#include "SUtil.h"
#include "Structures/SList.h"
#include "Structures/SHashMap.h"

global_var constexpr uint32_t MAX_SUGGESTIONS = 6;

struct Command
{
	int(*Execute)(const SStringView cmd, const SList<SStringView>& args);
};

struct CommandMgr
{
	SHashMap<SRawString, Command> Commands;
	SList<SStringView> CommandNames;
	SList<SStringView> Suggestions;
	SList<SStringView> InputArgs;

	int Length;
	int LastLength;
	char TextInputMemory[96];

	CommandMgr();

	void RegisterCommand(const char* CmdName, const Command& cmd);
	void TryExecuteCommand(const SStringView input);
	void PopulateSuggestions(const SStringView input);
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
Argument<SRawString>
GetArgString(uint32_t index, const SList<SStringView>& args);

Argument<int>
GetArgInt(uint32_t index, const SList<SStringView>& args);

Argument<float>
GetArgFloat(uint32_t index, const SList<SStringView>& args);

Argument<Vector2>
GetArgVec2(uint32_t index, const SList<SStringView>& args);