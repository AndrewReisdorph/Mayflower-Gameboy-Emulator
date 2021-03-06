#pragma once
#include "GBTypes.h"

enum mbc_mode
{
	MBC_MODE_ROM,
	MBC_MODE_RAM,
	MBC_MODE_RTC
};

static const int RamSizeMap[] = {0, 2048, 8192, 32768, 131072, 65536};
static const int RomSizeMap[] = {32768, 65536, 131072, 262144, 524288, 1048576, 2097152, 4194304, 8388608, 1179648, 1310720, 1572864};

class MemoryBankController
{
public:
	virtual byte ReadMemory8(word Address) = 0;
	virtual void WriteMemory8(word Address, byte value) = 0;
	virtual void SetRomBankNumber(byte BankNumber) = 0;
	virtual void SetRamBankNumber(byte BankNumber) = 0;
	virtual void SetRamEnabled(bool Enabled) = 0;
	virtual void SetRamBuffer(byte *RamBuffer, int Size) = 0;
	virtual byte GetRomBankNumber() = 0;
	virtual byte GetRamBankNumber() = 0;
	virtual void Destroy() = 0;
};

