#include "EMU_6502.h"

namespace Hardware {


	void EMU_6502::Execute(bool onecycle)
	{

		//since these are being call so often they have to be very efficient. the less checks per instruction the better
		long nano_per_cycle = 1000000000 / clock_speed;
		do {
			_additional_cycles = 0;
			uint8_t current = GetNextByte();
			std::string name = InstructionMatrix[current].name;

			//std::cout << _programCounter - 1 << " " << name << "\n";
			opcode = current;
			int cycles = InstructionMatrix[current].cycles;
			(this->*InstructionMatrix[current].addrmode)();
			(this->*InstructionMatrix[current].operate)();
			cycles += _additional_cycles;
			_total_cycles += cycles;
			if (_bus->IRQ) {
				IRQ();
				_bus->IRQ = false;
			}
			if (_bus->NMI) {
				NMI();
				_bus->NMI = false;
			}
		} while (!onecycle);
	}

	EMU_6502::EMU_6502(Bus* bus)
	{
		_bus = bus;
	}

	void EMU_6502::IRQ()
	{
		if (!_interrupt) {
			// Push the program counter to the stack. It's 16-bits dont
			// forget so that takes two pushes
			PushWord(_programCounter);
			// Then Push the status register to the stack
			PushByte(PackprocessorStatus());

			// Read new program counter location from fixed address
			_tempword = 0xFFFE;
			uint16_t lo = ReadByte(_tempword + 0);
			uint16_t hi = ReadByte(_tempword + 1);
			_programCounter = (hi << 8) | lo;
			// IRQs take time
			_total_cycles += 7;
		}
	}

	void EMU_6502::NMI()
	{
		WriteByte(0x0100 + _stackPointer, (_programCounter >> 8) & 0x00FF);
		_stackPointer--;
		WriteByte(0x0100 + _stackPointer, _programCounter & 0x00FF);
		_stackPointer--;

		_breakcommand = 0;
		_interrupt = 1;
		WriteByte(0x0100 + _stackPointer, PackprocessorStatus());
		_stackPointer--;

		_tempword = 0xFFFA;
		uint16_t lo = ReadByte(_tempword + 0);
		uint16_t hi = ReadByte(_tempword + 1);
		_programCounter = (hi << 8) | lo;

		_total_cycles += 8;

	}

	void EMU_6502::Reset()
	{
		// Get address to set program counter to
		_tempword = 0xFFFC;
		uint16_t lo = ReadByte(_tempword + 0);
		uint16_t hi = ReadByte(_tempword + 1);

		// Set it
		_programCounter = (hi << 8) | lo;

		// Reset internal registers
		_accumulator = 0;
		_indexX = 0;
		_indexY = 0;
		_stackPointer = 0xFD;
		SetUnpackedFlags(0);

		// Clear internal helper variables
		_tempword = 0x0000;
		_tempbyte = 0x00;

		// Reset takes time
		_total_cycles += 8;
	}
	void EMU_6502::JAM()
	{
		//catches illegal op codes
	}
	void EMU_6502::ADC()
	{
		IsImmed();
		uint8_t add = _accumulator + _tempbyte + _carry;
		if (0xff - _accumulator < (_tempbyte + _carry)) {
			//overflow occured
			_carry = 1;
		}
		else {
			_carry = 0;
		}
		_overflow = 0;
		if (_accumulator < 128 && _tempbyte < 128 && add > 127) {
			_overflow = 1;
		}
		OnSetFlag(add);
		_accumulator = add;
		_additional_cycles += 1;
	}

	void EMU_6502::AND()
	{
		IsImmed();
		_accumulator &= _tempbyte;
		OnSetFlag(_accumulator);
		_additional_cycles += 1;
	}

