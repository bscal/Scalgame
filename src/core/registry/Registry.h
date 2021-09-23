#pragma once

#include <cstdint>
#include <map>
#include <vector>

#include "Identifier.h"

namespace TheGame
{

	template<typename T>
	class Registry
	{
	private:
		std::map<Identifier, T> m_RegistryMap;
		std::vector<T> m_RegistryList;

	public:
		void Register(const Identifier& id, const T& value)
		{
			m_RegistryMap[id] = value;
			m_RegistryList.push_back(value);
		}

		T& GetValue(const Identifier& id)
		{
			return m_RegistryMap[id];
		}

		std::vector<T>& GetVector()
		{
			return *m_RegistryList;
		}
	};
}
