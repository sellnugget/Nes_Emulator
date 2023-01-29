#pragma once
#include <iostream>
#include <functional>
namespace Hardware {

	class EMU_6502
	{

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

		enum OPMODE {
			IMPLICIT, ACCUMULATOR, IMMEDIATE, ZEROPAGE, ZEROPAGE_X,
			ZEROPAGE_Y, RELATIVE, ABSOLUTE, ABSOLUTE_X, ABSOLUTE_Y,
			INDIRECT, INDEXED_INDIRECT, INDIRECT_INDEXED
		};


		
		//used for addressing modes. so instructions dont handle getting value
		uint8_t* _tempbyte;
		uint16_t* _tempword;
		OPMODE _opmode;

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
			if (value > 127) {
				_negative = 1;
			}
			else {
				_negative = 0;
			}
		}
		void PushByte(uint8_t value) {
			
			_stackPointer -= 1;
		}
		void PushWord(uint16_t value) {
			_stackPointer -= 2;
		}
		uint8_t PopByte() {
			_stackPointer+=1;
			return 0;
		}
		uint16_t PopWord() {
			_stackPointer+=2;
			return 0;
		}
		uint16_t LoadWord(uint16_t address) {

		}
		//instructions
		void ADC();
		void AND();
		void ASL();
		void BCC();
		void BCS();
		void BEQ();
		void BIT();
		void BMI();
		void BNE();
		void BPL();
		void BRK();
		void BVC();
		void BVS();
		void CLC();
		void CLD();
		void CLI();
		void CLV();
		void CMP();
		void CPX();
		void CPY();
		void DEC();
		void DEX();
		void DEY();
		void EOR();
		void INC();
		void INX();
		void INY();
		void JMP();
		void JSR();
		void LDA();
		void LDX();
		void LDY();
		void LSR();
		void NOP();
		void ORA();
		void PHA();
		void PHP();
		void PLA();
		void PLP();
		void ROL();
		void ROR();
		void RTI();
		void RTS();
		void SBC();
		void SEC();
		void SED();
		void SEI();
		void STA();
		void STX();
		void STY();
		void TAX();
		void TAY();
		void TSX();
		void TXA();
		void TXS();
		void TYA();



		
	public:
		void Start6502();
	};
}