	void EMU_6502::ASL()
	{
		IsImmed();
		uint16_t temp = (uint16_t)_tempbyte << 1;
		_carry = (temp & 0xFF00) > 0;
		_zero = (temp & 0x00FF) == 0x00;
		_negative = temp & 0x80;
		if (InstructionMatrix[opcode].addrmode == &EMU_6502::IMP)
			_accumulator = temp & 0x00FF;
		else
			WriteByte(_tempword, temp & 0x00FF);
		OnSetFlag(_tempbyte);
	}

	void EMU_6502::BCC()
	{
		if (!_carry) {
			_additional_cycles++;
			_tempword = _programCounter + _tempword;

			if ((_tempword & 0xFF00) != (_programCounter & 0xFF00))
				_additional_cycles++;

			_programCounter = _tempword;
		}
	}

	void EMU_6502::BCS()
	{
		if (_carry) {
			_additional_cycles++;
			_tempword = _programCounter + _tempword;

			if ((_tempword & 0xFF00) != (_programCounter & 0xFF00))
				_additional_cycles++;

			_programCounter = _tempword;
		}
	}

	void EMU_6502::BEQ()
	{
		if (_zero) {
			_additional_cycles++;
			_tempword = _programCounter + _tempword;

			if ((_tempword & 0xFF00) != (_programCounter & 0xFF00))
				_additional_cycles++;

			_programCounter = _tempword;
		}
	}

	void EMU_6502::BIT()
	{
		IsImmed();
		uint8_t anded = _accumulator & _tempbyte;
		if (anded == 0) {
			_zero = 1;
		}
		else {
			_zero = 0;
		}
		_overflow = _tempbyte >> 6 & 0b1;
		_negative = _tempbyte >> 7;

	}

	void EMU_6502::BMI()
	{
		if (_negative) {
			_additional_cycles++;
			_tempword = _programCounter + _tempword;

			if ((_tempword & 0xFF00) != (_programCounter & 0xFF00))
				_additional_cycles++;

			_programCounter = _tempword;
		}
	}

	void EMU_6502::BNE()
	{
		if (!_zero) {
			_additional_cycles++;
			_tempword = _programCounter + _tempword;

			if ((_tempword & 0xFF00) != (_programCounter & 0xFF00))
				_additional_cycles++;

			_programCounter = _tempword;
		}
	}

	void EMU_6502::BPL()
	{
		if (!_negative) {
			_additional_cycles++;
			_tempword = _programCounter + _tempword;

			if ((_tempword & 0xFF00) != (_programCounter & 0xFF00))
				_additional_cycles++;

			_programCounter = _tempword;
		}
	}

	void EMU_6502::BRK()
	{
		_programCounter++;

		_interrupt = 1;
		WriteByte(0x0100 + _stackPointer, (_programCounter >> 8) & 0x00FF);
		_stackPointer--;
		WriteByte(0x0100 + _stackPointer, _programCounter & 0x00FF);
		_stackPointer--;
		_breakcommand = 1;
		WriteByte(0x0100 + _stackPointer, PackprocessorStatus());
		_stackPointer--;
		_breakcommand = 0;

		_programCounter = (uint16_t)ReadByte(0xFFFE) | ((uint16_t)ReadByte(0xFFFF) << 8);
	}

	void EMU_6502::BVC()
	{
		if (!_overflow) {
			_additional_cycles++;
			_tempword = _programCounter + _tempword;

			if ((_tempword & 0xFF00) != (_programCounter & 0xFF00))
				_additional_cycles++;

			_programCounter = _tempword;
		}
	}

	void EMU_6502::BVS()
	{
		if (_overflow) {
			_additional_cycles++;
			_tempword = _programCounter + _tempword;

			if ((_tempword & 0xFF00) != (_programCounter & 0xFF00))
				_additional_cycles++;

			_programCounter = _tempword;
		}
	}

	void EMU_6502::CLC()
	{
		_carry = 0;
	}

	void EMU_6502::CLD()
	{
		_decimalmode = 0;
	}

