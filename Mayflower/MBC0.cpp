#include "MBC0.h"

using namespace std;

MBC0::MBC0(cart_attrs CartAttrs, byte *CartridgeROM)
{
	m_CartAttrs = CartAttrs;
	m_CartridgeROM = CartridgeROM;

	if (m_CartAttrs.RamSize > 0)
	{
		m_CartridgeRAM = new byte[m_CartAttrs.RamSize]();
	}

	m_RomBank = 1;

}

void MBC0::WriteMemory8(word Address, byte Value)
{
	if (Address >= 0xA000 && Address <= 0xBFFF)
	{
		m_CartridgeRAM[Address - 0xA000] = Value;
	}
	else
	{
		throw exception();
	}
}

byte MBC0::ReadMemory8(word Address)
{
	byte Value = 0;

	if (Address >= 0x0000 && Address <= 0x7FFF)
	{
		Value = m_CartridgeROM[Address];
	}
	else if (Address >= 0xA000 && Address <= 0xBFFF)
	{
		Value = m_CartridgeRAM[(Address - 0xA000)];
	}
	else
	{
		// Invalid Memory Address
		throw exception();
	}

	return Value;
}

byte MBC0::GetRomBankNumber()
{
	return m_RomBank;
}

byte MBC0::GetRamBankNumber()
{
	return m_RamBank;
}

void MBC0::Destroy()
{
	if (m_CartridgeRAM != nullptr)
	{
		delete m_CartridgeRAM;
	}
}
