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

enum CartridgeType
{
	CART_ROM_ONLY = 0x00,
	CART_MBC1 = 0x01,
	CART_MBC1_RAM = 0x02,
	CART_MBC1_RAM_BAT = 0x03,
	CART_MBC2 = 0x05,
	CART_MBC2_BAT = 0x06,
	CART_ROM_RAM = 0x08,
	CART_ROM_RAM_BAT = 0x09,
	CART_MMM01 = 0x0B,
	CART_MMM01_RAM = 0x0C,
	CART_MMM01_RAM_BAT = 0x0D,
	CART_MBC3_TIM_BAT = 0x0F,
	CART_MBC3_TIM_RAM_BAT = 0x10,
	CART_MBC3 = 0x11,
	CART_MBC3_RAM = 0x12,
	CART_MBC3_RAM_BAT = 0x13,
	CART_MBC5 = 0x19,
	CART_MBC5_RAM = 0x1A,
	CART_MBC5_RAM_BAT = 0x1B,
	CART_MBC5_RUM = 0x1C,
	CART_MBC5_RUM_RAM = 0x1D,
	CART_MBC5_RUM_RAM_BAT = 0x1E,
	CART_MBC6 = 0x20,
	CART_MBC7_SENS_RUM_RAM_BAT = 0x22,
	CART_POCKET_CAMERA = 0xFC,
	CART_BANDAI_TAMAS = 0xFD,
	CART_HUC3 = 0xFE,
	CART_HUC1_RAM_BAT = 0xFF
};

typedef struct cart_attrs
{
	CartridgeType CartType;
	int RomSize;
	int RamSize;
}cart_attrs;