	void EMU_6502::CLI()
	{
		_interrupt = 0;
	}

	void EMU_6502::CLV()
	{
		_overflow = 0;
	}

	void EMU_6502::CMP()
	{
		IsImmed();
		uint8_t cmpresult = _accumulator - _tempbyte;

		_carry = _accumulator >= _tempbyte;
		OnSetFlag(cmpresult);
		_additional_cycles += 1;
	}

	void EMU_6502::CPX()
	{
		IsImmed();
		uint8_t cmpresult = _indexX - _tempbyte;

		_carry = _indexX >= _tempbyte;
		OnSetFlag(cmpresult);
	}

	void EMU_6502::CPY()
	{
		IsImmed();
		uint8_t cmpresult = _indexY - _tempbyte;

		_carry = _indexY >= _tempbyte;
		OnSetFlag(cmpresult);
	}

	void EMU_6502::DEC()
	{
		IsImmed();
		uint16_t temp = _tempbyte - 1;
		WriteByte(_tempword, temp & 0x00ff);
		OnSetFlag(temp);
	}

	void EMU_6502::DEX()
	{
		_indexX -= 1;
		OnSetFlag(_indexX);
	}

	void EMU_6502::DEY()
	{
		_indexY -= 1;
		OnSetFlag(_indexY);
	}

	void EMU_6502::EOR()
	{
		IsImmed();
		_accumulator ^= _tempbyte;
		OnSetFlag(_accumulator);
	}

	void EMU_6502::INC()
	{
		IsImmed();
		_tempbyte += 1;
		WriteByte(_tempword, _tempbyte);
		OnSetFlag(_tempbyte);
	}

	void EMU_6502::INX()
	{
		_indexX += 1;
		OnSetFlag(_indexX);
	}

	void EMU_6502::INY()
	{
		_indexY += 1;
		OnSetFlag(_indexY);
	}

	void EMU_6502::JMP()
	{
		_programCounter = _tempword;
	}

	void EMU_6502::JSR()
	{
		_programCounter--;

		WriteByte(0x0100 + _stackPointer, (_programCounter >> 8) & 0x00FF);
		_stackPointer--;
		WriteByte(0x0100 + _stackPointer, _programCounter & 0x00FF);
		_stackPointer--;

		_programCounter = _tempword;
	}

	void EMU_6502::LDA()
	{
		IsImmed();
		_accumulator = _tempbyte;
		OnSetFlag(_accumulator);
		_additional_cycles += 1;
	}

	void EMU_6502::LDX()
	{
		IsImmed();
		_indexX = _tempbyte;
		OnSetFlag(_indexX);
		_additional_cycles += 1;
	}

	void EMU_6502::LDY()
	{
		IsImmed();
		_indexY = _tempbyte;
		OnSetFlag(_indexY);
		_additional_cycles += 1;
	}

	void EMU_6502::LSR()
	{
		IsImmed();
		_carry = _tempbyte & 0x0001;
		uint16_t temp = _tempbyte >> 1;
		_zero = (temp & 0x00FF) == 0x0000;
		_negative = temp & 0x0080;
		if (InstructionMatrix[opcode].addrmode == &EMU_6502::IMP)
			_accumulator = temp & 0x00FF;
		else
			WriteByte(_tempword, temp & 0x00FF);
	}

	void EMU_6502::NOP()
	{
		if (_currentmode != IMPLICIT) {
			_additional_cycles += 1;
		}
	}

	void EMU_6502::ORA()
	{
		IsImmed();
		_accumulator |= _tempbyte;
		OnSetFlag(_accumulator);
		_additional_cycles += 1;
	}

	void EMU_6502::PHA()
	{
		PushByte(_accumulator);
	}

	void EMU_6502::PHP()
	{
		PushByte(PackprocessorStatus());
		_breakcommand = 0;

	}

	void EMU_6502::PLA()
	{
		_accumulator = PopByte();
		OnSetFlag(_accumulator);
	}

