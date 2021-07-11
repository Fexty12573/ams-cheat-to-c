
#include "parser.h"
#include "convert.h"
#include "keys.h"

#include <iostream>

CheatCodeParser::CheatCodeParser(const std::string& filepath)
{
	file.open(filepath);
}

const std::string& CheatCodeParser::Parse()
{
	std::string line;

	AddOutputLine("typedef unsigned char u8;\n");
	AddOutputLine("typedef unsigned short u16;\n");
	AddOutputLine("typedef unsigned int u32;\n");
	AddOutputLine("typedef unsigned long long u64;\n");
	AddOutputLine("// Declarations so intellisense can do nice syntax highlighting :)\n");
	AddOutputLine("u64 Main, Heap;\n");
	AddOutputLine("_Bool AreKeysPressed(u32);\n");

	while (std::getline(file, line))
	{
		ParseLine(line);
	}

	FinishOutput();

	return output;
}

void CheatCodeParser::ParseLine(const std::string& line)
{
	size_t ptr = 0;
	CodeType type = GetCodeType(line, ptr);
	
	if (type == CodeType::NEW_CODE_START)
	{
		if (!firstCode) FinishOutput();
		else firstCode = false;

		AddOutputLine("\n/* " + line + " */\n");
		AddOutputLine("void Cheat() {\n");
		numIndents = 1;

		std::string regs = "\tu64 ";
		for (auto& [i, reg] : Registers)
		{
			regs += reg;
			regs += ", ";
		}
		regs = regs.replace(regs.size() - 2, 2, ";\n");
		AddOutputLine(regs);
	}
	else
	{
		ParseCodetype(type, line, ptr);
	}
}

CodeType CheatCodeParser::GetCodeType(const std::string& line, size_t& ptr)
{
	if (line[ptr] == ' ' || line[ptr] == '\t') GotoNextDWord(line, ptr);
	switch (line[ptr++])
	{
	case '0': return CodeType::STORE_MEMORY_STATIC;
	case '1': return CodeType::BEGIN_CONDITIONAL;
	case '2': return CodeType::END_CONDITIONAL;
	case '3': return CodeType::LOOP_START_END;
	case '4': return CodeType::LOAD_REGISTER_STATIC;
	case '5': return CodeType::LOAD_REGISTER_FROM_ADDRESS;
	case '6': return CodeType::STORE_REGISTER_ADDRESS_STATIC;
	case '7': return CodeType::ARITHMETIC_LEGACY;
	case '8': return CodeType::KEYPRESS_CONITIONAL;
	case '9': return CodeType::ARITHMETIC;
	case 'A': return CodeType::STORE_REGISTER_TO_ADDRESS;
	case 'B': return CodeType::RESERVED;
	case 'C':
		switch (line[ptr++])
		{
		case '0': return CodeType::BEGIN_REGISTER_CONDITIONAL;
		case '1': return CodeType::SAVE_RESTORE_REGISTER;
		case '2': return CodeType::SAVE_RESTORE_REGISTER_W_MASK;
		case '3': return CodeType::READ_WRITE_STATIC_REGISTER;
		default: return CodeType::ERROR_TYPE;
		}
	case 'F':
		switch (line[ptr++])
		{
		case 'F':
			switch (line[ptr++])
			{
			case '0': return CodeType::PAUSE_PROCESS;
			case '1': return CodeType::RESUME_PROCESS;
			case 'F': return CodeType::DEBUG_LOG;
			default: return CodeType::ERROR_TYPE;
			}
		default: return CodeType::ERROR_TYPE;
		}
	case '[': return CodeType::NEW_CODE_START;
	case '\0': return CodeType::EMPTY_LINE;
	default: return CodeType::ERROR_TYPE;
	}
}

