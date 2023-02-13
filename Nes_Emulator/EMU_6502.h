#pragma once
#include <iostream>
#include <functional>
#include "Bus.h"
#include <vector>
#include <chrono>

#include <thread>
namespace Hardware {

	class EMU_6502
	{
	public:
		//number of hz per second
		int clock_speed = 1789773;
		uint64_t _total_cycles;
		void Execute(bool onecycle = false);
		EMU_6502(Bus* bus);
	private:
		//registers

		//general purpose register for math etc
		uint8_t _accumulator;
		//can be used for indexing, y and i can do the same things
		uint8_t _indexX;
		uint8_t _indexY;
		//The stack pointer is = $0100+stack
		uint8_t _stackPointer;
		//where program execution is
		uint16_t _programCounter;

		//all of these are part of the Processor status register. they are packed when needed
		bool _carry;
		bool _zero;
		bool _interrupt;
		bool _decimalmode;
		bool _breakcommand;
		bool _nothingflag;
		bool _overflow;
		bool _negative;

		uint8_t PackprocessorStatus() {
			uint8_t output = _carry;
			output += _zero << 1;
			output += _interrupt << 2;
			output += _decimalmode << 3;
			output += _breakcommand << 4;
			output += _nothingflag << 5;
			output += _overflow << 6;
			output += _negative << 7;
			return output;
		}
		void SetUnpackedFlags(uint8_t processorstatus) {
			_carry = processorstatus & 0b1;
			processorstatus >>= 1;
			_zero = (processorstatus & 0b1);
			processorstatus >>= 1;
			_interrupt = (processorstatus & 0b1);
			processorstatus >>= 1;
			_decimalmode = (processorstatus & 0b1);
			processorstatus >>= 1;
			_breakcommand = (processorstatus & 0b1);
			processorstatus >>= 1;
			_nothingflag = (processorstatus & 0b1);
			processorstatus >>= 1;
			_overflow = (processorstatus & 0b1);
			processorstatus >>= 1;
			_negative = processorstatus;
		}





		//used for addressing modes. so instructions dont handle getting value
		uint8_t opcode;
		uint8_t _tempbyte;
		uint16_t _tempword;
		Bus* _bus;
		int _currentmode;

		enum OPMODE {
			IMPLICIT, ACCUMULATOR, IMMEDIATE, ZEROPAGE, ZEROPAGE_X,
			ZEROPAGE_Y, RELATIVE, ABSOLUTE, ABSOLUTE_X, ABSOLUTE_Y,
			INDIRECT, INDEXED_INDIRECT, INDIRECT_INDEXED
		};




		void WriteDataMode(uint8_t data) {
			if (_currentmode == ACCUMULATOR) {
				_accumulator = data;
			}
			else {
				_bus->WriteByte(_tempword, data);
			}
		}


		//Instructions

