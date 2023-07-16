#include "CommandMgr.h"

#include "SMemory.h"

#include <stdlib.h>

#define CMD_SUCCESS 0
#define CMD_FAILURE 1

global_var const char* LastCmdError = "Unknown error";

int SetCmdError(const char* err)
{
	LastCmdError = err;
	return CMD_FAILURE;
}

internal int TestStringExecute(const SString cmd, const SList<SString>& args)
{
	SLOG_INFO("TEST COMMAND EXECUTE!");

	Argument<SRawString> arg0 = GetArgString(0, args);
	if (!arg0.IsPresent) return SetCmdError("Argument 0 not specified");

	auto arg1 = GetArgVec2(1, args);
	if (!arg1.IsPresent) return SetCmdError("Arg 1 not set");

	TraceLog(LOG_DEBUG, "TestStringCommand = %s, x = %f, y = %f",
		arg0.Value.Data , arg1.Value.x, arg1.Value.y);

	return CMD_SUCCESS;
}

internal int TestExecute(const SString cmd, const SList<SString>& args)
{
	SLOG_INFO("TEST COMMAND EXECUTE!");

	return CMD_SUCCESS;
}

internal int TestExecute2(const SString cmd, const SList<SString>& args)
{
	SLOG_INFO("TEST 2 COMMAND EXECUTE!");

	Argument<int> arg0 = GetArgInt(0, args);
	if (!arg0.IsPresent) return SetCmdError("1st argument needs to be an int");

	Argument<float> arg1 = GetArgFloat(1, args);
	if (!arg1.IsPresent) return SetCmdError("2nd argument needs to be a float");

	TraceLog(LOG_DEBUG, "Arg0 = %d, Arg1 = %f", arg0.Value, arg1.Value);
	return CMD_SUCCESS;
}

CommandMgr::CommandMgr()
{
	ConsoleEntries.Initialize(100, CONSOLE_STR_LENGTH);

	Suggestions.Reserve(CONSOLE_MAX_SUGGETIONS);
	InputArgs.Reserve(5);

	Command testCommand = {};
	testCommand.Execute = TestExecute;

	Command testCommand2 = {};
	testCommand2.Execute = TestExecute2;
	testCommand2.ArgString = RawStringNew("[int] [float]", SAllocator::Game);

	Command stringCommand = {};
	stringCommand.Execute = TestStringExecute;
	stringCommand.ArgString = RawStringNew("[arg0(String)] [arg1(Vector2)]", SAllocator::Game);

	RegisterCommand("test", testCommand);
	RegisterCommand("test_command", testCommand);
	RegisterCommand("stringCommand", stringCommand);
	RegisterCommand("testCommand2", testCommand2);
}

void CommandMgr::RegisterCommand(const char* cmdName, const Command& cmd)
{
	SRawString string = RawStringNew(cmdName, SAllocator::Game);

	Commands.Insert(&string, &cmd);
	CommandNames.Push(&string);

	SLOG_INFO("[ Commands ] Registered command %s", cmdName);
}

void CommandMgr::TryExecuteCommand(SString input)
{
	if (input.Empty())
		return;

	InputArgs.Clear();

	// Handle getting command and
	// splitting arguments
	uint32_t start = 0;
	uint32_t end = 0;
	while ((end = input.Substring(' ', start)) != SSTR_NO_POS)
	{
		SString slice = input.Slice(start, end, SAllocator::Temp);
		InputArgs.Push(&slice);
		start = end + 1;
	}
	SString slice = input.Slice(start, input.Length, SAllocator::Temp);
	InputArgs.Push(&slice);

	// NOTE: The command name is the 1st
	// argument, so we remove and set
	// InputCommandStr as it
	
	SString cmdStr = InputArgs[0];
	InputArgs.RemoveAt(0);

	SRawString tmp = RawStringNew(cmdStr.Data(), cmdStr.Length, SAllocator::Temp);
	Command* foundCommand = Commands.Get(&tmp);
	if (foundCommand)
	{		
		int ret = foundCommand->Execute(cmdStr, InputArgs);
		if (ret == CMD_SUCCESS)
		{
			SLOG_INFO("Success cmd");
		}
		else
		{
			SLOG_ERR("[ Command ] Error: %s", LastCmdError);
		}
	}
}

void CommandMgr::PopulateSuggestions(SString input)
{
	if (input.Empty())
		return;

	Suggestions.Clear();
	for (uint32_t i = 0; i < CommandNames.Count; ++i)
	{
		if (Suggestions.Count == CONSOLE_MAX_SUGGETIONS)
			break;

		uint32_t idx = StrFind(CommandNames[i].Data, input.Data());
		if (idx != SSTR_NO_POS)
		{
			SString sug = SString(CommandNames[i].Data, CommandNames[i].Length, SAllocator::Temp);
			Suggestions.Push(&sug);
		}
	}
}

const char* CommandMgr::PopulateArgSuggestions(SString input)
{
	if (input.Empty())
		return nullptr;

	// Get first word
	uint32_t find = input.Substring(' ', 0);
	if (find == SSTR_NO_POS)
		find = input.Length;

	SString cmdStr = input.Slice(0, find, SAllocator::Temp);

	if (cmdStr.Empty())
		return nullptr;

	SRawString tmp = RawStringNew(cmdStr.Data(), cmdStr.Length, SAllocator::Temp);
	Command* cmd = Commands.Get(&tmp);
	
	if (cmd)
		return cmd->ArgString.Data;
	else
		return nullptr;
}

Argument<SRawString>
GetArgString(uint32_t index, const SList<SString>& args)
{
	Argument<SRawString> arg = {};
	if (index < args.Count)
	{
		arg.Value = RawStringNew(args[index].Data(), args[index].Length, SAllocator::Temp);
		arg.IsPresent = true;
	}
	return arg;
}

Argument<int>
GetArgInt(uint32_t index, const SList<SString>& args)
{
	Argument<int> arg = {};
	if (index >= args.Count) return arg;
	arg.Value = strtol(args[index].Data(), NULL, 10);
	arg.IsPresent = true;
	if (errno)
	{
		char buffer[256];
		SLOG_ERR("GetArgInt: %s", strerror_s(buffer, errno));
	}
	return arg;
}

Argument<float>
GetArgFloat(uint32_t index, const SList<SString>& args)
{
	Argument<float> arg = {};
	if (index >= args.Count) return arg;
	arg.Value = strtof(args[index].Data(), NULL);
	arg.IsPresent = true;
	if (errno)
	{
		char buffer[256];
		SLOG_ERR("GetArgFloat: %s", strerror_s(buffer, errno));
	}
	return arg;
}

Argument<Vector2>
GetArgVec2(uint32_t index, const SList<SString>& args)
{
	Argument<Vector2> arg = {};
	if (index >= args.Count) return arg;
	SString strView = args[index];
	uint32_t found = strView.FindChar(',');
	if (found != SSTR_NO_POS)
	{
		SString xStr = strView.FirstTemp(0, found);
		SString yStr = strView.FirstTemp(found + 1, strView.Length);
		arg.Value.x = strtof(xStr.Data(), NULL);

		if (errno)
		{
			char buffer[256];
			SLOG_ERR("GetArgVec2(X field): %s", strerror_s(buffer, errno));
		}

		arg.Value.y = strtof(yStr.Data(), NULL);

		if (errno)
		{
			char buffer[256];
			SLOG_ERR("GetArgVec2(Y field): %s", strerror_s(buffer, errno));
		}

		arg.IsPresent = true;
	}
	return arg;
}
