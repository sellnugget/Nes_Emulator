#pragma once
#include <iostream>
#include <fstream> 
#include "Bus.h"
namespace Hardware {
	class Cartridge
	{
		uint8_t* _Program_Rom;
		size_t _programsize;

		uint8_t* _Char_Rom;
		size_t _charsize;
		bool _charram = false;

		Bus* _bus;
	public:
		Cartridge(std::string path, Bus* bus);
		//loads a .nes or ines file
		void LoadINES(std::string path);

		void PrintRomContents();
	};
}


