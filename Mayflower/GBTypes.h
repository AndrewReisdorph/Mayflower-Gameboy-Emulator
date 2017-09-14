#pragma once

#define M_TestBit(val,bit) (val & bit ? 1 : 0)
#define M_ClearBit(val, bit) (val &= ~bit)

#define BIT_0 0x01
#define BIT_1 0x02
#define BIT_2 0x04
#define BIT_3 0x08
#define BIT_4 0x10
#define BIT_5 0x20
#define BIT_6 0x40
#define BIT_7 0x80

typedef char sint8;
typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned char gb_reg;
typedef unsigned char opcode;

enum RegisterID
{
	REGISTER_AF,
	REGISTER_BC,
	REGISTER_DE,
	REGISTER_HL,
	REGISTER_SP,
	REGISTER_PC,
	REGISTER_IE,
	REGISTER_IF,
	REGISTER_IME,
	REGISTER_IMA,
	REGISTER_LCDC,
	REGISTER_STAT,
	REGISTER_LY,
	REGISTER_ROM,
	REGISTER_DIV,
	REGISTER_TIMA
};