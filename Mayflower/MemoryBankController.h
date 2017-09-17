#pragma once
#include "GBTypes.h"

enum mbc_mode
{
	MBC_MODE_ROM,
	MBC_MODE_RAM
};

static const int RamSizeMap[] = {0, 2048, 8192, 32768, 131072, 65536};

class MemoryBankController
{
public:
	virtual byte ReadMemory8(word Address) = 0;
	virtual void WriteMemory8(word Address, byte value) = 0;
	virtual byte GetRomBankNumber() = 0;
	virtual byte GetRamBankNumber() = 0;
};