		/*6510 Instructions by Addressing Modes

		off- ++++++++++ Positive ++++++++++  ---------- Negative ----------
		set  00      20      40      60      80      a0      c0      e0      mode

		+00  BRK     JSR     RTI     RTS     NOP*    LDY     CPY     CPX     Impl/immed
		+01  ORA     AND     EOR     ADC     STA     LDA     CMP     SBC     (indir,x)
		+02   t       t       t       t      NOP*t   LDX     NOP*t   NOP*t     ? /immed
		+03  SLO*    RLA*    SRE*    RRA*    SAX*    LAX*    DCP*    ISB*    (indir,x)
		+04  NOP*    BIT     NOP*    NOP*    STY     LDY     CPY     CPX     Zeropage
		+05  ORA     AND     EOR     ADC     STA     LDA     CMP     SBC     Zeropage
		+06  ASL     ROL     LSR     ROR     STX     LDX     DEC     INC     Zeropage
		+07  SLO*    RLA*    SRE*    RRA*    SAX*    LAX*    DCP*    ISB*    Zeropage

		+08  PHP     PLP     PHA     PLA     DEY     TAY     INY     INX     Implied
		+09  ORA     AND     EOR     ADC     NOP*    LDA     CMP     SBC     Immediate
		+0a  ASL     ROL     LSR     ROR     TXA     TAX     DEX     NOP     Accu/impl
		+0b  ANC**   ANC**   ASR**   ARR**   ANE**   LXA**   SBX**   SBC*    Immediate
		+0c  NOP*    BIT     JMP     JMP ()  STY     LDY     CPY     CPX     Absolute
		+0d  ORA     AND     EOR     ADC     STA     LDA     CMP     SBC     Absolute
		+0e  ASL     ROL     LSR     ROR     STX     LDX     DEC     INC     Absolute
		+0f  SLO*    RLA*    SRE*    RRA*    SAX*    LAX*    DCP*    ISB*    Absolute

		+10  BPL     BMI     BVC     BVS     BCC     BCS     BNE     BEQ     Relative
		+11  ORA     AND     EOR     ADC     STA     LDA     CMP     SBC     (indir),y
		+12   t       t       t       t       t       t       t       t         ?
		+13  SLO*    RLA*    SRE*    RRA*    SHA**   LAX*    DCP*    ISB*    (indir),y
		+14  NOP*    NOP*    NOP*    NOP*    STY     LDY     NOP*    NOP*    Zeropage,x
		+15  ORA     AND     EOR     ADC     STA     LDA     CMP     SBC     Zeropage,x
		+16  ASL     ROL     LSR     ROR     STX  y) LDX  y) DEC     INC     Zeropage,x
		+17  SLO*    RLA*    SRE*    RRA*    SAX* y) LAX* y) DCP*    ISB*    Zeropage,x

		+18  CLC     SEC     CLI     SEI     TYA     CLV     CLD     SED     Implied
		+19  ORA     AND     EOR     ADC     STA     LDA     CMP     SBC     Absolute,y
		+1a  NOP*    NOP*    NOP*    NOP*    TXS     TSX     NOP*    NOP*    Implied
		+1b  SLO*    RLA*    SRE*    RRA*    SHS**   LAS**   DCP*    ISB*    Absolute,y
		+1c  NOP*    NOP*    NOP*    NOP*    SHY**   LDY     NOP*    NOP*    Absolute,x
		+1d  ORA     AND     EOR     ADC     STA     LDA     CMP     SBC     Absolute,x
		+1e  ASL     ROL     LSR     ROR     SHX**y) LDX  y) DEC     INC     Absolute,x
		+1f  SLO*    RLA*    SRE*    RRA*    SHA**y) LAX* y) DCP*    ISB*    Absolute,x

		ROR intruction is available on MC650x microprocessors after
		June, 1976.

		Legend:

		t       Jams the machine
		*t      Jams very rarely
		*       Undocumented command
		**      Unusual operation
		y)      indexed using Y instead of X
		()      indirect instead of absolute*/

		//to get instruction take mode and add it to offset. so JSR is mode 0x20 + offset of 0

			//helper functions

			//set the zero if its zero and negative if its negative
		void OnSetFlag(uint8_t value) {
			if (value == 0) {
				_zero = 1;
			}
			else {
				_zero = 0;
			}
			if (value >> 7 == 1) {
				_negative = 1;
			}
			else {
				_negative = 0;
			}
		}

		uint8_t ReadByte(uint16_t address) {
			return _bus->ReadByte(address);
		}
		void WriteByte(uint16_t address, uint8_t data) {
			_bus->WriteByte(address, data);
		}
		uint8_t GetNextByte() {
			uint8_t rtn = ReadByte(_programCounter);
			_programCounter++;
			return rtn;
		}
		uint16_t GetNextWord() {
			uint16_t rtn = ReadByte(_programCounter);
			_programCounter++;
			rtn += ReadByte(_programCounter) << 8;
			_programCounter++;
			return rtn;
		}
		void PushByte(uint8_t value) {

			WriteByte(0x0100 + _stackPointer, value);
			_stackPointer -= 1;
		}
		void PushWord(uint16_t value) {

			WriteByte(0x0100 + _stackPointer, value);
			_stackPointer -= 1;
			WriteByte(0x0100 + _stackPointer, value >> 8);
			_stackPointer -= 1;
		}
		uint8_t PopByte() {
			_stackPointer += 1;
			return ReadByte(0x0100 + _stackPointer);
		}
		uint16_t PopWord() {
			uint16_t rtn = PopByte() << 8;
			rtn += PopByte();
			return rtn;
		}


