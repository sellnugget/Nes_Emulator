#include "Bus.h"

namespace Hardware {

    Bus::Bus()
    {
        MemMap = new Modual[0xffff];
        internalRam = new uint8_t[0x800];
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
    }
    void Bus::WriteByte(uint16_t address, uint8_t data)
    {
    }
    uint8_t Bus::ReadByte(uint16_t address)
    {
        return *OnDeviceRead[MemMap[address]](address, this);
    }
    std::function <uint8_t* (uint16_t address, Bus* bus)> Bus::OnDeviceRead[7] =
    {
        [](uint16_t address, Bus* bus) {return &bus->internalRam[address]; },
        [](uint16_t address, Bus* bus) { return &bus->internalRam[address % 0x800]; },
        //nes ppu registers
        [](uint16_t address, Bus* bus) { return nullptr; },
        //mirror of $2000-$2007
        [](uint16_t address, Bus* bus) { return nullptr; },
        //NES APU and I/O registers
        [](uint16_t address, Bus* bus) { return nullptr; },
        // 	APU and I/O functionality that is normally disabled
        [](uint16_t address, Bus* bus) { return nullptr; },
        //Cartridge space: PRG ROM, PRG RAM, and mapper registers
        [](uint16_t address, Bus* bus) { return nullptr; },



    };

    std::function <void (uint16_t address, uint8_t data, Bus* bus)> Bus::OnDeviceWrite[7] =
    {
        [](uint16_t address, uint8_t data, Bus* bus) {bus->internalRam[address] = data; },
        [](uint16_t address, uint8_t data, Bus* bus) { bus->internalRam[address % 0x800] = data; },
        //nes ppu registers
        [](uint16_t address, uint8_t data, Bus* bus) { ; },
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
