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

internal int TestStringExecute(const SStringView cmd, const SList<SStringView>& args)
{
	SLOG_INFO("TEST COMMAND EXECUTE!");

	Argument<SStringView> arg0 = GetArgString(0, args);
	if (!arg0.IsPresent) return SetCmdError("Argument 0 not specified");

	auto arg1 = GetArgVec2(1, args);
	if (!arg1.IsPresent) return SetCmdError("Arg 1 not set");

	TraceLog(LOG_DEBUG, "TestStringCommand = %s, x = %f, y = %f",
		arg0.Value.Str , arg1.Value.x, arg1.Value.y);

	return CMD_SUCCESS;
}

internal int TestExecute(const SStringView cmd, const SList<SStringView>& args)
{
	SLOG_INFO("TEST COMMAND EXECUTE!");

	return CMD_SUCCESS;
}

internal int TestExecute2(const SStringView cmd, const SList<SStringView>& args)
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
	Commands.KeyEqualsFunction = STableDefaultKeyEquals;
	Commands.KeyHashFunction = SStringViewHash;

	Suggestions.Reserve(MAX_SUGGESTIONS);
	InputArgs.Reserve(5);

	Command testCommand = {};
	testCommand.Execute = TestExecute;

	Command testCommand2 = {};
	testCommand2.Execute = TestExecute2;

	Command stringCommand = { TestStringExecute };

	RegisterCommand("test", testCommand);
	RegisterCommand("test_command", testCommand);
	RegisterCommand("stringCommand", stringCommand);
	RegisterCommand("testCommand2", testCommand2);
}

void CommandMgr::RegisterCommand(const char* cmdName, const Command& cmd)
{
	SStringView cmdNameStringView(cmdName);

	CommandNames.Push(&cmdNameStringView);
	Commands.Put(&cmdNameStringView, &cmd);

	SLOG_INFO("[ Commands ] Registered command %s", cmdNameStringView.Str);
}

void CommandMgr::TryExecuteCommand(const SStringView input)
{
	if (input.Empty()) return;

	InputArgs.Clear();

	// Handle getting command and
	// splitting arguments
	const char delim = ' ';
	uint32_t start = 0;
	size_t end = 0;
	while ((end = input.FindChar(delim, start)) != SSTR_NO_POS)
	{
		SStringView subStr = input.SubString(start, end);
		InputArgs.Push(&subStr);
		start = end + 1;
	}
	SStringView subStr = input.SubString(start, input.End());
	InputArgs.Push(&subStr);

	// NOTE: The command name is the 1st
	// argument, so we remove and set
	// InputCommandStr as it
	STempString commandStr = InputArgs[0].ToTempString();
	SStringView commandStrView(commandStr.Str, commandStr.Length);

	InputArgs.RemoveAt(0);

	Command* foundCommand = Commands.Get(&commandStrView);
	if (foundCommand)
	{		
		int ret = foundCommand->Execute(commandStrView, InputArgs);
		if (ret == CMD_SUCCESS)
		{
			SLOG_INFO("Success cmd");
		}
		else
		{
			SLOG_ERR("[ Command ] Error: %s", LastCmdError);
		}
	}
	SMemClear(TextInputMemory, sizeof(TextInputMemory));
	TextInputMemory[sizeof(TextInputMemory) - 1] = '\0';
	Length = 0;
	LastLength = 0;
	Suggestions.Clear();
}

void CommandMgr::PopulateSuggestions(const SStringView input)
{
	if (Length <= 0) return;

	Suggestions.Clear();
	for (int i = 0; i < CommandNames.Count; ++i)
	{
		if (Suggestions.Count == MAX_SUGGESTIONS) break;
		if (CommandNames[i].Find(input.Str) != SSTR_NO_POS)
		{
			Suggestions.Push(CommandNames.PeekAt(i));
		}
	}
}

Argument<SStringView>
GetArgString(int index, const SList<SStringView>& args)
{
	Argument<SStringView> arg = {};
	if (index < args.Count)
	{
		arg.Value = args[index];
		arg.IsPresent = true;
	}
	return arg;
}

Argument<int>
GetArgInt(int index, const SList<SStringView>& args)
{
	Argument<int> arg = {};
	if (index >= args.Count) return arg;
	arg.Value = strtol(args[index].Str, NULL, 10);
	arg.IsPresent = true;
	if (errno)
	{
		char buffer[256];
		SLOG_ERR("GetArgInt: %s", strerror_s(buffer, errno));
	}
	return arg;
}

Argument<float>
GetArgFloat(int index, const SList<SStringView>& args)
{
	Argument<float> arg = {};
	if (index >= args.Count) return arg;
	arg.Value = strtof(args[index].Str, NULL);
	arg.IsPresent = true;
	if (errno)
	{
		char buffer[256];
		SLOG_ERR("GetArgFloat: %s", strerror_s(buffer, errno));
	}
	return arg;
}

Argument<Vector2>
GetArgVec2(int index, const SList<SStringView>& args)
{
	Argument<Vector2> arg = {};
	if (index >= args.Count) return arg;
	SStringView strView = args[index];
	uint32_t found = strView.FindChar(',');
	if (found != SSTR_NO_POS)
	{
		SStringView xStr = strView.SubString(0, found);
		SStringView yStr = strView.SubString(found + 1, strView.Length);
		arg.Value.x = strtof(xStr.Str, NULL);

		if (errno)
		{
			char buffer[256];
			SLOG_ERR("GetArgVec2(X field): %s", strerror_s(buffer, errno));
		}

		arg.Value.y = strtof(yStr.Str, NULL);

		if (errno)
		{
			char buffer[256];
			SLOG_ERR("GetArgVec2(Y field): %s", strerror_s(buffer, errno));
		}

		arg.IsPresent = true;
	}
	return arg;
}
