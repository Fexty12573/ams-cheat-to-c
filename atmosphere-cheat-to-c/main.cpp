
#include "parser.h"

#include <iostream>
#include <fstream>

int main(int argc, char* argv[])
{
	if (argc > 1)
	{
		CheatCodeParser cheatparser(argv[1]);

		std::ofstream outfile("out.c");

		const std::string& out = cheatparser.Parse();
		outfile.write(out.data(), out.size());
	}
	else
	{
		std::string file;
		std::cout << "Enter filepath: ";
		std::getline(std::cin, file);

		CheatCodeParser cheatparser(file);

		std::ofstream outfile("out.c");

		const std::string& out = cheatparser.Parse();
		outfile.write(out.data(), out.size());
	}

	std::cout << "Result written to out.c\nHit Enter to exit.";
	std::cin.get();

	return 0;
}