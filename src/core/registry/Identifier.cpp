#include "Identifier.h"

#include "GameHeaders.h"

namespace TheGame
{
	const std::string Identifier::DefaultNamespace("TheGame");
	const Identifier Identifier::EmptyIdentifier("NULL", "Empty");

	Identifier::Identifier(const std::string& path)
		: Namespace(DefaultNamespace), Path(path)
	{
	}

	Identifier::Identifier(const std::string& idNamespace, const std::string& path)
		: Namespace(idNamespace), Path(path)
	{
	}

	std::string Identifier::ToString() const
	{
		return std::string(Namespace + ":" + Path);
	}

	Identifier Identifier::ParseString(const std::string& string)
	{
		size_t index = string.find(':');
		if (index < 1)
		{
			TraceLog(LOG_ERROR, "Could not parse string to Identifier, no (:)! String: %s", string);
			return EmptyIdentifier;
		}
		if (string.starts_with(':'))
		{
			TraceLog(LOG_WARNING, "Parsed Identifier has no namespace! String: %s", string);
			return Identifier("", string.substr(index));
		}
		return Identifier(string.substr(0, index - 1), string.substr(index));
	}
}

