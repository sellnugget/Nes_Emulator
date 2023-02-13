#include "RamModual.h"


namespace Hardware {

	RamModual::RamModual(Bus* bus, uint32_t size, uint16_t mapoffset, int mirror_after)
	{
		_bus = bus;
		_Ram = new uint8_t[size];

		std::function<uint8_t(uint16_t address)> onread = [this, size](uint16_t address)
		{
			return _Ram[address % size];
		};
		std::function<void(uint16_t address, uint8_t data)> onwrite = [this, size](uint16_t address, uint8_t data)
		{
			_Ram[address % size] = data;
		};
	
		std::string modualname = std::string("RamModual") + std::to_string(size);\
		_bus->BindDevice(modualname);
		_bus->BindReadDevice(modualname, onread);
		_bus->BindWriteDevice(modualname, onwrite);
		_bus->MapRegion(modualname, mapoffset, size * (mirror_after + 1));
	
	}
}

