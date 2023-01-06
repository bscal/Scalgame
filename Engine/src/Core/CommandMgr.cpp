#include "CommandMgr.h"

#define CMD_SUCCESS 0
#define CMD_FAILURE 1

global_var std::string LastCmdError = "Unknown error";

int SetCmdError(const char* err)
{
	LastCmdError = std::string(err);
	return CMD_FAILURE;
}

internal int TestExecute(const std::string_view cmd, const std::vector<std::string_view>& args)
{
	SLOG_INFO("TEST COMMAND EXECUTE!");

	return CMD_SUCCESS;
}

internal int TestExecute2(const std::string_view cmd, const std::vector<std::string_view>& args)
{
	SLOG_INFO("TEST 2 COMMAND EXECUTE!");

	SetCmdError("TestExecute2!");

	return CMD_FAILURE;
}

CommandMgr::CommandMgr()
{
	Suggestions.reserve(MAX_SUGGESTIONS);
	InputArguments.reserve(4);

	Command testCommand = {};
	testCommand.Execute = TestExecute;

	Command testCommand2 = {};
	testCommand2.Execute = TestExecute2;

	RegisterCommand("test", testCommand);
	RegisterCommand("test_command", testCommand);
	RegisterCommand("fake_command", testCommand);
	RegisterCommand("not_command", testCommand2);
}

void CommandMgr::RegisterCommand(std::string_view CmdName, const Command& cmd)
{
	Commands.emplace(std::make_pair(CmdName, cmd));
	CommandsNames.emplace_back(CmdName);

	SLOG_INFO("[ Commands ] Registered command %s", CmdName.data());
}

void CommandMgr::TryExecuteCommand()
{
	auto find = Commands.find(InputCommandStr.data());
	if (find != Commands.end())
	{		
		int ret = find->second.Execute(InputCommandStr, InputArguments);
		if (ret == CMD_SUCCESS)
		{
			SLOG_INFO("Success cmd");
		}
		else
		{
			SLOG_ERR("[ Command ] Error: %s", LastCmdError.c_str());
		}
	}
	Suggestions.clear();
	InputArguments.clear();
	InputCommandStr = {};
}

void CommandMgr::UpdateInputCommand(const std::string_view input)
{
	if (!input.empty())
	{
		// Handle getting command and
		// splitting arguments
		std::string_view curCommand = {};
		const char delim = ' ';
		std::size_t start = 0;
		std::size_t end = input.find(delim);
		while (end != std::string::npos)
		{
			std::string_view substr = input.substr(start, end - start);
			if (curCommand.empty()) curCommand = substr;
			else InputArguments.push_back(substr);
			start = end + 1;
			end = input.find(delim, start);
		}
		
		if (curCommand.empty()) curCommand = input;

		Suggestions.clear();
		for (int i = 0; i < CommandsNames.size(); ++i)
		{
			if (Suggestions.size() == MAX_SUGGESTIONS) break;
			if (CommandsNames[i].find(curCommand) != std::string::npos)
			{
				Suggestions.emplace_back(CommandsNames[i]);
			}
		}
		InputCommandStr = curCommand;
	}
}
