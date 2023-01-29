#include "EMU_6502.h"

namespace Hardware {

	void EMU_6502::ADC()
	{
		uint8_t add = _accumulator + *_tempbyte;
		if (0xff - _accumulator < *_tempbyte) {
			//overflow occured
			_carry = 1;
		}
		else {
			_carry = 0;
		}
		_overflow = 0;
		if (_accumulator < 128 && *_tempbyte < 128 && add > 127) {
			_overflow = 1;
		}
		OnSetFlag(add);
		_accumulator = add;
	}

	void EMU_6502::AND()
	{
		_accumulator &= *_tempbyte;
		OnSetFlag(_accumulator);
	}

	void EMU_6502::ASL()
	{
		if (_opmode != IMPLICIT && _opmode != ACCUMULATOR) {
			_carry = 0;
			if (_accumulator > 127) {
				_carry = 1;
			}

			_accumulator <<= 1;
			OnSetFlag(_accumulator);
		}
		else {
			_carry = 0;
			if (*_tempbyte > 127) {
				_carry = 1;
			}

			*_tempbyte <<= 1;
			OnSetFlag(*_tempbyte);
		}
	}

	void EMU_6502::BCC()
	{
		if (!_carry) {
			_programCounter = *_tempword;
		}
	}

	void EMU_6502::BCS()
	{
		if (_carry) {
			_programCounter = *_tempword;
		}
	}

	void EMU_6502::BEQ()
	{
		if (_zero) {
			_programCounter = *_tempword;
		}
	}

	void EMU_6502::BIT()
	{
		uint8_t anded = _accumulator & *_tempbyte;
		if (anded == 0) {
			_zero = 1;
		}
		else {
			_zero = 0;
		}
		_overflow = *_tempbyte >> 6 & 0b1;
		_negative = *_tempbyte >> 7;

	}

	void EMU_6502::BMI()
	{
		if (_negative) {
			_programCounter = *_tempword;
		}
	}

	void EMU_6502::BNE()
	{
		if (!_zero) {
			_programCounter = *_tempword;
		}
	}

	void EMU_6502::BPL()
	{
		if (!_negative) {
			_programCounter = *_tempword;
		}
	}

	void EMU_6502::BRK()
	{
		PushWord(_programCounter);
		PHP();
		_programCounter = LoadWord(0xfffe);
		_breakcommand = 1;
	}

	void EMU_6502::BVC()
	{
		if (!_overflow) {
			_programCounter = *_tempword;
		}
	}

	void EMU_6502::BVS()
	{
		if (_overflow) {
			_programCounter = *_tempword;
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
		uint8_t cmpresult = _accumulator - *_tempbyte;

		_carry = _accumulator >= *_tempbyte;
		OnSetFlag(cmpresult);
	}

	void EMU_6502::CPX()
	{
		uint8_t cmpresult = _indexX - *_tempbyte;

		_carry = _indexX >= *_tempbyte;
		OnSetFlag(cmpresult);
	}

	void EMU_6502::CPY()
	{
		uint8_t cmpresult = _indexY - *_tempbyte;

		_carry = _indexY >= *_tempbyte;
		OnSetFlag(cmpresult);
	}

	void EMU_6502::DEC()
	{
		*_tempbyte -= 1;
		OnSetFlag(*_tempbyte);
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
		_accumulator ^= *_tempbyte;
		OnSetFlag(_accumulator);
	}

	void EMU_6502::INC()
	{
		*_tempbyte += 1;
		OnSetFlag(*_tempbyte);
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
		_programCounter = *_tempword;
	}

	void EMU_6502::JSR()
	{
		PushByte(_programCounter);
		_programCounter = *_tempword;
	}

	void EMU_6502::LDA()
	{
		_accumulator = *_tempbyte;
		OnSetFlag(_accumulator);
	}

	void EMU_6502::LDX()
	{
		_indexX = *_tempbyte;
		OnSetFlag(_indexX);
	}

	void EMU_6502::LDY()
	{
		_indexY = *_tempbyte;
		OnSetFlag(_indexY);
	}

	void EMU_6502::LSR()
	{
		if (_opmode != IMPLICIT && _opmode != ACCUMULATOR) {
			_carry = 0;
			if (_accumulator & 0b1) {
				_carry = 1;
			}

			_accumulator >>= 1;
			OnSetFlag(_accumulator);
		}
		else {
			_carry = 0;
			if (*_tempbyte & 0b1) {
				_carry = 1;
			}

			*_tempbyte >>= 1;
			OnSetFlag(*_tempbyte);
		}
	}

	void EMU_6502::NOP()
	{
	}

	void EMU_6502::ORA()
	{
		_accumulator |= *_tempbyte;
		OnSetFlag(_accumulator);
	}

	void EMU_6502::PHA()
	{
		PushByte(_accumulator);
	}

	void EMU_6502::PHP()
	{
		PushByte(PackprocessorStatus());
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
		bool prevcarry = _carry;
		uint8_t* v = &_accumulator;
		if (_opmode == IMPLICIT || _opmode == ACCUMULATOR) {
			v = _tempbyte;
	
		}
		_carry = 0;
		if (*v > 127) {
			_carry = 1;
		}

		*v <<= 1;
		*v |= prevcarry;
		OnSetFlag(*v);
	}

	void EMU_6502::ROR()
	{
		bool prevcarry = _carry;
		uint8_t* v = &_accumulator;
		if (_opmode == IMPLICIT || _opmode == ACCUMULATOR) {
			v = _tempbyte;

		}
		_carry = 0;
		if (*v & 0b1) {
			_carry = 1;
		}

		*v >>= 1;
		*v |= prevcarry << 7;
		OnSetFlag(*v);
	}

	void EMU_6502::RTI()
	{
		SetUnpackedFlags(PopByte());
		_programCounter = PopWord();
	}

	void EMU_6502::RTS()
	{
		_programCounter = PopWord();
	}

	void EMU_6502::SBC()
	{
		uint8_t sub = _accumulator - *_tempbyte - ~_carry;
		if (0xff - _accumulator < *_tempbyte) {
			//overflow occured
			_carry = 0;
		}
		_overflow = 0;
		if (_accumulator < 128 && *_tempbyte < 128 && sub > 127) {
			_overflow = 1;
		}
		OnSetFlag(sub);
		_accumulator = sub;
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
		*_tempbyte = _accumulator;
	}

	void EMU_6502::STX()
	{
		*_tempbyte = _indexX;
	}

	void EMU_6502::STY()
	{
		*_tempbyte = _indexY;
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

}