	void EMU_6502::PLP()
	{
		SetUnpackedFlags(PopByte());
	}

	void EMU_6502::ROL()
	{
		IsImmed();
		uint16_t temp = (uint16_t)(_tempbyte << 1) | _carry;
		_carry = temp & 0xFF00;
		_zero = (temp & 0x00FF) == 0x0000;
		_negative = temp & 0x0080;
		if (InstructionMatrix[opcode].addrmode == &EMU_6502::IMP)
			_accumulator = temp & 0x00FF;
		else
			WriteByte(_tempword, temp & 0x00FF);
	}

	void EMU_6502::ROR()
	{
		IsImmed();
		uint16_t temp = (uint16_t)(_carry << 7) | (_tempbyte >> 1);
		_carry = _tempbyte & 0x01;
		_zero = (temp & 0x00FF) == 0x00;
		_negative = temp & 0x0080;
		if (InstructionMatrix[opcode].addrmode == &EMU_6502::IMP)
			_accumulator = temp & 0x00FF;
		else
			WriteByte(_tempword, temp & 0x00FF);
	}

	void EMU_6502::RTI()
	{
		
		_stackPointer++;
		SetUnpackedFlags(ReadByte(0x0100 + _stackPointer));
		_breakcommand = !_breakcommand;

		_stackPointer++;
		_programCounter = (uint16_t)ReadByte(0x0100 + _stackPointer);
		_stackPointer++;
		_programCounter |= (uint16_t)ReadByte(0x0100 + _stackPointer) << 8;
	}

	void EMU_6502::RTS()
	{
		_stackPointer++;
		_programCounter = (uint16_t)ReadByte(0x0100 + _stackPointer);
		_stackPointer++;
		_programCounter |= (uint16_t)ReadByte(0x0100 + _stackPointer) << 8;

		_programCounter++;
	}

	void EMU_6502::SBC()
	{
		IsImmed();
		uint16_t value = ((uint16_t)_tempbyte) ^ 0x00FF;

		// Notice this is exactly the same as addition from here!
		uint16_t temp = (uint16_t)_accumulator + value + (uint16_t)_carry;
		_carry = temp & 0xFF00;
		_zero = ((temp & 0x00FF) == 0);
		_overflow = (temp ^ (uint16_t)_accumulator) & (temp ^ value) & 0x0080;
		_negative = temp & 0x0080;
		_accumulator = temp & 0x00FF;
	}

	void EMU_6502::SEC()
	{
		_carry = 1;
	}

	void EMU_6502::SED()
	{
		_decimalmode = 1;
	}

	void EMU_6502::SEI()
	{
		_interrupt = 1;
	}

	void EMU_6502::STA()
	{
		WriteByte(_tempword, _accumulator);
	}

	void EMU_6502::STX()
	{
		WriteByte(_tempword, _indexX);
	}

	void EMU_6502::STY()
	{
		WriteByte(_tempword, _indexY);
	}

	void EMU_6502::TAX()
	{
		_indexX = _accumulator;
		OnSetFlag(_indexX);
	}

	void EMU_6502::TAY()
	{
		_indexY = _accumulator;
		OnSetFlag(_indexY);
	}

	void EMU_6502::TSX()
	{
		_indexX = _stackPointer;
		OnSetFlag(_indexX);
	}

	void EMU_6502::TXA()
	{
		_accumulator = _indexX;
		OnSetFlag(_accumulator);
	}

	void EMU_6502::TXS()
	{
		_stackPointer = _indexX;
	}

	void EMU_6502::TYA()
	{
		_accumulator = _indexY;
		OnSetFlag(_accumulator);
	}

	void EMU_6502::IMP()
	{
		_tempbyte = _accumulator;
		_currentmode = IMPLICIT;
	}

	void EMU_6502::ACU()
	{
		_tempbyte = _accumulator;
		_currentmode = ACCUMULATOR;
	}