		void IsImmed() {
			if (_currentmode > 2) {
				_tempbyte = ReadByte(_tempword);
			}
		}
		void IRQ();
		void NMI();
		public:
		void Reset();
		private:
		//instructions
		void JAM();
		void ADC(); void AND(); void ASL(); void BCC(); void BCS(); void BEQ(); void BIT(); void BMI();
		void BNE(); void BPL(); void BRK(); void BVC(); void BVS();	void CLC(); void CLD();	void CLI();
		void CLV();	void CMP();	void CPX(); void CPY(); void DEC();	void DEX(); void DEY(); void EOR();
		void INC();	void INX(); void INY();	void JMP(); void JSR(); void LDA(); void LDX(); void LDY();
		void LSR();	void NOP();	void ORA();	void PHA(); void PHP();	void PLA(); void PLP();	void ROL();
		void ROR(); void RTI(); void RTS(); void SBC(); void SEC(); void SED(); void SEI(); void STA();
		void STX(); void STY(); void TAX(); void TAY(); void TSX(); void TXA(); void TXS(); void TYA();

		/*IMPLICIT, ACCUMULATOR, IMMEDIATE, ZEROPAGE, ZEROPAGE_X,
			ZEROPAGE_Y, RELATIVE, ABSOLUTE, ABSOLUTE_X, ABSOLUTE_Y,
			INDIRECT, INDEXED_INDIRECT, INDIRECT_INDEXED*/



		void IMP(); void ACU(); void IMM(); void ZP0(); void ZPX(); void ZPY();
		void REL(); void ABS(); void ABX(); void ABY(); void IND(); void IID();
		void IDI();

		int _additional_cycles;
		struct Instruction {
			std::string name;
			void (EMU_6502::* operate)(void) = nullptr;
			void (EMU_6502::* addrmode)(void) = nullptr;
			uint8_t cycles;
		};