void CheatCodeParser::ParseCodetype(CodeType type, const std::string& line, size_t& ptr)
{
	switch (type)
	{
	case CodeType::STORE_MEMORY_STATIC: // Type 0
	{
		const char datatype = line[ptr];
		const char regsuffix = line[ptr++];
		const char region = line[ptr++];
		const char reg = line[ptr++];

		ptr += 2; // skip 00

		char addr[11] = { 0 };
		ReadAddress(line, ptr, addr);

		char value[17] = { 0 }; memset(value, 0, 17);
		ReadValue(line, ptr, value);

		char nl[100] = { 0 };
		sprintf_s(nl, 100, "(*(%s*)(%s + 0x%s)) = (%s)0x%s; // using %s%s as offset register\n",
			Datatypes[datatype], Regions[region], addr,
			Datatypes[datatype], value, Registers[reg], RegSuffix[regsuffix]);

		std::string nl_(nl);
		PrependIndents(nl_);

		AddOutputLine(nl_);
		break;
	}
	case CodeType::BEGIN_CONDITIONAL: // Type 1
	{
		char datatype = line[ptr++];
		char region = line[ptr++];
		char comparison = line[ptr++];

		char addr[11] = { 0 };
		ReadAddress(line, ptr, addr);

		char value[17] = { 0 }; memset(value, 0, 17);
		ReadValue(line, ptr, value, datatype);

		char nl[60] = { 0 };
		sprintf_s(nl, 60, "\tif ((*(%s*)(%s + 0x%s)) %s 0x%s) {\n", Datatypes[datatype], 
			Regions[region], addr, Comparisons[comparison], value);

		std::string nl_(line);
		PrependIndents(nl_);

		std::cout << nl_ << std::endl;
		AddOutputLine(nl_);

		numIndents++;
		break;
	}
	case CodeType::END_CONDITIONAL: // Type 2
	{
		if (numIndents > 1)
		{
			numIndents--;
			std::string nl = "";
			PrependIndents(nl);

			AddOutputLine(nl + "}\n");
		}
		
		break;
	}
	case CodeType::LOOP_START_END: // Type 3
	{
		switch (line[ptr++])
		{
		case '0':
		{
			char reg = line[++ptr];
			GotoNextDWord(line, ptr);

			char count[9] = { 0 };
			ReadCount(line, ptr, count);

			char nl[60] = { 0 };
			sprintf_s(nl, 60, "for (%s = 0; %s < %d; %s++) {\n", Registers[reg],
				Registers[reg], strtol(count, NULL, 16), Registers[reg]);

			std::string nl_(nl);
			PrependIndents(nl_);
			numIndents++;

			AddOutputLine(nl_);
			break;
		}
		case '1':
		{
			numIndents--;
			std::string nl = "}\n";
			PrependIndents(nl);

			AddOutputLine(nl);
			break;
		}
		}
		break;
	}
	case CodeType::LOAD_REGISTER_STATIC: // Type 4
	{
		ptr += 2;
		const char* reg = Registers[ptr];

		GotoNextDWord(line, ptr);

		char value[17] = { 0 }; memset(value, 0, 17);
		ReadValue(line, ptr, value, '8');

		char nl[30] = { 0 };
		sprintf_s(nl, 30, "%s = %d;\n", reg, strtol(value, NULL, 16));

		std::string nl_(nl);
		PrependIndents(nl_);

		AddOutputLine(nl_);
		break;
	}
	case CodeType::LOAD_REGISTER_FROM_ADDRESS: // Type 5
	{
		char datatype = line[ptr++];
		char region = line[ptr++];
		char reg = line[ptr++];

		char optype = line[ptr++]; ptr++;

		char addr[11] = { 0 };
		ReadAddress(line, ptr, addr);
		char nl[80] = { 0 };

		if (optype == '0')
		{
			sprintf_s(nl, 80, "%s%s = (*(%s*)(%s + 0x%s));\n", Registers[reg], RegSuffix[datatype],
				Datatypes[datatype], Regions[region], addr);
		}
		else if (optype == '1')
		{
			sprintf_s(nl, 80, "%s%s = (*(%s*)(%s + 0x%s));\n", Registers[reg], RegSuffix[datatype],
				Datatypes[datatype], Registers[reg], addr);
		}
		else
		{
			sprintf_s(nl, 80, "/* Invalid instruction */\n");
		}

		std::string nl_(nl);
		PrependIndents(nl_);

		AddOutputLine(nl_);
		break;
	}
	case CodeType::STORE_REGISTER_ADDRESS_STATIC: // Type 6
	{
		char datatype = line[ptr++]; ptr++;
		char basereg = line[ptr++];
		char inc = line[ptr++];
		bool useOffsetReg = line[ptr++] == '1';
		char offsetreg = line[ptr++];

		GotoNextDWord(line, ptr);
		char value[17] = { 0 }; memset(value, 0, 17);
		ReadValue(line, ptr, value, datatype);

		char nl[60] = { 0 };
		if (useOffsetReg)
		{
			sprintf_s(nl, 60, "(*(%s*)(%s + %s)) = (%s)0x%s;\n", Datatypes[datatype],
				Registers[basereg], Registers[offsetreg], Datatypes[datatype], value);
		}
		else
		{
			sprintf_s(nl, 60, "(*(%s*)%s) = (%s)0x%s;\n", Datatypes[datatype],
				Registers[basereg], Datatypes[datatype], value);
		}

		std::string nl_(nl);
		PrependIndents(nl_);

		AddOutputLine(nl_);
		break;
	}
	case CodeType::ARITHMETIC_LEGACY: // Type 7
	{
		char datatype = line[ptr++]; ptr++;
		char reg = line[ptr++];
		char operation = line[ptr++];
		GotoNextDWord(line, ptr);

		char value[17] = { 0 };
		ReadValue(line, ptr, value, datatype);

		char nl[60] = { 0 };
		sprintf_s(nl, 60, "%s %s 0x%X;\n", Registers[reg], OperationsLegacy[operation], strtol(value, NULL, 16));

		std::string nl_(nl);
		PrependIndents(nl_);

		AddOutputLine(nl_);
		break;
	}
	case CodeType::KEYPRESS_CONITIONAL: // Type 8
	{
		char keymap[8] = { 0 };
		ReadKey(line, ptr, keymap);

		char nl[60] = { 0 };
		sprintf_s(nl, 60, "if (AreKeysPressed(0x%s)) {\n", keymap);

		std::string nl_(nl);
		PrependIndents(nl_);

		numIndents++;

		AddOutputLine(nl_);
		break;
	}
	case CodeType::ARITHMETIC: // Type 9
	{
		char datatype = line[ptr++];
		char operation = line[ptr++];
		char result = line[ptr++];
		char lh = line[ptr++];

		const char* suffix = RegSuffix[datatype];

		switch (line[ptr++])
		{
		case '0':
		{
			char rh = line[ptr++];

			char nl[60] = { 0 };

			if (operation == '7' || operation == '9')
			{
				sprintf_s(nl, 60, "%s%s = %s%s%s;\n", Registers[result], suffix,
					Operations[operation], Registers[lh], suffix);
			}
			else
			{
				
				sprintf_s(nl, 60, "%s%s = %s%s %s %s%s;\n", Registers[result], suffix,
					Registers[lh], suffix, Operations[operation],
					Registers[rh], suffix);
			}

			std::string nl_(nl);
			PrependIndents(nl_);

			AddOutputLine(nl_);
			break;
		}
		case '1':
		{
			GotoNextDWord(line, ptr);

			char value[17] = { 0 };
			ReadValue(line, ptr, value, datatype);
			
			char nl[60] = { 0 };

			if (operation == '7' || operation == '9')
			{
				sprintf_s(nl, 60, "%s%s = %s%s%s\n", Registers[result], suffix,
					Operations[operation], Registers[lh], suffix);
			}
			else
			{
				sprintf_s(nl, 60, "%s%s = %s%s %s 0x%s\n", Registers[result], suffix,
					Registers[lh], suffix, Operations[operation], value);
			}

			std::string nl_(nl);
			PrependIndents(nl_);

			AddOutputLine(nl_);
			break;
		}
		}
		break;
	}
	case CodeType::STORE_REGISTER_TO_ADDRESS: // Type A
	{
		char datatype = line[ptr++];
		char reg = line[ptr++];
		char basereg = line[ptr++];
		char inc = line[ptr++];
		char offsettype = line[ptr++];
		char offsetreg_region = line[ptr++];

		const char* suffix = RegSuffix[datatype];

		char value[17] = { 0 };
		char nl[60] = { 0 };

		switch (offsettype)
		{
		case '0':
			sprintf_s(nl, 60, "(*(%s*)%s) = %s%s;\n", Datatypes[datatype], Registers[basereg], Registers[reg], suffix);
			break;
		case '1':
			sprintf_s(nl, 60, "(*(%s*)(%s + %s)) = %s%s;\n", Datatypes[datatype], Registers[basereg],
				Registers[offsetreg_region], Registers[reg], suffix);
			break;
		case '2':
			ReadOffset(line, ptr, value);
			sprintf_s(nl, 60, "(*(%s*)(%s + 0x%s)) = %s%s;\n", Datatypes[datatype], Registers[basereg],
				value, Registers[reg], suffix);
			break;
		case '3':
			sprintf_s(nl, 60, "(*(%s*)(%s + %s)) = %s%s;\n", Datatypes[datatype], Regions[offsetreg_region],
				Registers[basereg], Registers[reg], suffix);
			break;
		case '4':
			ReadOffset(line, ptr, value);
			sprintf_s(nl, 60, "(*(%s*)(%s + 0x%s)) = %s%s;\n", Datatypes[datatype], Regions[offsetreg_region],
				value, Registers[reg], suffix);
			break;
		case '5':
			ReadOffset(line, ptr, value);
			sprintf_s(nl, 60, "(*(%s*)(%s + 0x%s + %s)) = %s%s;\n", Datatypes[datatype], Regions[offsetreg_region],
				value, Registers[basereg], Registers[reg], suffix);
			break;
		default:
			sprintf_s(nl, 60, "/* Invalid line */\n");
			break;
		}

		std::string nl_(nl);
		PrependIndents(nl_);

		AddOutputLine(nl_);
		break;
	}
	case CodeType::RESERVED: // Type B
		break;
	case CodeType::BEGIN_REGISTER_CONDITIONAL: // Type C0
	{
		char datatype = line[ptr++];
		char operation = line[ptr++];
		char sourcereg = line[ptr++];
		char optype = line[ptr++];

		char nl[60] = { 0 };

		switch (optype)
		{
		case '0':
		{
			char region = line[ptr++];
			char offset[9] = { 0 };
			ReadOffset(line, ptr, offset);

			sprintf_s(nl, 60, "if (%s%s %s (*(%s*)(%s + 0x%s))) {\n", Registers[sourcereg], RegSuffix[datatype],
				Comparisons[operation], Datatypes[datatype], Regions[region], offset);
			break;
		}
		case '1':
		{
			char region = line[ptr++];
			char offsetreg = line[ptr++];

			sprintf_s(nl, 60, "if (%s%s %s (*(%s*)(%s + %s))) {\n", Registers[sourcereg], RegSuffix[datatype],
				Comparisons[operation], Datatypes[datatype], Regions[region], Registers[offsetreg]);
			break;
		}
		case '2':
		{
			char offsetreg = line[ptr++];
			char offset[9] = { 0 };
			ReadOffset(line, ptr, offset);

			sprintf_s(nl, 60, "if (%s%s %s (*(%s*)(%s + 0x%s))) {\n", Registers[sourcereg], RegSuffix[datatype],
				Comparisons[operation], Datatypes[datatype], Regions[offsetreg], offset);
			break;
		}
		case '3':
		{
			char reg = line[ptr++];
			char offsetreg = line[ptr++];

			sprintf_s(nl, 60, "if (%s%s %s (*(%s*)(%s + %s))) {\n", Registers[sourcereg], RegSuffix[datatype],
				Comparisons[operation], Datatypes[datatype], Registers[reg], Registers[offsetreg]);
			break;
		}
		case '4':
		{
			GotoNextDWord(line, ptr);
			char value[17] = { 0 };
			ReadValue(line, ptr, value, datatype);

			sprintf_s(nl, 60, "if (%s%s %s 0x%s) {\n", Registers[sourcereg], RegSuffix[datatype],
				Comparisons[operation], value);
			break;
		}
		case '5':
		{
			char rh = line[ptr++];

			sprintf_s(nl, 60, "if (%s%s %s %s%s) {\n", Registers[sourcereg], RegSuffix[datatype],
				Comparisons[operation], Registers[rh], RegSuffix[datatype]);
			break;
		}
		default:
			sprintf_s(nl, 60, "if (/* Invalid condition */) {\n");
			break;
		}

		std::string nl_(nl);
		PrependIndents(nl_);

		numIndents++;

		AddOutputLine(nl_);
		break;
	}
	case CodeType::SAVE_RESTORE_REGISTER: // Type C1
	{
		ptr++; char dst = line[ptr++];
		ptr++; char src = line[ptr++];
		char optype = line[ptr++];

		char nl[60] = { 0 };

		switch (optype)
		{
		case '0':
			sprintf_s(nl, 60, "RestoreRegister(%s, %s); // dst, src\n", Registers[dst], Registers[src]);
			break;
		case '1':
			sprintf_s(nl, 60, "SaveRegister(%s, %s); // dst, src\n", Registers[dst], Registers[src]);
			break;
		case '2':
			sprintf_s(nl, 60, "ClearSavedValue(%s, %s); // dst, src\n", Registers[dst], Registers[src]);
			break;
		case '3':
			sprintf_s(nl, 60, "ClearRegister(%s); // dst\n", Registers[dst]);
			break;
		}

		std::string nl_(nl);
		PrependIndents(nl_);

		AddOutputLine(nl_);
		break;
	}
	case CodeType::SAVE_RESTORE_REGISTER_W_MASK: // Type C2
	{
		char optype = line[ptr++]; ptr++;
		char mask[5] = { 0 };

		for (int i = 0; i < 4; i++, ptr++) mask[i] = line[ptr];

		char nl[60] = { 0 };

		switch (optype)
		{
		case '0':
			sprintf_s(nl, 60, "RestoreRegisters(0x%s);\n", mask);
			break;
		case '1':
			sprintf_s(nl, 60, "SaveRegisters(%s);\n", mask);
			break;
		case '2':
			sprintf_s(nl, 60, "ClearSavedValues(%s);\n", mask);
			break;
		case '3':
			sprintf_s(nl, 60, "ClearRegisters(%s);\n", mask);
			break;
		}

		std::string nl_(nl);
		PrependIndents(nl_);

		AddOutputLine(nl_);
		break;
	}
	case CodeType::READ_WRITE_STATIC_REGISTER: // Type C3
	{
		ptr += 3;
		char statreg[3] = { 0 };
		statreg[0] = line[ptr++];
		statreg[1] = line[ptr++];
		char reg = line[ptr++];

		char nl[30] = { 0 };
		if (strtoul(statreg, NULL, 16) < 0x80)
		{
			sprintf_s(nl, 30, "ReadStaticRegister(%s);\n", Registers[reg]);
		}
		else
		{
			sprintf_s(nl, 30, "WriteStaticRegister(%s);\n", Registers[reg]);
		}

		std::string nl_(nl);
		PrependIndents(nl_);

		AddOutputLine(nl_);
		break;
	}
	case CodeType::DOUBLE_EXTENDED_WIDTH_INSTRUCTION: // Type F0
		break;
	case CodeType::PAUSE_PROCESS: // Type FF0
	{
		std::string nl = "PauseProcess()";
		PrependIndents(nl);

		AddOutputLine(nl);
		break;
	}
	case CodeType::RESUME_PROCESS: // Type FF1
	{
		std::string nl = "ResumeProcess()";
		PrependIndents(nl);

		AddOutputLine(nl);
		break;
	}
	case CodeType::DEBUG_LOG: // Type FFF
	{
		char datatype = line[ptr++];
		char logid = line[ptr++];
		char optype = line[ptr++];

		char nl[60] = { 0 };

		switch (optype)
		{
		case '0':
		{
			char region = line[ptr++];
			char offset[10] = { 0 };
			ReadOffset(line, ptr, offset);

			sprintf_s(nl, 60, "Log(*(%s*)(%s + 0x%s));\n", Datatypes[datatype], Regions[region], offset);
			break;
		}
		case '1':
		{
			char region = line[ptr++];
			char offsetreg = line[ptr++];

			sprintf_s(nl, 60, "Log(*(%s*)(%s + %s));\n", Datatypes[datatype], Regions[region], Registers[offsetreg]);
		}
		case '2':
		{
			char reg = line[ptr++];
			char offset[10] = { 0 };
			ReadOffset(line, ptr, offset);

			sprintf_s(nl, 60, "Log(*(%s*)(%s + 0x%s));\n", Datatypes[datatype], Registers[reg], offset);
			break;
		}
		case '3':
		{
			char basereg = line[ptr++];
			char offsetreg = line[ptr++];

			sprintf_s(nl, 60, "Log(*(%s*)(%s + %s));\n", Datatypes[datatype], Registers[basereg], Registers[offsetreg]);
			break;
		}
		case '4':
		{
			char reg = line[ptr++];
			sprintf_s(nl, 60, "Log(%s);\n", Registers[reg]);
			break;
		}
		default:
			sprintf_s(nl, 60, "Log(/* Invalid Instruction */);\n"); break;
		}
		break;
	}
	case CodeType::END_CODE:
		FinishOutput();
		break;
	case CodeType::ERROR_TYPE:
		AddOutputLine("/* Invalid Instruction type */\n");
		break;
	case CodeType::EMPTY_LINE:
		AddOutputLine("\n");
		break;
	default:
		break;
	}
}

