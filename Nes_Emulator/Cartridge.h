#pragma once
#include <iostream>
#include <fstream> 
#include "Bus.h"
namespace Hardware {

	class Cartridge
	{
		
	public:


		enum MIRROR {
			HORIZONTAL,
			VERTICAL,
			ONESCREEN_LO,
			ONESCREEN_HI
		} mirror = HORIZONTAL;
		uint8_t* _Program_Rom;
		size_t _programsize;

		uint8_t* _Char_Rom;
		size_t _charsize;
		bool _charram = false;
		Cartridge(std::string path);

		void MapToBus(Bus* _bus, uint16_t c_address, bool justchar);
		//loads a .nes or ines file
		void LoadINES(std::string path);
	};
}


