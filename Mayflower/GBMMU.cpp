#include "GBMMU.h"
#include <fstream>

using namespace std;

GBMMU::GBMMU(EmulatorEngine *Emulator)
{
	m_Emulator = Emulator;
	m_Memory[IF_REGISTER] = 0xE1;
	m_Memory[STAT_REGISTER] = 0x84;
	m_Memory[TAC_REGISTER] = 0xF8;
}

// GameBoy Memory Areas
// $FFFF	        Interrupt Enable Flag
// $FF80 - $FFFE	Zero Page - 127 bytes
// $FF00 - $FF7F	Hardware I / O Registers
// $FEA0 - $FEFF	Unusable Memory
// $FE00 - $FE9F	OAM - Object Attribute Memory
// $E000 - $FDFF	Echo RAM - Reserved, Do Not Use
// $D000 - $DFFF	Internal RAM - Bank 1 - 7 (switchable - CGB only)
// $C000 - $CFFF	Internal RAM - Bank 0 (fixed)
// $A000 - $BFFF	Cartridge RAM(If Available)
// $9C00 - $9FFF	BG Map Data 2
// $9800 - $9BFF	BG Map Data 1
// $8000 - $97FF	Character RAM
// $4000 - $7FFF	Cartridge ROM - Switchable Banks 1 - xx
// $0150 - $3FFF	Cartridge ROM - Bank 0 (fixed)
// $0100 - $014F	Cartridge Header Area
// $0000 - $00FF	Restart and Interrupt Vectors
void GBMMU::InitMemory()
{
	// Load rom banks 0 and 1
	for (int ByteIter = 0; ByteIter <= 0x7FFF; ByteIter++)
	{
		m_Memory[ByteIter] = m_RomBin[ByteIter];
	}

	// Load bootrom
	for (int ByteIter = 0; ByteIter < 0x100; ByteIter++)
	{
		m_Memory[ByteIter] = BootRom[ByteIter];
	}

#if 0
	m_Memory[P1_REGISTER] = 0xCF;
	m_Memory[SB_REGISTER] = 0x00;
	m_Memory[SC_REGISTER] = 0x7E;
	m_Memory[0xFF03] = 0xFF;
	m_Memory[LCDC_REGISTER] = 0x91;
	m_Memory[BGP_REGISTER] = 0xFC;
#endif
}

int GBMMU::GetCurrentRomBank()
{
	return m_CurrentRomBank;
}

void GBMMU::SetCPU(GBCPU *CPU)
{
	m_CPU = CPU;
}

void GBMMU::DMATransfer()
{
	word SourceAddr = m_Memory[DMA_REGISTER] << 8;
	for (int ByteIter = 0; ByteIter < OAM_TRANSFER_LEN; ByteIter++)
	{
		m_Memory[OAM_START_ADDR + ByteIter] = m_Memory[SourceAddr + ByteIter];
	}
}

word GBMMU::StackPop()
{
	word PopValue = (m_Memory[m_CPU->m_StackPointer + 1] << 8) + m_Memory[m_CPU->m_StackPointer];
	m_CPU->m_StackPointer += 2;
	return PopValue;
}

void GBMMU::StackPush(word value)
{
	m_CPU->m_StackPointer -= 2;
	m_Memory[m_CPU->m_StackPointer] = value & 0xFF;
	m_Memory[m_CPU->m_StackPointer + 1] = (value & 0xFF00) >> 8;
}

void GBMMU::WriteMemory8(word Address, byte value)
{
	if (Address >= 0x0000 && Address <= 0x1FFF)
	{
		m_CartRamEnabled = (value & 0xA) ? true : false;
	}
	else if (Address >= 0x2000 && Address <= 0x3FFF)
	{
		SwitchRomBank(value);
	}
	else if (Address >= 0x4000 && Address <= 0x5FFF)
	{
		SwitchRamBankOrRTCRegister(value);
	}
	else if (Address >= 0x6000 && Address <= 0x7FFF)
	{
		HandleRomRamSelectOrLatchClockData(value);
	}
	else if (Address <= 0x7FFF)
	{
		cout << "Writing to read only address: 0x" << hex << Address << "  That shit's fucked" << endl;
		cout << "PC: 0x" << hex << m_CPU->GetProgramCounter() << endl;
		throw exception();
	}
	else
	{
		switch (Address)
		{
		case SC_REGISTER:
			break;
		case TAC_REGISTER:
			if (m_Memory[TAC_REGISTER] != (0xF8 | value))
			{
				m_Memory[TAC_REGISTER] = (0xF8 | value);
				m_Memory[TIMA_REGISTER] = m_Memory[TMA_REGISTER];
				m_Emulator->ResetClockFrequency();
			}
			break;
		case IF_REGISTER:
			m_Memory[Address] = value | 0xE0;
			break;
		case DIV_REGISTER:
			m_Memory[DIV_REGISTER] = 0x00;
			break;
		case DSROM_REGISTER:
			// Load rom instructions over boot code
			for (int i = 0; i < 0x100; i++)
			{
				m_Memory[i] = m_RomBin[i];
			}
			m_BootRomEnabled = false;
			m_Emulator->SetPendingInstructionListUpdate();
			break;
		default:
			if (Address >= 0xA000 && Address <= 0xBFFF)
			{
				if (m_CartRamEnabled == false)
				{
					//throw exception();
				}
			}

			m_Memory[Address] = value;
			if (Address == DMA_REGISTER)
			{
				DMATransfer();
			}
			break;
		}
	}
}

