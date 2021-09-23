#pragma once

#include <string>

namespace TheGame
{
	class Identifier
	{
	public:
		static const std::string DefaultNamespace;
		static const Identifier EmptyIdentifier;

		const std::string Namespace;
		const std::string Path;

	public:
		Identifier(const std::string& path);
		Identifier(const std::string& idNamespace, const std::string& path);

		std::string ToString() const;

		static Identifier ParseString(const std::string& string);
	};
}