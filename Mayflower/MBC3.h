#pragma once
#include <wx/filefn.h> 
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/longlong.h>
#include "Cartridge.h"
#include "MemoryBankController.h"
#include "GBTypes.h"

enum rtc_reg
{
	RTC_REG_SECONDS = 0x08,
	RTC_REG_MINUTES = 0x09,
	RTC_REG_HOURS   = 0x0A,
	RTC_REG_DAY_LSB = 0x0B,
	RTC_REG_DAT_MSB = 0x0C
};

typedef union rtc_dh
{
	struct
	{
		byte DayCounterBit8  : 1;
		byte Reserved        : 5;
		byte Halt            : 1;
		byte DayCounterCarry : 1;
	};
	byte all;
} rtc_dh;

class MBC3 :
	public MemoryBankController
{
private:
	mbc_mode m_Mode = MBC_MODE_RAM;
	rtc_reg m_RtcReg = RTC_REG_SECONDS;
	rtc_dh m_RtcDH;
	int m_LastLatchWriteValue = -1;
	bool m_RamEnabled = false;
	bool m_BootRomEnabled = true;
	byte *m_CartridgeROM = nullptr;
	byte *m_CartridgeRAM = nullptr;
	cart_attrs m_CartAttrs;
	byte m_RamBank = 0;
	byte m_RomBank = 1;
	std::chrono::microseconds m_RtcStartTime;
	std::chrono::microseconds m_RtcLatchedTime;

	void SwitchRomBank(byte BankNumber);
	void WriteRTC(byte Value);
	byte ReadRTC();
	void InitRam();

public:
	byte GetRomBankNumber();
	void SetRomBankNumber(byte BankNumber);
	void SetRamBankNumber(byte BankNumber);
	void SetRamEnabled(bool Enabled);
	void SetRamBuffer(byte *RamBuffer, int Size);
	byte GetRamBankNumber();
	byte ReadMemory8(word Address);
	void WriteMemory8(word Address, byte Value);
	void Destroy();
	MBC3(cart_attrs CartAttrs, byte *CartridgeROM);
};

