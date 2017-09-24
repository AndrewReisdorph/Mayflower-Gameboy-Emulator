#pragma once
#include "Cartridge.h"
#include "MemoryBankController.h"
#include "GBTypes.h"

class MBC0 :
	public MemoryBankController
{
private:
	cart_attrs m_CartAttrs;
	byte *m_CartridgeROM = nullptr;
	byte *m_CartridgeRAM = nullptr;
	byte m_RamBank = 0;
	byte m_RomBank = 1;

public:
	byte GetRomBankNumber();
	byte GetRamBankNumber();
	byte ReadMemory8(word Address);
	void WriteMemory8(word Address, byte Value);
	void Destroy();
	MBC0(cart_attrs CartAttrs, byte *CartridgeROM);
};

