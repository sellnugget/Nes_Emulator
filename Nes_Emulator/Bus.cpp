#include "Bus.h"

namespace Hardware {

    Bus::Bus()
    {
        MemMap = new Modual[0xffff];
        internalRam = new uint8_t[0x800];

        //sets all device locations 
        uint16_t i = 0;
        for (; i < 0x0800; i++) {
            MemMap[i] = INTERNAL_RAM;
        }
        for (; i < 0x2000; i++) {
            MemMap[i] = MIRROR_RAM;
        }
        for (; i < 0x2008; i++) {
            MemMap[i] = PPU_REGISTER;
        }
        for (; i < 0x4000; i++) {
            MemMap[i] = PPU_REGISTER_MIRROR;
        }
        for (; i < 0x4018; i++) {
            MemMap[i] = APU_IO_REGISTERS;
        }
        for (; i < 0x4020; i++) {
            MemMap[i] = DISABLED_APU_IO;
        }
        for (; i != 0; i++) {
            MemMap[i] = CARTRIDGE_SPACE;
        }

        //initializes memory to zero

        for (i = 0; i < 0x800; i++) {
            internalRam[i] = 0;
        }




    }
    void Bus::WriteByte(uint16_t address, uint8_t data)
    {
        OnDeviceWrite[MemMap[address]](address, data, this);
    }
    uint8_t Bus::ReadByte(uint16_t address)
    {
        return OnDeviceRead[MemMap[address]](address, this);
    }
    std::function <uint8_t (uint16_t address, Bus* bus)> Bus::OnDeviceRead[7] =
    {
        [](uint16_t address, Bus* bus) {return bus->internalRam[address]; },
        [](uint16_t address, Bus* bus) { return bus->internalRam[address % 0x800]; },
        //nes ppu registers
        [](uint16_t address, Bus* bus) { return 0; },
        //mirror of $2000-$2007
        [](uint16_t address, Bus* bus) { return 0; },
        //NES APU and I/O registers
        [](uint16_t address, Bus* bus) { return 0; },
        // 	APU and I/O functionality that is normally disabled
        [](uint16_t address, Bus* bus) { return 0; },
        //Cartridge space: PRG ROM, PRG RAM, and mapper registers
        [](uint16_t address, Bus* bus) { return 0; },



    };

    std::function <void (uint16_t address, uint8_t data, Bus* bus)> Bus::OnDeviceWrite[7] =
    {
        [](uint16_t address, uint8_t data, Bus* bus) {bus->internalRam[address] = data; },
        [](uint16_t address, uint8_t data, Bus* bus) { bus->internalRam[address % 0x800] = data; },
        //nes ppu registers
        [](uint16_t address, uint8_t data, Bus* bus) { std::cout << (char)data; },
        //mirror of $2000-$2007
        [](uint16_t address, uint8_t data, Bus* bus) { ; },
        //NES APU and I/O registers
        [](uint16_t address, uint8_t data, Bus* bus) { ; },
        // 	APU and I/O functionality that is normally disabled
        [](uint16_t address, uint8_t data, Bus* bus) { ; },
        //Cartridge space: PRG ROM, PRG RAM, and mapper registers
        [](uint16_t address, uint8_t data, Bus* bus) {  },



    };

}