void GBMMU::WriteMemory16(word Address, word value)
{
	m_Memory[Address] = (value & 0x00FF);
	m_Memory[Address + 1] = (value & 0xFF00) >> 8;

}

byte GBMMU::ReadMemory8(word Address)
{
	byte Value = m_Memory[Address];

	switch (Address)
	{
	case P1_REGISTER:
	{
		joypad_info Joypad = m_Emulator->GetJoypad();
		Value = (Value & 0x30) | 0xC0;
		if (M_TestBit(Value, BIT_4))
		{
			Value |= Joypad.A ? JOYPAD_A : 0;
			Value |= Joypad.B ? JOYPAD_B : 0;
			Value |= Joypad.Select ? JOYPAD_SELECT : 0;
			Value |= Joypad.Start ? JOYPAD_START : 0;


		}
		else if (M_TestBit(Value, BIT_5))
		{
			Value |= Joypad.Right ? JOYPAD_RIGHT : 0;
			Value |= Joypad.Left ? JOYPAD_LEFT : 0;
			Value |= Joypad.Up ? JOYPAD_UP : 0;
			Value |= Joypad.Down ? JOYPAD_DOWN : 0;
		}
	}
		break;
	}

	return Value;
}

word GBMMU::ReadMemory16(word Address)
{
	return m_Memory[Address] + (m_Memory[Address + 1] << 8);
}

void GBMMU::SwitchRamBankOrRTCRegister(byte BankOrRTC)
{
	switch (m_CartType)
	{
	case CART_MBC1_RAM:
	case CART_MBC1_RAM_BAT:
		if (m_BankMode == BANK_MODE_ROM)
		{
			SwitchRomBank(m_CurrentRomBank | (BankOrRTC & 0x60));
		}
		else // BANK_MODE_RAM
		{
			int SelectedRamBank = BankOrRTC & 0x3;

			if (SelectedRamBank == m_SelectedRamBank)
			{
				// Don't do anything
			}
			else
			{
				int BankOffset = m_SelectedRamBank * RAM_BANK_SIZE;
				// Load active bank ram back into cartridge ram buffer
				for (int RamByteIter = 0; RamByteIter < 0x1FFFF; RamByteIter++)
				{
					m_CartridgeRam[BankOffset + RamByteIter] = m_Memory[ADDR_CART_RAM + RamByteIter];
				}
				// Load new bank from cartridge ram buffer
				for (int RamByteIter = 0; RamByteIter < 0x1FFFF; RamByteIter++)
				{
					m_Memory[ADDR_CART_RAM + RamByteIter] = m_CartridgeRam[BankOffset + RamByteIter];
				}
			}
		}
		break;
	}
}

void GBMMU::HandleRomRamSelectOrLatchClockData(byte value)
{
	switch (m_CartType)
	{
	case CART_MBC1_RAM:
	case CART_MBC1_RAM_BAT:
		if (value == BANK_MODE_ROM || value == BANK_MODE_RAM)
		{
			m_BankMode = (BankingMode) value;
		}
		break;
	}
}

void GBMMU::SwitchRomBank(byte BankNumber)
{
	if (BankNumber == 0 || BankNumber & 0x60 )
	{
		BankNumber |= 1;
	}

	if (BankNumber != m_CurrentRomBank)
	{

		int RomBankAddr = ROM_BANK_SIZE * BankNumber;
		int ByteOffset = 0;
		for (int ByteIter = RomBankAddr; ByteIter <= (RomBankAddr + ROM_BANK_SIZE); ByteIter++)
		{
			m_Memory[ROM_BANK_N_ADDRESS + ByteOffset++] = m_RomBin[ByteIter];
		}

		m_CurrentRomBank = BankNumber;
		m_Emulator->SetPendingInstructionListUpdate();
	}

}

void GBMMU::SetupMBC()
{
	m_CartType = (CartridgeType) m_RomBin[0x147];

	switch (m_CartType)
	{

	}
}

void GBMMU::ReadRomFile(wxString RomFilePath)
{
	ifstream romFile;
	size_t rom_size = 0;
	romFile.open(RomFilePath.ToUTF8(), ios::in | ios::binary);
	if (romFile.is_open())
	{
		m_RomBin = NULL;
		romFile.seekg(0, ios::end);
		rom_size = romFile.tellg();
		romFile.seekg(0, ios::beg);
		m_RomBin = new byte[rom_size];
		romFile.read((char *)m_RomBin, rom_size);
	}

	SetupMBC();
}

GBMMU::~GBMMU()
{
}
