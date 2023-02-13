#pragma once
#include <iostream>
#include <functional>
#include "ErrorHandle.h"
#include <vector>
namespace Hardware {
	class Bus
	{
		
	private:
		
		//must be a int because of performance reasons
		uint8_t* _MemMap;
		//these arguments can be overloaded so that the bus does not need a reference to other devices

		std::function<uint8_t(uint16_t addres)> OnDeviceRead[0xff];
		std::function<void(uint16_t address, uint8_t data)> OnDeviceWrite[0xff];

		std::map<std::string, uint8_t> _slots;
		std::map<uint8_t, std::string> _slotnames;
		int _currentnewslot;

	public:
		Bus();
		void BindReadDevice(std::string name, std::function<uint8_t(uint16_t address)> function);
		void BindWriteDevice(std::string name, std::function<void(uint16_t address, uint8_t data)> function);
		void BindDevice(std::string name);

		void WriteByte(uint16_t address, uint8_t data, bool print = false);
		uint8_t ReadByte(uint16_t address, bool print = false);
		
		//tocall indicates the functions that will be called on read or write
		void MapRegion(std::string tocall, uint16_t location, uint16_t size);

		bool NMI;
		bool IRQ;

	};

}

