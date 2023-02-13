#pragma once
#include "Bus.h"
#include <string>
namespace Hardware {
	class RamModual
	{
		Bus* _bus;
	public:
		RamModual(Bus* bus, uint32_t size, uint16_t mapoffset, int mirror_after = 0);

		uint8_t* _Ram;
		uint32_t size;
	};
}