		Instruction InstructionMatrix[256] = { { "BRK", &EMU_6502::BRK, &EMU_6502::IMM, 7 }, { "ORA", &EMU_6502::ORA, &EMU_6502::IID, 6 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 2 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 8 }, { "???", &EMU_6502::NOP, &EMU_6502::IMP, 3 }, { "ORA", &EMU_6502::ORA, &EMU_6502::ZP0, 3 }, { "ASL", &EMU_6502::ASL, &EMU_6502::ZP0, 5 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 5 }, { "PHP", &EMU_6502::PHP, &EMU_6502::IMP, 3 }, { "ORA", &EMU_6502::ORA, &EMU_6502::IMM, 2 }, { "ASL", &EMU_6502::ASL, &EMU_6502::IMP, 2 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 2 }, { "???", &EMU_6502::NOP, &EMU_6502::IMP, 4 }, { "ORA", &EMU_6502::ORA, &EMU_6502::ABS, 4 }, { "ASL", &EMU_6502::ASL, &EMU_6502::ABS, 6 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 6 },
		{ "BPL", &EMU_6502::BPL, &EMU_6502::REL, 2 }, { "ORA", &EMU_6502::ORA, &EMU_6502::IDI, 5 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 2 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 8 }, { "???", &EMU_6502::NOP, &EMU_6502::IMP, 4 }, { "ORA", &EMU_6502::ORA, &EMU_6502::ZPX, 4 }, { "ASL", &EMU_6502::ASL, &EMU_6502::ZPX, 6 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 6 }, { "CLC", &EMU_6502::CLC, &EMU_6502::IMP, 2 }, { "ORA", &EMU_6502::ORA, &EMU_6502::ABY, 4 }, { "???", &EMU_6502::NOP, &EMU_6502::IMP, 2 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 7 }, { "???", &EMU_6502::NOP, &EMU_6502::IMP, 4 }, { "ORA", &EMU_6502::ORA, &EMU_6502::ABX, 4 }, { "ASL", &EMU_6502::ASL, &EMU_6502::ABX, 7 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 7 },
		{ "JSR", &EMU_6502::JSR, &EMU_6502::ABS, 6 }, { "AND", &EMU_6502::AND, &EMU_6502::IID, 6 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 2 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 8 }, { "BIT", &EMU_6502::BIT, &EMU_6502::ZP0, 3 }, { "AND", &EMU_6502::AND, &EMU_6502::ZP0, 3 }, { "ROL", &EMU_6502::ROL, &EMU_6502::ZP0, 5 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 5 }, { "PLP", &EMU_6502::PLP, &EMU_6502::IMP, 4 }, { "AND", &EMU_6502::AND, &EMU_6502::IMM, 2 }, { "ROL", &EMU_6502::ROL, &EMU_6502::IMP, 2 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 2 }, { "BIT", &EMU_6502::BIT, &EMU_6502::ABS, 4 }, { "AND", &EMU_6502::AND, &EMU_6502::ABS, 4 }, { "ROL", &EMU_6502::ROL, &EMU_6502::ABS, 6 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 6 },
		{ "BMI", &EMU_6502::BMI, &EMU_6502::REL, 2 }, { "AND", &EMU_6502::AND, &EMU_6502::IDI, 5 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 2 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 8 }, { "???", &EMU_6502::NOP, &EMU_6502::IMP, 4 }, { "AND", &EMU_6502::AND, &EMU_6502::ZPX, 4 }, { "ROL", &EMU_6502::ROL, &EMU_6502::ZPX, 6 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 6 }, { "SEC", &EMU_6502::SEC, &EMU_6502::IMP, 2 }, { "AND", &EMU_6502::AND, &EMU_6502::ABY, 4 }, { "???", &EMU_6502::NOP, &EMU_6502::IMP, 2 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 7 }, { "???", &EMU_6502::NOP, &EMU_6502::IMP, 4 }, { "AND", &EMU_6502::AND, &EMU_6502::ABX, 4 }, { "ROL", &EMU_6502::ROL, &EMU_6502::ABX, 7 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 7 },
		{ "RTI", &EMU_6502::RTI, &EMU_6502::IMP, 6 }, { "EOR", &EMU_6502::EOR, &EMU_6502::IID, 6 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 2 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 8 }, { "???", &EMU_6502::NOP, &EMU_6502::IMP, 3 }, { "EOR", &EMU_6502::EOR, &EMU_6502::ZP0, 3 }, { "LSR", &EMU_6502::LSR, &EMU_6502::ZP0, 5 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 5 }, { "PHA", &EMU_6502::PHA, &EMU_6502::IMP, 3 }, { "EOR", &EMU_6502::EOR, &EMU_6502::IMM, 2 }, { "LSR", &EMU_6502::LSR, &EMU_6502::IMP, 2 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 2 }, { "JMP", &EMU_6502::JMP, &EMU_6502::ABS, 3 }, { "EOR", &EMU_6502::EOR, &EMU_6502::ABS, 4 }, { "LSR", &EMU_6502::LSR, &EMU_6502::ABS, 6 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 6 },
		{ "BVC", &EMU_6502::BVC, &EMU_6502::REL, 2 }, { "EOR", &EMU_6502::EOR, &EMU_6502::IDI, 5 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 2 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 8 }, { "???", &EMU_6502::NOP, &EMU_6502::IMP, 4 }, { "EOR", &EMU_6502::EOR, &EMU_6502::ZPX, 4 }, { "LSR", &EMU_6502::LSR, &EMU_6502::ZPX, 6 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 6 }, { "CLI", &EMU_6502::CLI, &EMU_6502::IMP, 2 }, { "EOR", &EMU_6502::EOR, &EMU_6502::ABY, 4 }, { "???", &EMU_6502::NOP, &EMU_6502::IMP, 2 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 7 }, { "???", &EMU_6502::NOP, &EMU_6502::IMP, 4 }, { "EOR", &EMU_6502::EOR, &EMU_6502::ABX, 4 }, { "LSR", &EMU_6502::LSR, &EMU_6502::ABX, 7 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 7 },
		{ "RTS", &EMU_6502::RTS, &EMU_6502::IMP, 6 }, { "ADC", &EMU_6502::ADC, &EMU_6502::IID, 6 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 2 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 8 }, { "???", &EMU_6502::NOP, &EMU_6502::IMP, 3 }, { "ADC", &EMU_6502::ADC, &EMU_6502::ZP0, 3 }, { "ROR", &EMU_6502::ROR, &EMU_6502::ZP0, 5 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 5 }, { "PLA", &EMU_6502::PLA, &EMU_6502::IMP, 4 }, { "ADC", &EMU_6502::ADC, &EMU_6502::IMM, 2 }, { "ROR", &EMU_6502::ROR, &EMU_6502::IMP, 2 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 2 }, { "JMP", &EMU_6502::JMP, &EMU_6502::IND, 5 }, { "ADC", &EMU_6502::ADC, &EMU_6502::ABS, 4 }, { "ROR", &EMU_6502::ROR, &EMU_6502::ABS, 6 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 6 },
		{ "BVS", &EMU_6502::BVS, &EMU_6502::REL, 2 }, { "ADC", &EMU_6502::ADC, &EMU_6502::IDI, 5 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 2 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 8 }, { "???", &EMU_6502::NOP, &EMU_6502::IMP, 4 }, { "ADC", &EMU_6502::ADC, &EMU_6502::ZPX, 4 }, { "ROR", &EMU_6502::ROR, &EMU_6502::ZPX, 6 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 6 }, { "SEI", &EMU_6502::SEI, &EMU_6502::IMP, 2 }, { "ADC", &EMU_6502::ADC, &EMU_6502::ABY, 4 }, { "???", &EMU_6502::NOP, &EMU_6502::IMP, 2 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 7 }, { "???", &EMU_6502::NOP, &EMU_6502::IMP, 4 }, { "ADC", &EMU_6502::ADC, &EMU_6502::ABX, 4 }, { "ROR", &EMU_6502::ROR, &EMU_6502::ABX, 7 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 7 },
		{ "???", &EMU_6502::NOP, &EMU_6502::IMP, 2 }, { "STA", &EMU_6502::STA, &EMU_6502::IID, 6 }, { "???", &EMU_6502::NOP, &EMU_6502::IMP, 2 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 6 }, { "STY", &EMU_6502::STY, &EMU_6502::ZP0, 3 }, { "STA", &EMU_6502::STA, &EMU_6502::ZP0, 3 }, { "STX", &EMU_6502::STX, &EMU_6502::ZP0, 3 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 3 }, { "DEY", &EMU_6502::DEY, &EMU_6502::IMP, 2 }, { "???", &EMU_6502::NOP, &EMU_6502::IMP, 2 }, { "TXA", &EMU_6502::TXA, &EMU_6502::IMP, 2 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 2 }, { "STY", &EMU_6502::STY, &EMU_6502::ABS, 4 }, { "STA", &EMU_6502::STA, &EMU_6502::ABS, 4 }, { "STX", &EMU_6502::STX, &EMU_6502::ABS, 4 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 4 },
		{ "BCC", &EMU_6502::BCC, &EMU_6502::REL, 2 }, { "STA", &EMU_6502::STA, &EMU_6502::IDI, 6 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 2 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 6 }, { "STY", &EMU_6502::STY, &EMU_6502::ZPX, 4 }, { "STA", &EMU_6502::STA, &EMU_6502::ZPX, 4 }, { "STX", &EMU_6502::STX, &EMU_6502::ZPY, 4 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 4 }, { "TYA", &EMU_6502::TYA, &EMU_6502::IMP, 2 }, { "STA", &EMU_6502::STA, &EMU_6502::ABY, 5 }, { "TXS", &EMU_6502::TXS, &EMU_6502::IMP, 2 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 5 }, { "???", &EMU_6502::NOP, &EMU_6502::IMP, 5 }, { "STA", &EMU_6502::STA, &EMU_6502::ABX, 5 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 5 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 5 },
		{ "LDY", &EMU_6502::LDY, &EMU_6502::IMM, 2 }, { "LDA", &EMU_6502::LDA, &EMU_6502::IID, 6 }, { "LDX", &EMU_6502::LDX, &EMU_6502::IMM, 2 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 6 }, { "LDY", &EMU_6502::LDY, &EMU_6502::ZP0, 3 }, { "LDA", &EMU_6502::LDA, &EMU_6502::ZP0, 3 }, { "LDX", &EMU_6502::LDX, &EMU_6502::ZP0, 3 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 3 }, { "TAY", &EMU_6502::TAY, &EMU_6502::IMP, 2 }, { "LDA", &EMU_6502::LDA, &EMU_6502::IMM, 2 }, { "TAX", &EMU_6502::TAX, &EMU_6502::IMP, 2 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 2 }, { "LDY", &EMU_6502::LDY, &EMU_6502::ABS, 4 }, { "LDA", &EMU_6502::LDA, &EMU_6502::ABS, 4 }, { "LDX", &EMU_6502::LDX, &EMU_6502::ABS, 4 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 4 },
		{ "BCS", &EMU_6502::BCS, &EMU_6502::REL, 2 }, { "LDA", &EMU_6502::LDA, &EMU_6502::IDI, 5 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 2 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 5 }, { "LDY", &EMU_6502::LDY, &EMU_6502::ZPX, 4 }, { "LDA", &EMU_6502::LDA, &EMU_6502::ZPX, 4 }, { "LDX", &EMU_6502::LDX, &EMU_6502::ZPY, 4 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 4 }, { "CLV", &EMU_6502::CLV, &EMU_6502::IMP, 2 }, { "LDA", &EMU_6502::LDA, &EMU_6502::ABY, 4 }, { "TSX", &EMU_6502::TSX, &EMU_6502::IMP, 2 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 4 }, { "LDY", &EMU_6502::LDY, &EMU_6502::ABX, 4 }, { "LDA", &EMU_6502::LDA, &EMU_6502::ABX, 4 }, { "LDX", &EMU_6502::LDX, &EMU_6502::ABY, 4 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 4 },
		{ "CPY", &EMU_6502::CPY, &EMU_6502::IMM, 2 }, { "CMP", &EMU_6502::CMP, &EMU_6502::IID, 6 }, { "???", &EMU_6502::NOP, &EMU_6502::IMP, 2 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 8 }, { "CPY", &EMU_6502::CPY, &EMU_6502::ZP0, 3 }, { "CMP", &EMU_6502::CMP, &EMU_6502::ZP0, 3 }, { "DEC", &EMU_6502::DEC, &EMU_6502::ZP0, 5 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 5 }, { "INY", &EMU_6502::INY, &EMU_6502::IMP, 2 }, { "CMP", &EMU_6502::CMP, &EMU_6502::IMM, 2 }, { "DEX", &EMU_6502::DEX, &EMU_6502::IMP, 2 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 2 }, { "CPY", &EMU_6502::CPY, &EMU_6502::ABS, 4 }, { "CMP", &EMU_6502::CMP, &EMU_6502::ABS, 4 }, { "DEC", &EMU_6502::DEC, &EMU_6502::ABS, 6 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 6 },
		{ "BNE", &EMU_6502::BNE, &EMU_6502::REL, 2 }, { "CMP", &EMU_6502::CMP, &EMU_6502::IDI, 5 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 2 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 8 }, { "???", &EMU_6502::NOP, &EMU_6502::IMP, 4 }, { "CMP", &EMU_6502::CMP, &EMU_6502::ZPX, 4 }, { "DEC", &EMU_6502::DEC, &EMU_6502::ZPX, 6 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 6 }, { "CLD", &EMU_6502::CLD, &EMU_6502::IMP, 2 }, { "CMP", &EMU_6502::CMP, &EMU_6502::ABY, 4 }, { "NOP", &EMU_6502::NOP, &EMU_6502::IMP, 2 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 7 }, { "???", &EMU_6502::NOP, &EMU_6502::IMP, 4 }, { "CMP", &EMU_6502::CMP, &EMU_6502::ABX, 4 }, { "DEC", &EMU_6502::DEC, &EMU_6502::ABX, 7 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 7 },
		{ "CPX", &EMU_6502::CPX, &EMU_6502::IMM, 2 }, { "SBC", &EMU_6502::SBC, &EMU_6502::IID, 6 }, { "???", &EMU_6502::NOP, &EMU_6502::IMP, 2 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 8 }, { "CPX", &EMU_6502::CPX, &EMU_6502::ZP0, 3 }, { "SBC", &EMU_6502::SBC, &EMU_6502::ZP0, 3 }, { "INC", &EMU_6502::INC, &EMU_6502::ZP0, 5 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 5 }, { "INX", &EMU_6502::INX, &EMU_6502::IMP, 2 }, { "SBC", &EMU_6502::SBC, &EMU_6502::IMM, 2 }, { "NOP", &EMU_6502::NOP, &EMU_6502::IMP, 2 }, { "???", &EMU_6502::SBC, &EMU_6502::IMP, 2 }, { "CPX", &EMU_6502::CPX, &EMU_6502::ABS, 4 }, { "SBC", &EMU_6502::SBC, &EMU_6502::ABS, 4 }, { "INC", &EMU_6502::INC, &EMU_6502::ABS, 6 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 6 },
		{ "BEQ", &EMU_6502::BEQ, &EMU_6502::REL, 2 }, { "SBC", &EMU_6502::SBC, &EMU_6502::IDI, 5 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 2 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 8 }, { "???", &EMU_6502::NOP, &EMU_6502::IMP, 4 }, { "SBC", &EMU_6502::SBC, &EMU_6502::ZPX, 4 }, { "INC", &EMU_6502::INC, &EMU_6502::ZPX, 6 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 6 }, { "SED", &EMU_6502::SED, &EMU_6502::IMP, 2 }, { "SBC", &EMU_6502::SBC, &EMU_6502::ABY, 4 }, { "NOP", &EMU_6502::NOP, &EMU_6502::IMP, 2 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 7 }, { "???", &EMU_6502::NOP, &EMU_6502::IMP, 4 }, { "SBC", &EMU_6502::SBC, &EMU_6502::ABX, 4 }, { "INC", &EMU_6502::INC, &EMU_6502::ABX, 7 }, { "???", &EMU_6502::JAM, &EMU_6502::IMP, 7 },
		};
	};
}

