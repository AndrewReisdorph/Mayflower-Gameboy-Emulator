#include "MBC1.h"

using namespace std;

MBC1::MBC1(cart_attrs CartAttrs, byte *CartridgeROM)
{
	m_CartAttrs = CartAttrs;
	m_CartridgeROM = CartridgeROM;

	m_CartridgeRAM = new byte[m_CartAttrs.RamSize]();

	m_RomBank = 1;
}

byte MBC1::ReadMemory8(word Address)
{
	byte Value = 0;
	
	if (Address >= 0x0000 && Address <= 0x3FFF)
	{
		Value = m_CartridgeROM[Address];
	}
	else if (Address >= 0x4000 && Address <= 0x7FFF)
	{
		int RomOffset = (m_RomBank * ROM_BANK_SIZE) + (Address - 0x4000);
		Value = m_CartridgeROM[RomOffset];
	}
	else if (Address >= 0xA000 && Address <= 0xBFFF)
	{
		int RamAddress = (m_RamBank * RAM_BANK_SIZE) + (Address - 0xA000);
		Value = m_CartridgeRAM[RamAddress];
	}
	else
	{
		// Invalid Memory Address
		throw exception();
	}

	return Value;
}

void MBC1::WriteMemory8(word Address, byte Value)
{
	if (Address >= 0x0000 && Address <= 0x1FFF)
	{
		m_RamEnabled = (Value & 0xA) ? true : false;
	}
	else if (Address >= 0x2000 && Address <= 0x3FFF)
	{
		SwitchRomBank(Value & 0x1F);
	}
	else if (Address >= 0x4000 && Address <= 0x5FFF)
	{
		if (m_Mode == MBC_MODE_ROM)
		{
			SwitchRomBank((Value & 0x60) | m_RomBank);
		}
		else // m_Mode == MBC_MODE_RAM
		{
			m_RamBank = Value & 0x3;
		}
	}
	else if (Address >= 0x6000 && Address <= 0x7FFF)
	{
		if (Value == 0)
		{
			m_Mode = MBC_MODE_ROM;
		}
		else if (Value == 1)
		{
			m_Mode = MBC_MODE_RAM;
		}
	}
	else if (Address >= 0xA000 && Address <= 0xBFFF)
	{
		int RamOffset = (m_RamBank * RAM_BANK_SIZE) + (Address - 0xA000);
		m_CartridgeRAM[RamOffset] = Value;
	}
}

void MBC1::SwitchRomBank(byte BankNumber)
{
	if (BankNumber == 0 || BankNumber & 0x60 || BankNumber & 0x40 || BankNumber & 0x20)
	{
		BankNumber |= 1;
	}
	m_RomBank = BankNumber;
}

byte MBC1::GetRomBankNumber()
{
	return m_RomBank;
}

byte MBC1::GetRamBankNumber()
{
	return m_RamBank;
}

MBC1::~MBC1()
{
	delete m_CartridgeRAM;
}
