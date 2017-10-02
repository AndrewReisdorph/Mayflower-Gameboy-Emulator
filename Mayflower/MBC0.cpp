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
	// Initialize Value with 0xFF. This is the default value for reads to memory
	// locations which exceed the bounds of the cartridge storage
	byte Value = 0xFF;

	if (Address >= 0x0000 && Address <= 0x7FFF)
	{
		if (Address < m_CartAttrs.RomSize)
		{
			Value = m_CartridgeROM[Address];
		}
	}
	else if (Address >= 0xA000 && Address <= 0xBFFF && Address < m_CartAttrs.RamSize)
	{
		Value = m_CartridgeRAM[(Address - 0xA000)];
	}
	else
	{
		// Invalid Memory Address. The MMU should not ever request memory outside
		// the given regions.
		throw exception();
	}

	return Value;
}

void MBC0::SetRomBankNumber(byte BankNumber)
{
	return;
}

void MBC0::SetRamBankNumber(byte BankNumber)
{
	return;
}

void MBC0::SetRamEnabled(bool Enabled)
{
	return;
}

void MBC0::SetRamBuffer(byte *RamBuffer, int Size)
{
	if (m_CartAttrs.RamSize == Size)
	{
		memcpy(m_CartridgeRAM, RamBuffer, Size);
	}
	else
	{
		throw std::exception();
	}
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
