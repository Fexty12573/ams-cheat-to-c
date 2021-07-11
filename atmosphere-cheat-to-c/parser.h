#pragma once

#include "state.h"

#include <fstream>
#include <string>

class CheatCodeParser
{
public:
	CheatCodeParser(const std::string& filepath);

	const std::string& Parse();

private:
	void ParseCodetype(CodeType type, const std::string& line, size_t& ptr);
	void ParseLine(const std::string& line);

	CodeType GetCodeType(const std::string& line, size_t& ptr);

	inline void PrependIndents(std::string& nl);
	void AddOutputLine(const std::string& line);

	void ReadAddress(const std::string& line, size_t& ptr, char* addr);
	void ReadValue(const std::string& line, size_t& ptr, char* value, const char datatype = '4');
	void ReadCount(const std::string& line, size_t& ptr, char* count);
	void ReadKey(const std::string& line, size_t& ptr, char* keymap);
	void ReadOffset(const std::string& line, size_t& ptr, char* offset);

	void GotoNextDWord(const std::string& line, size_t& ptr);

	void FinishOutput();

private:
	std::string code;
	std::string output;

	std::ifstream file;
	std::ofstream out;

	int numIndents = 0;
	bool firstCode = true;
};
