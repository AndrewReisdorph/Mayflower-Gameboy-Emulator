#pragma once
#include <wx/wx.h>
#include "GBMMU.h"
#include "EmulatorEngine.h"
#include "GBTypes.h"

#include <iostream>
#include <fstream>
#include <sstream> 
#include <iomanip>
#include <fstream>
#include <string>


class GBMMU;


#define OAM_START_ADDR 0xFE00 // Sprite Attribute table 0xFE00-0xFE9F

// I/O Registers
#define P1_REGISTER    0xFF00 // Register for reading joy pad info and determining system type. (R / W)
#define SB_REGISTER    0xFF01 // Serial transfer data (R/W)
#define SC_REGISTER    0xFF02 // SIO control (R/W)
#define DIV_REGISTER   0xFF04 // Divider Register (R/W)
#define TIMA_REGISTER  0xFF05 // Timer counter (R/W)
#define TMA_REGISTER   0xFF06 // Timer Modulo (R/W)
#define TAC_REGISTER   0xFF07 // Timer Control (R/W)
#define IF_REGISTER    0xFF0F // Interrupt Flag (R/W)
#define NR10_REGISTER  0xFF10 // Sound Mode 1 register, Sweep register (R / W)
#define NR11_REGISTER  0xFF11 // Sound Mode 1 register, Sound length/Wave pattern duty(R / W)
#define NR52_REGISTER  0xFF26
// FF30 - FF3F Wave Pattern RAM
#define LCDC_REGISTER  0xFF40 // LCD Control (R/W)
#define STAT_REGISTER  0xFF41 // LCDC Status (R/W)
#define SCY_REGISTER   0xFF42 // Scroll Y (R/W)
#define SCX_REGISTER   0xFF43 // Scroll X (R/W)
#define LY_REGISTER    0xFF44 // LCDC Y-Coordinate (R)
#define LYC_REGISTER   0xFF45 // LY Compare (R/W)
#define DMA_REGISTER   0xFF46 // DMA Transfer and Start Address (W)
#define BGP_REGISTER   0xFF47 // BG & Window Palette Data (R/W)
#define OBP0_REGISTER  0xFF48 // Object Palette 0 Data (R/W)
#define OBP1_REGISTER  0xFF49 // Object Palette 1 Data (R/W)
#define DSROM_REGISTER 0xFF50 // Disable bootrom
#define WY_REGISTER    0xFF4A // Window Y Position (R/W)
#define WX_REGISTER    0xFF4B // Window X Position (R/W)
#define IE_REGISTER    0xFFFF // Interrupt Enable (R/W)


#define NUM_INTERRUPTS 5
// Interrupt flags
#define VBLANK_INTERRUPT   0x01
#define LCD_STAT_INTERRUPT 0x02
#define TIMER_INTERRUPT    0x04
#define SERIAL_INTERRUPT   0x08
#define JOYPAD_INTERRUPT   0x10

// Interrupt Routine Addresses
#define VBLANK_INT_ADDR   0x40
#define LCD_STAT_INT_ADDR 0x48
#define TIMER_INT_ADDR    0x50
#define SERIAL_INT_ADDR   0x58
#define JOYPAD_INT_ADDR   0x60



enum OpType
{
	OP_MISC_CONTROL = 0,
	OP_JUMP_CALL,
	OP_LD_STR_MV_8,
	OP_LD_STR_MV_16,
	OP_ALU_8,
	OP_ALU_16,
	OP_ROT_SHFT_BIT_8,
	OP_CB,
	NUM_OP_TYPES,
	OP_EMPTY
};

enum ArgType {
	ARG_MISSING,
	ARG_NONE,
	ARG_SIGNED_8,
	ARG_UNSIGNED_8,
	ARG_SIGNED_16,
	ARG_UNSIGNED_16
};

union OpArg
{
	signed char sint8;
	unsigned char uint8;
	unsigned short uint16;
};


typedef union flag_reg
{
	struct
	{
		byte reserved : 4;
		byte C : 1;
		byte H : 1;
		byte N : 1;
		byte Z : 1;
	};
	byte all;
}flag_reg;

typedef struct Instruction
{
	opcode OpCode;
	OpType Type;
	wxString Name;
	int NumCycles[2];
	ArgType OpArgType;
	OpArg Argument;
} Instruction;

