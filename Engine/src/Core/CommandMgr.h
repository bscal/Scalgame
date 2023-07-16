#pragma once

#include "Core.h"

#include "SString.h"
#include "SUtil.h"

#include "Structures/StaticArray.h"
#include "Structures/SList.h"
#include "Structures/SHashMap.h"

struct Command
{
	int(*Execute)(const SString cmd, const SList<SString>& args);

	SRawString ArgString;
};

constexpr global_var int CONSOLE_STR_LENGTH = 60;
constexpr global_var int CONSOLE_MAX_SUGGETIONS = 5;

struct CommandMgr
{
	char Input[CONSOLE_STR_LENGTH];
	int Length;

	SStringsBuffer ConsoleEntries;

	SHashMap<SRawString, Command, SRawStringHasher> Commands;
	SList<SRawString> CommandNames;

	SList<SString> Suggestions;
	SList<SString> SuggestionsArgs;

	SList<SString> InputArgs;

	CommandMgr();

	void RegisterCommand(const char* CmdName, const Command& cmd);
	void TryExecuteCommand(SString input);
	void PopulateSuggestions(SString input);
	const char* PopulateArgSuggestions(SString input);
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
GetArgString(uint32_t index, const SList<SString>& args);

Argument<int>
GetArgInt(uint32_t index, const SList<SString>& args);

Argument<float>
GetArgFloat(uint32_t index, const SList<SString>& args);

Argument<Vector2>
GetArgVec2(uint32_t index, const SList<SString>& args);