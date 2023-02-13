#include "Bus.h"
#include <iomanip>
namespace Hardware {
    void Bus::MapRegion(std::string tocall, uint16_t location, uint16_t size)
    {
        if ((int)location + (int)size > 0x10000) {
            Error::Error(Error::IncorrectMap);
        }
        if (!_slots.count(tocall)) {
            //this is when a unbinded function is used
            Error::Error(Error::IncorrectMap);
        }
        for (int i = location; i < location + size; i++) {
            _MemMap[i] = _slots[tocall];
        }
    }

    void Bus::BindDevice(std::string name) {
        _slots[name] = _currentnewslot;
        _slotnames[_currentnewslot] = name;
        _currentnewslot++;
    }

    void Bus::BindReadDevice(std::string name, std::function<uint8_t(uint16_t address)> function)
    {
        OnDeviceRead[_slots[name]] = function;
    }

    void Bus::BindWriteDevice(std::string name, std::function<void(uint16_t address, uint8_t data)> function) {
        OnDeviceWrite[_slots[name]] = function;
    }
    Bus::Bus()
    {
        _MemMap = new uint8_t[0xffff];
        for (int i = 0; i < 255; i++) {
            OnDeviceRead[i] = [](uint16_t address) {return 0; };
            OnDeviceWrite[i] = [](uint16_t address, uint8_t data) { };
        }

    }
    void Bus::WriteByte(uint16_t address, uint8_t data, bool print)
    {
        if (print) {
            std::cout << "0x";
            if (address < 0x1000) {
                std::cout << "0";
                if (address < 0x100) {
                    std::cout << "0";
                }
            }
            std::cout << std::hex << address << " w from device:" << _slotnames[_MemMap[address]] << "\n";
        }
        OnDeviceWrite[_MemMap[address]](address, data);
    }
    uint8_t Bus::ReadByte(uint16_t address, bool print)
    {
        if (print) {
            std::cout << "0x";
            if (address < 0x1000) {
                std::cout << "0";
                if (address < 0x100) {
                    std::cout << "0";
                }
            }
            std::cout << std::hex << address << " r from device:" << _slotnames[_MemMap[address]] << "\n";
        } 
        return OnDeviceRead[_MemMap[address]](address);
    }

}
