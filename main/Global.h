#pragma once
#include <optional>
#include <string>
#include <vector>

class GlobalVars{
public:
	int numberArguments;
	std::vector<std::string> arguments;
	char* output;
};

inline std::optional<GlobalVars> global;
