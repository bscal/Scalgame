#include "CommandMgr.h"

#include "SMemory.h"

#define CMD_SUCCESS 0
#define CMD_FAILURE 1

global_var std::string LastCmdError = "Unknown error";

int SetCmdError(const char* err)
{
	LastCmdError = std::string(err);
	return CMD_FAILURE;
}

internal int TestStringExecute(const std::string_view cmd, const std::vector<std::string_view>& args)
{
	SLOG_INFO("TEST COMMAND EXECUTE!");

	Argument<std::string_view> arg0 = GetArgString(0, args);
	if (!arg0.IsPresent) return SetCmdError("Argument 0 not specified");

	auto arg1 = GetArgVec2(1, args);
	if (!arg1.IsPresent) return SetCmdError("Arg 1 not set");

	std::string str(arg0.Value.begin(), arg0.Value.end());
	TraceLog(LOG_DEBUG, "TestStringCommand = %s, x = %f, y = %f",
		str.c_str(), arg1.Value.x, arg1.Value.y);

	return CMD_SUCCESS;
}

internal int TestExecute(const std::string_view cmd, const std::vector<std::string_view>& args)
{
	SLOG_INFO("TEST COMMAND EXECUTE!");

	return CMD_SUCCESS;
}

internal int TestExecute2(const std::string_view cmd, const std::vector<std::string_view>& args)
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
	Suggestions.reserve(MAX_SUGGESTIONS);
	InputArgs.reserve(5);

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

void CommandMgr::RegisterCommand(std::string_view CmdName, const Command& cmd)
{
	Commands.emplace(std::make_pair(CmdName, cmd));
	CommandsNames.emplace_back(CmdName);

	SLOG_INFO("[ Commands ] Registered command %s", CmdName.data());
}

void CommandMgr::TryExecuteCommand(const std::string_view input)
{
	if (input.empty()) return;

	InputArgs.clear();

	// Handle getting command and
	// splitting arguments
	const char delim = ' ';
	size_t start = 0;
	size_t end = 0;
	while ((end = input.find(delim, start)) != std::string::npos)
	{
		InputArgs.emplace_back(input.substr(start, end - start));
		start = end + 1;
	}
	InputArgs.emplace_back(input.substr(start));

	// NOTE: The command name is the 1st
	// argument, so we remove and set
	// InputCommandStr as it
	const std::string& command = std::string(InputArgs[0].data(), InputArgs[0].size());
	const char* inputCommand = InputArgs[0].data();

	InputArgs.erase(InputArgs.begin());

	auto find = Commands.find(command);
	if (find != Commands.end())
	{		
		int ret = find->second.Execute(command, InputArgs);
		if (ret == CMD_SUCCESS)
		{
			SLOG_INFO("Success cmd");
		}
		else
		{
			SLOG_ERR("[ Command ] Error: %s", LastCmdError.c_str());
		}
	}
	Scal::MemClear(TextInputMemory, sizeof(TextInputMemory));
	TextInputMemory[sizeof(TextInputMemory) - 1] = '\0';
	Length = 0;
	LastLength = 0;
	Suggestions.clear();
}

void CommandMgr::PopulateSuggestions(const std::string_view input)
{
	if (Length <= 0) return;

	Suggestions.clear();
	for (int i = 0; i < CommandsNames.size(); ++i)
	{
		if (Suggestions.size() == MAX_SUGGESTIONS) break;
		if (CommandsNames[i].find(input) != std::string::npos)
		{
			Suggestions.emplace_back(CommandsNames[i]);
		}
	}
}

Argument<std::string_view>
GetArgString(int index, const std::vector<std::string_view>& args)
{
	Argument<std::string_view> arg = {};
	if (index < args.size())
	{
		arg.Value = args[index];
		arg.IsPresent = true;
	}
	return arg;
}

Argument<int>
GetArgInt(int index, const std::vector<std::string_view>& args)
{
	Argument<int> arg = {};
	if (index >= args.size()) return arg;
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

Argument<float>
GetArgFloat(int index, const std::vector<std::string_view>& args)
{
	Argument<float> arg = {};
	if (index >= args.size()) return arg;
	try
	{
		arg.Value = std::stof(args[index].data());
		arg.IsPresent = true;
	}
	catch (std::exception e)
	{
		SLOG_ERR("GetArgFloat: %s", e.what());
	}
	return arg;
}

Argument<Vector2>
GetArgVec2(int index, const std::vector<std::string_view>& args)
{
	Argument<Vector2> arg = {};
	if (index >= args.size()) return arg;
	try
	{
		const auto& strView = args[index];
		size_t found = strView.find(',');
		if (found != std::string::npos)
		{
			auto xStr = strView.substr(0, found);
			auto yStr = strView.substr(found + 1, strView.size());
			arg.Value.x = std::stof(xStr.data());
			arg.Value.y = std::stof(yStr.data());
			arg.IsPresent = true;
		}
	}
	catch (std::exception e)
	{
		SLOG_ERR("GetArgVec2: %s", e.what());
	}
	return arg;
}
