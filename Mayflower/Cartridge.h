#pragma once
#include <wx/wx.h>
#include "EmulatorEngine.h"
#include "MemoryBankController.h"
#include "MBC1.h"
#include "GBTypes.h"

#define CART_TYPE_ADDR     0x0147
#define CART_ROM_SIZE_ADDR 0x0148
#define CART_RAM_SIZE_ADDR 0x0149




class Cartridge
{
private:
	EmulatorEngine *m_Emulator = nullptr;
	byte *m_RomData = nullptr;
	MemoryBankController *m_MBC = nullptr;
	cart_attrs m_CartAttrs;

	bool ReadRomFile(wxString RomFilePath);
	void InitMBC();
	void ReadCartAttributes();

public:
	byte GetRomBankNumber();
	byte GetRamBankNumber();
	byte ReadMemory8(word Address);
	void WriteMemory8(word Address, byte Value);
	bool Initialize(wxString RomFilePath);
	Cartridge(EmulatorEngine *Emulator);
	~Cartridge();
};

