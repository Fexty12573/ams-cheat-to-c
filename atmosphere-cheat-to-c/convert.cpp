
#include "convert.h"

std::map<char, const char*> Registers = {
	{'0', "R0"},
	{'1', "R1"},
	{'2', "R2"},
	{'3', "R3"},
	{'4', "R4"},
	{'5', "R5"},
	{'6', "R6"},
	{'7', "R7"},
	{'8', "R8"},
	{'9', "R9"},
	{'A', "R10"},
	{'B', "R11"},
	{'C', "R12"},
	{'D', "R13"},
	{'E', "R14"},
	{'F', "R15"}
};

std::map<char, const char*> RegSuffix = {
	{'1', "B"},
	{'2', "W"},
	{'4', "D"},
	{'8', ""}
};

std::map<char, const char*> Datatypes = {
	{'1', "u8"},
	{'2', "u16"},
	{'4', "u32"},
	{'8', "u64"}
};

std::map<char, const char*> Regions = {
	{'0', "Main"},
	{'1', "Heap"}
};

std::map<char, const char*> Comparisons = {
	{'1', ">"},
	{'2', ">="},
	{'3', "<"},
	{'4', "<="},
	{'5', "=="},
	{'6', "!="}
};

std::map<char, const char*> OperationsLegacy = {
	{'0', "+="},
	{'1', "-="},
	{'2', "*="},
	{'3', "<<="},
	{'4', ">>="}
};

std::map<char, const char*> Operations = {
	{'0', "+"},
	{'1', "-"},
	{'2', "*"},
	{'3', "<<"},
	{'4', ">>"},
	{'5', "&"},
	{'6', "|"},
	{'7', "~"},
	{'8', "^"},
	{'9', ""},
};