	void EMU_6502::IMM()
	{
		_tempbyte = GetNextByte(); _tempword = _tempbyte;
		_currentmode = IMMEDIATE;
	}

	void EMU_6502::ZP0()
	{
		_tempword = ReadByte(_programCounter);
		_programCounter++;
		_tempword &= 0x00FF;
		_currentmode = ZEROPAGE;
	}

	void EMU_6502::ZPX()
	{
		_tempword = (ReadByte(_programCounter) + _indexX);
		_programCounter++;
		_tempword &= 0x00FF;
		_currentmode = ZEROPAGE_X;
	}

	void EMU_6502::ZPY()
	{
		_tempword = (ReadByte(_programCounter) + _indexY);
		_programCounter++;
		_tempword &= 0x00FF;
		_currentmode = ZEROPAGE_Y;
	}

	void EMU_6502::REL()
	{
		_tempword = ReadByte(_programCounter);
		_programCounter++;
		if (_tempword & 0x80)
			_tempword |= 0xFF00;
		_currentmode = RELATIVE;
	}

	void EMU_6502::ABS()
	{
		uint16_t lo = ReadByte(_programCounter);
		_programCounter++;
		uint16_t hi = ReadByte(_programCounter);
		_programCounter++;

		_tempword = (hi << 8) | lo;
		_currentmode = ABSOLUTE;
	}

	void EMU_6502::ABX()
	{
		uint16_t lo = GetNextByte();
		uint16_t hi = GetNextByte();

		_tempword = (hi << 8) | lo;
		_tempword += _indexX;

		if ((_tempword & 0xFF00) != (hi << 8))
			_additional_cycles += 1;

		_currentmode = ABSOLUTE_X;
	}

	void EMU_6502::ABY()
	{
		uint16_t lo = GetNextByte();
		uint16_t hi = GetNextByte();

		_tempword = (hi << 8) | lo;
		_tempword += _indexY;

		if ((_tempword & 0xFF00) != (hi << 8))
			_additional_cycles += 1;

		_currentmode = ABSOLUTE_Y;
	}

	void EMU_6502::IND()
	{
		uint16_t ptr_lo = ReadByte(_programCounter);
		_programCounter++;
		uint16_t ptr_hi = ReadByte(_programCounter);
		_programCounter++;

		uint16_t ptr = (ptr_hi << 8) | ptr_lo;

		if (ptr_lo == 0x00FF) // Simulate page boundary hardware bug
		{
			_tempword = (ReadByte(ptr & 0xFF00) << 8) | ReadByte(ptr + 0);
		}
		else // Behave normally
		{
			_tempword = (ReadByte(ptr + 1) << 8) | ReadByte(ptr + 0);
			if (_tempword < 0x8000 && _tempword > 0x4000) {
				std::cout << "x:" << _indexX << " y:" << _indexY << " a:" << _accumulator;
			}
		}
		_currentmode = INDIRECT;
	}

	void EMU_6502::IID()
	{
		uint16_t t = ReadByte(_programCounter);
		_programCounter++;

		uint16_t lo = ReadByte((uint16_t)(t + (uint16_t)_indexX) & 0x00FF);
		uint16_t hi = ReadByte((uint16_t)(t + (uint16_t)_indexX + 1) & 0x00FF);

		_tempword = (hi << 8) | lo;
		_currentmode = INDIRECT_INDEXED;
	}

	void EMU_6502::IDI()
	{
		uint16_t t = GetNextByte();

		uint16_t lo = ReadByte(t & 0x00FF);
		uint16_t hi = ReadByte((t + 1) & 0x00FF);

		_tempword = (hi << 8) | lo;
		_tempword += _indexY;

		if ((_tempword & 0xFF00) != (hi << 8))
			_additional_cycles += 1;

		_currentmode = INDEXED_INDIRECT;
	}
}
