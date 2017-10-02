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
	// Initialize Value with 0xFF. This is the default value for reads to memory
	// locations which exceed the bounds of the cartridge storage
	byte Value = 0xFF;
	
	if (Address >= 0x0000 && Address <= 0x3FFF)
	{
		if (Address < m_CartAttrs.RomSize)
		{
			Value = m_CartridgeROM[Address];
		}
	}
	else if (Address >= 0x4000 && Address <= 0x7FFF)
	{
		int RomOffset = (m_RomBank * ROM_BANK_SIZE) + (Address - 0x4000);
		if (RomOffset < m_CartAttrs.RomSize)
		{
			Value = m_CartridgeROM[RomOffset];
		}
	}
	else if (Address >= 0xA000 && Address <= 0xBFFF)
	{
		int RamAddress = (m_RamBank * RAM_BANK_SIZE) + (Address - 0xA000);
		if (RamAddress < m_CartAttrs.RamSize)
		{
			Value = m_CartridgeRAM[RamAddress];
		}
	}
	else
	{
		// Invalid Memory Address. The MMU should not ever request memory outside
		// the given regions.
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

void MBC1::SetRomBankNumber(byte BankNumber)
{
	m_RomBank = BankNumber;
}

byte MBC1::GetRomBankNumber()
{
	return m_RomBank;
}

void MBC1::SetRamEnabled(bool Enabled)
{
	m_RamEnabled = Enabled;
}

void MBC1::SetRamBankNumber(byte BankNumber)
{
	m_RamBank = BankNumber;
}

void MBC1::SetRamBuffer(byte *RamBuffer, int Size)
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

byte MBC1::GetRamBankNumber()
{
	return m_RamBank;
}

void MBC1::Destroy()
{
	delete m_CartridgeRAM;
}
