#pragma once
#include <iostream>
#include <functional>
#include "ErrorHandle.h"
namespace Hardware {
	class Bus
	{
		
	private:
		enum Modual {
			INTERNAL_RAM, MIRROR_RAM, PPU_REGISTER,
			PPU_REGISTER_MIRROR, APU_IO_REGISTERS,
			DISABLED_APU_IO, CARTRIDGE_SPACE
		};
	


		uint8_t* internalRam;
		Modual* MemMap;
		//ordered so internal ram is the first device
		static std::function<uint8_t* (uint16_t address, Bus* bus)> OnDeviceRead[7];
		static std::function<void (uint16_t address, uint8_t data, Bus* bus)> OnDeviceWrite[7];
	public:
		Bus();
		void WriteByte(uint16_t address, uint8_t data);
		uint8_t ReadByte(uint16_t address);
	};

}

