#include "MBC3.h"

using namespace std;

MBC3::MBC3(cart_attrs CartAttrs, byte *CartridgeROM)
{
	m_CartAttrs = CartAttrs;
	m_CartridgeROM = CartridgeROM;

	InitRam();

	m_RtcStartTime = chrono::duration_cast< chrono::microseconds >(chrono::system_clock::now().time_since_epoch());
	m_RomBank = 1;
	m_LastLatchWriteValue = -1;
	m_RtcDH.all = 0;
}

byte MBC3::ReadMemory8(word Address)
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
		if (m_Mode == MBC_MODE_RAM)
		{
			int RamAddress = (m_RamBank * RAM_BANK_SIZE) + (Address - 0xA000);
			if (RamAddress < m_CartAttrs.RamSize)
			{
				Value = m_CartridgeRAM[RamAddress];
			}
		}
		else if (m_Mode == MBC_MODE_RTC)
		{
			Value = ReadRTC();
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

void MBC3::WriteMemory8(word Address, byte Value)
{
	if (Address >= 0x0000 && Address <= 0x1FFF)
	{
		m_RamEnabled = (Value & 0xA) ? true : false;
	}
	else if (Address >= 0x2000 && Address <= 0x3FFF)
	{
		SwitchRomBank(Value & 0x7F);
	}
	else if (Address >= 0x4000 && Address <= 0x5FFF)
	{
		if(Value <= 0x03)
		{
			m_RamBank = Value & 0x3;
			m_Mode = MBC_MODE_RAM;
		}
		else if (Value >= 0x08 && Value <= 0x0C)
		{
			m_RtcReg = (rtc_reg)Value;
			m_Mode = MBC_MODE_RTC;
		}
	}
	else if (Address >= 0x6000 && Address <= 0x7FFF)
	{
		if (m_LastLatchWriteValue == 0 && Value == 1)
		{
			m_RtcLatchedTime = chrono::duration_cast<chrono::microseconds>(chrono::system_clock::now().time_since_epoch() - m_RtcStartTime);
		}
		m_LastLatchWriteValue = Value;
	}
	else if (Address >= 0xA000 && Address <= 0xBFFF)
	{
		if (m_Mode == MBC_MODE_RAM)
		{
			int RamOffset = (m_RamBank * RAM_BANK_SIZE) + (Address - 0xA000);
			m_CartridgeRAM[RamOffset] = Value;
		}
		else if (m_Mode == MBC_MODE_RTC)
		{
			WriteRTC(Value);
		}
	}
}

void MBC3::WriteRTC(byte Value)
{

}

byte MBC3::ReadRTC()
{
	auto TempTime = m_RtcLatchedTime;
	auto ElapsedHours = std::chrono::duration_cast<std::chrono::hours>(TempTime);
	TempTime -= ElapsedHours;
	auto ElapsedMinutes = std::chrono::duration_cast<std::chrono::minutes>(TempTime);
	TempTime -= ElapsedMinutes;
	auto ElapsedSeconds = std::chrono::duration_cast<std::chrono::seconds>(TempTime);

	byte RtcVal = 0;

	switch (m_RtcReg)
	{
	case RTC_REG_SECONDS:
		RtcVal = ElapsedSeconds.count();
		break;
	case RTC_REG_MINUTES:
		RtcVal = ElapsedMinutes.count();
		break;
	case RTC_REG_HOURS:
		RtcVal = ElapsedHours.count();
		break;
	case RTC_REG_DAY_LSB:
		break;
	case RTC_REG_DAT_MSB:
		RtcVal = m_RtcDH.all;
		break;
	default:
		break;
	}

	return RtcVal;
}

void MBC3::SwitchRomBank(byte BankNumber)
{
	m_RomBank = BankNumber ? BankNumber : 1;
}

void MBC3::SetRamBankNumber(byte BankNumber)
{
	m_RamBank = BankNumber;
}

void MBC3::SetRomBankNumber(byte BankNumber)
{
	m_RomBank = BankNumber;
}

byte MBC3::GetRomBankNumber()
{
	return m_RomBank;
}

void MBC3::SetRamEnabled(bool Enabled)
{
	m_RamEnabled = Enabled;
}

void MBC3::SetRamBuffer(byte *RamBuffer, int Size)
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

byte MBC3::GetRamBankNumber()
{
	return m_RamBank;
}

void MBC3::InitRam()
{
	m_CartridgeRAM = new byte[m_CartAttrs.RamSize]();

	wxFileName CartFileName(m_CartAttrs.FilePath);
	wxFileName SaveFileName(CartFileName);
	SaveFileName.SetExt("sav");

	if (wxFileExists(SaveFileName.GetFullPath()))
	{
		wxFile SaveFile(SaveFileName.GetFullPath());
		wxULongLong FileSize = wxFileName::GetSize(SaveFileName.GetFullPath());
		if (FileSize == m_CartAttrs.RamSize)
		{
			SaveFile.Read(m_CartridgeRAM, m_CartAttrs.RamSize);
		}
		SaveFile.Close();
	}
}

void MBC3::Destroy()
{
	switch (m_CartAttrs.CartType)
	{
	case CART_MBC3_RAM_BAT:
	case CART_MBC3_TIM_RAM_BAT:
	{
		wxFileName CartFileName(m_CartAttrs.FilePath);
		wxFileName SaveFileName(CartFileName);
		SaveFileName.SetExt("sav");
		wxFile SaveFile(SaveFileName.GetFullPath(), wxFile::write);
		SaveFile.Write(m_CartridgeRAM, m_CartAttrs.RamSize);
		SaveFile.Close();
	}
	break;
	default:
		break;
	}

	delete m_CartridgeRAM;

}