inline void CheatCodeParser::PrependIndents(std::string& nl)
{
	for (int i = 0; i < numIndents; i++) nl = "\t" + nl;
}

void CheatCodeParser::AddOutputLine(const std::string& line)
{
	output += line;
}

void CheatCodeParser::ReadAddress(const std::string& line, size_t& ptr, char* addr)
{
	int i = 0;
	while (i < 10)
	{
		if (line[ptr] != ' ')
		{
			addr[i] = line[ptr];
			i++;
		}
		ptr++;
	}
}

void CheatCodeParser::ReadValue(const std::string& line, size_t& ptr, char* value, const char datatype)
{
	int i = 0;
	
	if (datatype == '8')
	{
		while (i < 17 && ptr < line.size())
		{
			if (line[ptr] != ' ')
			{
				value[i] = line[ptr];
				i++;
			}
			ptr++;
		}
	}
	else
	{
		GotoNextDWord(line, ptr);

		if (ptr == line.size()) ptr -= 8; // Accidentally passed last Dword

		while (i < 9 && ptr < line.size())
		{
			if (line[ptr] != ' ')
			{
				value[i] = line[ptr];
				i++;
			}
			ptr++;
		}
	}

}

void CheatCodeParser::ReadCount(const std::string& line, size_t& ptr, char* count)
{
	for (int i = 0; i < 8, ptr < line.size(); i++, ptr++)
	{
		count[i] = line[ptr];
	}
}

void CheatCodeParser::ReadKey(const std::string& line, size_t& ptr, char* keymap)
{
	for (int i = 0; i < 7, ptr < line.size(); i++, ptr++) keymap[i] = line[ptr];
}

void CheatCodeParser::ReadOffset(const std::string& line, size_t& ptr, char* offset)
{
	int i = 0;
	while (i < 9 && ptr < line.size())
	{
		if (line[ptr] != ' ' && line[ptr] != '\t')
		{
			offset[i] = line[ptr];
			i++;
		}
		ptr++;
	}
}

void CheatCodeParser::GotoNextDWord(const std::string& line, size_t& ptr)
{
	while (line[ptr] != ' ' && line[ptr] != '\t' && ptr < line.size())
	{
		ptr++;
	}
	while (line[ptr] == ' ' || line[ptr] == '\t' && ptr < line.size())
	{
		ptr++;
	}
}

void CheatCodeParser::FinishOutput()
{
	while (numIndents > 0)
	{
		--numIndents;

		std::string brace = "}\n";
		PrependIndents(brace);

		output += brace;
	}
}
