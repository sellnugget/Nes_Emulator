#include "Cartridge.h"

namespace Hardware {

	Cartridge::Cartridge(std::string path,Bus* bus)
	{
		_bus = bus;
		LoadINES(path);
		PrintRomContents();
	}
	void Cartridge::LoadINES(std::string path)
	{
		std::ifstream file(path, std::ifstream::in|std::ifstream::binary);


		
		uint8_t header[16];
		file.read((char*)header, 16);
		//header[4] is the number of 16kb chunks the rom needs
		_programsize = 0x4000 * header[5];
		_Program_Rom = new uint8_t[_programsize];

		_charsize = 0x2000;
		_charram = true;
		if (header[6] != 0) {
			_charram = false;
			_charsize *= header[6];
			
		}
		_Char_Rom = new uint8_t[_charsize];

		//read rom and program data from file
		file.read((char*)_Program_Rom,_programsize);

		file.read((char*)_Char_Rom, _charsize);

		file.close();
	}
	void Cartridge::PrintRomContents()
	{
		for (int i = 0; i < _programsize; i++) {
			std::cout << std::hex << i <<  ": " << _Program_Rom[i] << "\n";
		}
	}
}