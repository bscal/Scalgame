#pragma once

namespace TheGame
{
	class GameServer
	{

	public:
		int StartClient();
		int StartHost();
		int StartServer();

	private:
		void Init();

	};

}