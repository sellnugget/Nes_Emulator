#include "Cartridge.h"

namespace Hardware {

	Cartridge::Cartridge(std::string path)
	{
		LoadINES(path);
		
		//next bind to the bus

		
	}

	void Cartridge::MapToBus(Bus* _bus, uint16_t c_address, bool justchar)
	{
		_bus->BindDevice("CHARROM");
		_bus->BindReadDevice("CHARROM", [this](uint16_t address) {return _Char_Rom[address - 0x6000]; });
		_bus->MapRegion("CHARROM", c_address, _charsize);
		if (!justchar) {
			_bus->BindDevice("PRGROM");
			_bus->BindReadDevice("PRGROM", [this](uint16_t address) {return _Program_Rom[address - 0x8000]; });
			_bus->MapRegion("PRGROM", 0x2000 + c_address, 0x8000);
		}
		
		
	}


	void Cartridge::LoadINES(std::string path)
	{
		std::ifstream file(path, std::ifstream::in|std::ifstream::binary);


		
		uint8_t header[16];
		file.read((char*)header, 16);
		//header[4] is the number of 16kb chunks the rom needs
		_programsize = 0x4000 * header[4];
		_Program_Rom = new uint8_t[_programsize];

		_charsize = 0x2000;
		_charram = true;
		if (header[5] != 0) {
			_charram = false;
			_charsize *= header[5];
			
		}
		_Char_Rom = new uint8_t[_charsize];

		//read rom and program data from file
		file.read((char*)_Program_Rom,_programsize);

		file.read((char*)_Char_Rom, _charsize);

 		file.close();
	}
}