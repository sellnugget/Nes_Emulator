#include "EMU_6502.h"
#include <vector>
namespace Hardware {
	void EMU_6502::Execute()
	{
		_programCounter = 0x32;
		_interrupt = false;
		_accumulator = 0;
		_indexX = 0;
		_indexY = 0;
		_stackPointer = 0xfd;
		enum OPMODE {
			IMPLICIT, ACCUMULATOR, IMMEDIATE, ZEROPAGE, ZEROPAGE_X,
			ZEROPAGE_Y, RELATIVE, ABSOLUTE, ABSOLUTE_X, ABSOLUTE_Y,
			INDIRECT, INDEXED_INDIRECT, INDIRECT_INDEXED
		};
		//since these are being call so often they have to be very efficient. the less checks per instruction the better
		
		std::function<void()> mode_lambdas[] = {
		[this] {},//Implicit does nothing 
		[this] {_tempbyte = _accumulator; }, //Accumulator sets the tempbyte to the accumulator
		[this] {_tempbyte = GetNextByte(); }, //loads the next value into _tempbyte
		[this] {_tempword = GetNextByte(); _tempbyte = ReadByte(_tempword); },//uses next value as 8bit pointer.
		[this] {_tempword = (GetNextByte() + _indexX) % 256; _tempbyte = ReadByte(_tempword); }, //uses next value as 8bit pointer then adds x to that.
		[this] {_tempword = ((GetNextByte()) + _indexY) % 256; _tempbyte = ReadByte(_tempword); }, //uses next value as 8bit pointer then adds y to that.
		[this] {_tempword = _programCounter + ((signed char)(GetNextByte())); }, 
		[this] {_tempword = GetNextWord(); _tempbyte = ReadByte(_tempword); },
		[this] {_tempword = GetNextWord() + _indexX; _tempbyte = ReadByte(_tempword); },
		[this] {_tempword = GetNextWord() + _indexY; _tempbyte = ReadByte(_tempword); },
		[this] {_tempword = (uint16_t)ReadByte(GetNextWord()); },
		[this] {uint8_t nbyte = GetNextByte(); _tempword = ReadByte((nbyte + _indexX) % 256) + ReadByte((nbyte + _indexX + 1) % 256) << 8;  _tempbyte = ReadByte(ReadByte((nbyte + _indexX) % 256) + ReadByte((nbyte + _indexX + 1) % 256) << 8); },
		[this] {uint8_t nbyte = GetNextByte(); _tempword = ReadByte(nbyte) + ReadByte(nbyte + 1 % 256) * 256 + _indexY; _tempbyte = ReadByte(_tempword); }
		};


		//instruction matrix 8 x 32 matrix which stores all the instructions
		Instruction instructionMatrix[] =
		{
			Brk, Ora, Jam, Jam, Nop, Ora, Asl, Jam,		Php, Ora, Asl, Jam, Nop, Ora, Asl, Jam,		Bpl, Ora, Jam, Jam, Nop, Ora, Asl, Jam,		Clc, Ora, Nop, Jam, Nop, Ora, Asl, Jam,
			Jsr, And, Jam, Jam, Bit, And, Rol, Jam,		Plp, And, Rol, Jam, Bit, And, Rol, Jam,		Bmi, And, Jam, Jam, Nop, And, Rol, Jam,		Sec, And, Nop, Jam, Nop, And, Rol, Jam,
			Rti, Eor, Jam, Jam, Nop, Eor, Lsr, Jam,		Pha, Eor, Lsr, Jam, Jmp, Eor, Lsr, Jam,		Bvc, Eor, Jam, Jam, Nop, Eor, Lsr, Jam,		Cli, Eor, Nop, Jam, Nop, Eor, Lsr, Jam,
			Rts, Adc, Jam, Jam, Nop, Adc, Ror, Jam,		Pla, Adc, Ror, Jam, Jmp, Adc, Ror, Jam,		Bvs, Adc, Jam, Jam, Nop, Adc, Ror, Jam,		Sei, Adc, Nop, Jam, Nop, Adc, Ror, Jam, 
			Nop, Sta, Jam, Jam, Sty, Sta, Stx, Jam,		Dey, Nop, Txa, Jam, Sty, Sta, Stx, Jam,		Bcc, Sta, Jam, Jam, Sty, Sta, Stx, Jam,		Tya, Sta, Txs, Jam, Jam, Sta, Jam, Jam,
			Ldy, Lda, Ldx, Jam, Ldy, Lda, Ldx, Jam,		Tay, Lda, Tax, Jam, Ldy, Lda, Ldx, Jam,		Bcs, Lda, Jam, Jam, Ldy, Lda, Ldx, Jam,		Clv, Lda, Tsx, Jam, Ldy, Lda, Ldx, Jam,
			Cpy, Cmp, Jam, Jam, Cpy, Cmp, Dec, Jam,		Iny, Cmp, Dex, Jam, Cpy, Cmp, Dec, Jam,		Bne, Cmp, Jam, Jam, Nop, Cmp, Dec, Jam,		Cld, Cmp, Nop, Jam, Nop, Cmp, Dec, Jam,
			Cpx, Sbc, Jam, Jam, Cpx, Sbc, Inc, Jam,		Inx, Sbc, Nop, Sbc, Cpx, Sbc, Inc, Jam,		Beq, Sbc, Jam, Jam, Nop, Sbc, Inc, Jam,		Sed, Sbc, Nop, Jam, Nop, Sbc, Inc, Jam
		};
		
		OPMODE OpMatrix[32 * 8];


		OPMODE rowmodes[32] =
		{
			IMPLICIT,
			INDEXED_INDIRECT,
			IMMEDIATE,
			INDEXED_INDIRECT,
			ZEROPAGE,
			ZEROPAGE,
			ZEROPAGE,
			ZEROPAGE,

			IMPLICIT,
			IMMEDIATE,
			ACCUMULATOR,
			IMMEDIATE,
			ABSOLUTE,
			ABSOLUTE,
			ABSOLUTE,
			ABSOLUTE,

			RELATIVE,
			INDIRECT_INDEXED,
			IMPLICIT,
			INDIRECT_INDEXED,
			ZEROPAGE_X,
			ZEROPAGE_X,
			ZEROPAGE_X,
			ZEROPAGE_X,

			IMPLICIT,
			ABSOLUTE_Y,
			IMPLICIT,
			ABSOLUTE_Y,
			ABSOLUTE_X,
			ABSOLUTE_X,
			ABSOLUTE_X,
			ABSOLUTE_X
		};

		struct SpecialCase {
			int address;
			OPMODE opmode;
		};

		std::vector<SpecialCase> SpecialCases =
		{
			{0x20, IMMEDIATE},
			{0xa0, IMMEDIATE},
			{0xc0, IMMEDIATE},
			{0xe0, IMMEDIATE},

			{0x80 + 0x0a, IMPLICIT},
			{0xa0 + 0x0a, IMPLICIT},
			{0xc0 + 0x0a, IMPLICIT},
			{0xe0 + 0x0a, IMPLICIT},

			{0x60 + 0x0c, INDIRECT},
			{0x80 + 0x16, ZEROPAGE_Y},
			{0x80 + 0x17, ZEROPAGE_Y},
			{0xa0 + 0x16, ZEROPAGE_Y},
			{0xa0 + 0x17, ZEROPAGE_Y},
			{0x80 + 0x1e, ABSOLUTE_Y},
			{0xa0 + 0x1e, ABSOLUTE_Y},
			{0x80 + 0x1f, ABSOLUTE_Y},
			{0xa0 + 0x1f, ABSOLUTE_Y},
		};

		//initialize the op matrix
		for (int i = 0; i < 32; i++) {
			for (int x = 0; x < 8; x++) {
				OpMatrix[32 * x + i] = rowmodes[i];
			}
		}
		//then set the special cases

		for (int i = 0; i < SpecialCases.size(); i++) {
			OpMatrix[SpecialCases[i].address] = SpecialCases[i].opmode;
		}


	

		while (1) {
			uint8_t current = GetNextByte();
			_currentmode = OpMatrix[current];
			mode_lambdas[OpMatrix[current]]();
			_funcs[instructionMatrix[current]]();
			//std::cout <<"PC:" << _programCounter << " Accumulator:" << _accumulator << " IndexX : " << _indexX << " IndexY : " << _indexY << "\n";
		}
	}
	void EMU_6502::StartCPU(Bus* bus)
	{
		_bus = bus;
		std::thread CpuThread(&EMU_6502::Execute, this);
		CpuThread.join();
	}
	void EMU_6502::JAM()
	{
		std::cout << "6502 got jammed:(\n";
		exit(1);
	}
	void EMU_6502::ADC()
	{
		uint8_t add = _accumulator + _tempbyte;
		if (0xff - _accumulator < _tempbyte) {
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
	}

	void EMU_6502::AND()
	{
		_accumulator &= _tempbyte;
		OnSetFlag(_accumulator);
	}

	void EMU_6502::ASL()
	{
		_carry = 0;
		if (_tempbyte > 127) {
			_carry = 1;
		}

		_tempbyte <<= 1;
		WriteDataMode(_tempbyte);
		OnSetFlag(_tempbyte);
	}

	void EMU_6502::BCC()
	{
		if (!_carry) {
			_programCounter = _tempword;
		}
	}

	void EMU_6502::BCS()
	{
		if (_carry) {
			_programCounter = _tempword;
		}
	}

	void EMU_6502::BEQ()
	{
		if (_zero) {
			_programCounter = _tempword;
		}
	}

	void EMU_6502::BIT()
	{
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
			_programCounter = _tempword;
		}
	}