typedef struct interrupt
{
	byte Flag;
	word Address;
} interrupt;




// CB Math Macros
#define M_CB_SRA(reg) {m_RegisterF.C = M_TestBit(reg, BIT_0);reg = (reg >> 1) | (reg & 0x80);m_RegisterF.Z = (reg == 0);m_RegisterF.N = 0;m_RegisterF.H = 0;}
#define M_CB_SLA(reg) {m_RegisterF.C = M_TestBit(reg, BIT_7);reg <<= 1;m_RegisterF.Z = (reg == 0);m_RegisterF.N = 0;m_RegisterF.H = 0;}
#define M_CB_RLC(reg) {if (reg & BIT_7){reg = (reg << 1) | 1;m_RegisterF.C = 1;}else{reg = (reg << 1);m_RegisterF.C = 0;}m_RegisterF.Z = (reg == 0);m_RegisterF.N = 0;m_RegisterF.H = 0; }
#define M_CB_RRC(reg) {if (reg & BIT_0){reg = (reg >> 1) | BIT_7;m_RegisterF.C = 1;}else{reg = (reg >> 1);m_RegisterF.C = 0;}m_RegisterF.Z = (reg == 0);m_RegisterF.N = 0;m_RegisterF.H = 0;}
#define M_CB_SRL(reg) {m_RegisterF.C = (reg & BIT_0)?1:0;reg >>= 1;m_RegisterF.Z = (reg == 0);m_RegisterF.N = 0;m_RegisterF.H = 0; }
#define M_CB_ADC(reg) {int tempC = m_RegisterF.C;m_RegisterF.C = (((m_RegisterA)+(reg + tempC)) > 0xFF);m_RegisterF.H = (((m_RegisterA & 0x0F) + (reg & 0x0F)+tempC) > 0x0F);m_RegisterA += reg + tempC;m_RegisterF.Z = (m_RegisterA == 0);m_RegisterF.N = 0;}
#define M_CB_TestBit(reg, bit) {m_RegisterF.Z = (reg & bit) ? 0 : 1;m_RegisterF.N = 0;m_RegisterF.H = 1;} 
#define M_CB_RotateLeft(reg) {if(reg & BIT_7){reg = (reg << 1) | m_RegisterF.C;m_RegisterF.C = 1;}else{reg = (reg << 1) | m_RegisterF.C;m_RegisterF.C = 0;}m_RegisterF.Z = (reg == 0);m_RegisterF.H = 0;m_RegisterF.N=0;}
#define M_CB_RotateRight(reg) {if (reg & BIT_0){reg = (reg >> 1) | (m_RegisterF.C ? BIT_7 : 0);m_RegisterF.C = 1;}else{reg = (reg >> 1) | (m_RegisterF.C ? BIT_7 : 0);m_RegisterF.C = 0;}m_RegisterF.Z = (reg == 0);m_RegisterF.N = 0;m_RegisterF.H = 0;}
#define M_CB_Swap(reg) 	{byte temp = reg & 0xF0;reg <<= 4;reg |= (temp>>4);m_RegisterF.Z = (reg == 0);m_RegisterF.N = 0;m_RegisterF.H = 0;m_RegisterF.C = 0; }

