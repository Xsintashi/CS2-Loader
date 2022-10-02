#pragma once
#include <optional>
#include <string>
#include <vector>

class GlobalVars{
public:
	int numberArguments;
	std::vector<std::string> arguments;
	std::string steamPath;
	std::string gameArgs;
};

inline std::optional<GlobalVars> global;