	void EMU_6502::BNE()
	{
		if (!_zero) {
			_programCounter = _tempword;
		}
	}

	void EMU_6502::BPL()
	{
		if (!_negative) {
			_programCounter = _tempword;
		}
	}

	void EMU_6502::BRK()
	{
		PushWord(_programCounter);
		PHP();
		_programCounter = (uint16_t)ReadByte(0xfffe);
		_breakcommand = 1;
	}

	void EMU_6502::BVC()
	{
		if (!_overflow) {
			_programCounter = _tempword;
		}
	}

	void EMU_6502::BVS()
	{
		if (_overflow) {
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
		uint8_t cmpresult = _accumulator - _tempbyte;

		_carry = _accumulator >= _tempbyte;
		OnSetFlag(cmpresult);
	}

	void EMU_6502::CPX()
	{
		uint8_t cmpresult = _indexX - _tempbyte;

		_carry = _indexX >= _tempbyte;
		OnSetFlag(cmpresult);
	}

	void EMU_6502::CPY()
	{
		uint8_t cmpresult = _indexY - _tempbyte;

		_carry = _indexY >= _tempbyte;
		OnSetFlag(cmpresult);
	}

	void EMU_6502::DEC()
	{
		_tempbyte -= 1;
		WriteByte(_tempword, _tempbyte);
		OnSetFlag(_tempbyte);
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
		_accumulator ^= _tempbyte;
		OnSetFlag(_accumulator);
	}

	void EMU_6502::INC()
	{
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
		PushByte(_programCounter);
		_programCounter = _tempword;
	}

	void EMU_6502::LDA()
	{
		_accumulator = _tempbyte;
		OnSetFlag(_accumulator);
	}

	void EMU_6502::LDX()
	{
		_indexX = _tempbyte;
		OnSetFlag(_indexX);
	}

	void EMU_6502::LDY()
	{
		_indexY = _tempbyte;
		OnSetFlag(_indexY);
	}

	void EMU_6502::LSR()
	{
		_carry = 0;
		if (_tempbyte & 0b1) {
			_carry = 1;
		}

		_tempbyte >>= 1;
		WriteDataMode(_tempbyte);
		OnSetFlag(_tempbyte);
	}

	void EMU_6502::NOP()
	{
	}

	void EMU_6502::ORA()
	{
		_accumulator |= _tempbyte;
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
		_carry = 0;
		if (_tempbyte > 127) {
			_carry = 1;
		}

		_tempbyte <<= 1;
		_tempbyte |= prevcarry;
		WriteDataMode(_tempbyte);
		OnSetFlag(_tempbyte);
	}

	void EMU_6502::ROR()
	{
		bool prevcarry = _carry;
		_carry = 0;
		if (_tempbyte & 0b1) {
			_carry = 1;
		}

		_tempbyte >>= 1;
		_tempbyte |= prevcarry << 7;
		WriteDataMode(_tempbyte);
		OnSetFlag(_tempbyte);
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
		uint8_t sub = _accumulator - _tempbyte - ~_carry;
		if (0xff - _accumulator < _tempbyte) {
			//overflow occured
			_carry = 0;
		}
		_overflow = 0;
		if (_accumulator < 128 && _tempbyte < 128 && sub > 127) {
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

}