// Register Macros
#define M_SetHL(NewHL) {m_RegisterH = (NewHL & 0xFF00) >> 8;m_RegisterL = NewHL & 0x00FF;}
#define M_IncrementHL() {word hl = M_RegisterHL; hl++; m_RegisterH = (hl & 0xFF00)>>8; m_RegisterL = hl & 0x00FF;}
#define M_DecrementHL() {word hl = M_RegisterHL; hl--; m_RegisterH = (hl & 0xFF00)>>8; m_RegisterL = hl & 0x00FF;}
#define M_RegisterAF ((m_RegisterA << 8) + (m_RegisterF.all))
#define M_RegisterBC ((m_RegisterB << 8) + (m_RegisterC))
#define M_RegisterDE ((m_RegisterD << 8) + (m_RegisterE))
#define M_RegisterHL ((m_RegisterH << 8) + (m_RegisterL))
#define M_CPReg(reg) {byte result = m_RegisterA - reg;m_RegisterF.Z = (m_RegisterA == reg);m_RegisterF.N = 1;m_RegisterF.H = ((m_RegisterA & 0xF) - (reg & 0xF)) < 0;m_RegisterF.C = (m_RegisterA < reg);}
#define M_SBC_Reg(reg) {byte tempC = m_RegisterF.C; m_RegisterF.H = ((m_RegisterA & 0xF) - (reg & 0xF) - tempC) < 0;m_RegisterF.C = m_RegisterA < (reg + tempC);m_RegisterF.N = 1;m_RegisterA -= (reg + tempC);m_RegisterF.Z = (m_RegisterA == 0); }
#define M_OR_Reg(reg) {m_RegisterA |= reg;m_RegisterF.Z = (m_RegisterA == 0);m_RegisterF.C = 0;m_RegisterF.H = 0;m_RegisterF.N = 0;}
#define M_INC_Reg(reg) {m_RegisterF.H = (((reg & 0xF) +1) > 0xF);reg++;m_RegisterF.N = 0;m_RegisterF.Z = (reg == 0);}
#define M_DEC_Reg(reg) {byte Original = reg;reg--;m_RegisterF.Z = (reg == 0);m_RegisterF.N = 1;m_RegisterF.H = ((Original & 0x0F) == 0);}
#define M_CheckBit7Carry(reg, val) (((reg)+(val)) > 0xFF)
#define M_CheckBit3Carry(reg, val) ((((reg) & 0x0F)+((val) & 0x0F)) > 0x0F)
#define M_XOR_Reg(reg) {m_RegisterA ^= reg;m_RegisterF.Z = (m_RegisterA == 0);m_RegisterF.N = 0;m_RegisterF.H = 0;m_RegisterF.C = 0;}
#define M_AND_Reg(reg) {m_RegisterA &= reg;m_RegisterF.Z = (m_RegisterA == 0);m_RegisterF.N = 0;m_RegisterF.H = 1;m_RegisterF.C = 0;}

#define M_DisableInterrupts() {m_ReferenceInterruptStatus = 0;m_PendingInterruptStatusUpdate = true;InterruptInstructionJustExecuted = true; }
#define M_EnableInterrupts() {m_ReferenceInterruptStatus = 1;m_PendingInterruptStatusUpdate = true;m_InterruptInstructionJustExecuted = true; }


class GBCPU
{
private:
	// Registers
	gb_reg m_RegisterA = 0;
	gb_reg m_RegisterB = 0;
	gb_reg m_RegisterC = 0;
	gb_reg m_RegisterD = 0;
	gb_reg m_RegisterE = 0;
	flag_reg m_RegisterF = { 0 };
	gb_reg m_RegisterH = 0;
	gb_reg m_RegisterL = 0;
	gb_reg m_RegisterIME = 0;
	gb_reg m_RegisterIMA = 0;
	word m_ProgramCounter = 0;
	word m_ReferenceProgramCounter = 0;
	byte m_ReferenceInterruptStatus;

	int (GBCPU::*m_OpHandlers[NUM_OP_TYPES])(Instruction&) = { NULL };

	GBMMU *m_MMU;

	EmulatorEngine *m_Emulator;

	bool m_PendingInterruptStatusUpdate = false;
	bool m_InterruptInstructionJustExecuted = false;
	
	std::ofstream m_logfile;


	int ExecuteJumpCallOp(Instruction &Op);
	int ExecuteMiscControlOp(Instruction &Op);
	int ExecuteLdStrMv8Op(Instruction &Op);
	int ExecuteLdStrMv16Op(Instruction &Op);
	int Execute8BitAluOp(Instruction &Op);
	int Execute16BitAluOp(Instruction &Op);
	int ExecuteRotShftBitOp(Instruction &Op);
	int ExecuteCBPrefixOp(Instruction &Op);

public:
	word m_StackPointer = 0;
	void Reset();
	int ExecuteNextOp();
	word GetProgramCounter();
	word GetReferenceProgramCounter();
	word GetRegisterValue(RegisterID RegisterID);
	void SetProgramCounter(word PCValue);
	void HandleInterrupts();
	Instruction &GetInstruction(opcode OpCode);
	Instruction &GetCBInstruction(opcode OpCode);


	GBCPU(GBMMU *MMU, EmulatorEngine *Emulator);
	~GBCPU();
};

