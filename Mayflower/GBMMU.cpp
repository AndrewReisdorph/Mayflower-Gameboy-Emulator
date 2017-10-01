#include "GBMMU.h"
#include <fstream>

using namespace std;

GBMMU::GBMMU(EmulatorEngine *Emulator)
{
	m_Emulator = Emulator;

	m_Cartridge = new Cartridge(m_Emulator);
}

void GBMMU::Reset()
{
	m_BootRomEnabled = true;
	m_Memory[IF_REGISTER] = 0xE1;
	m_Memory[STAT_REGISTER] = 0x84;
	m_Memory[TAC_REGISTER] = 0xF8;
	m_Memory[0xFF4D] = 0xFF;
}

void GBMMU::LoadState(SaveState &State)
{
	state_entry StateEntryDIV = State.Get("DIV");
	if (StateEntryDIV.Buffer != nullptr)
	{
		SetDIV(*StateEntryDIV.Buffer);
	}

	state_entry StateEntryTMA = State.Get("TMA");
	if (StateEntryTMA.Buffer != nullptr)
	{
		WriteMemory8(TMA_REGISTER, *StateEntryTMA.Buffer, true);
	}

	state_entry StateEntryTIMA = State.Get("TIMA");
	if (StateEntryTIMA.Buffer != nullptr)
	{
		WriteMemory8(TIMA_REGISTER, *StateEntryTIMA.Buffer, true);
	}

	state_entry StateEntryTAC = State.Get("TAC");
	if (StateEntryTAC.Buffer != nullptr)
	{
		WriteMemory8(TAC_REGISTER, *StateEntryTAC.Buffer, true);
	}

	state_entry StateEntryIF = State.Get("IF");
	if (StateEntryIF.Buffer != nullptr)
	{
		WriteMemory8(IF_REGISTER, *StateEntryIF.Buffer, true);
	}

	state_entry StateEntryIE = State.Get("IE");
	if (StateEntryIE.Buffer != nullptr)
	{
		WriteMemory8(IE_REGISTER, *StateEntryIE.Buffer, true);
	}

	state_entry StateEntryLCDC = State.Get("LCDC");
	if (StateEntryLCDC.Buffer != nullptr)
	{
		WriteMemory8(LCDC_REGISTER, *StateEntryLCDC.Buffer, true);
	}

	state_entry StateEntrySTAT = State.Get("STAT");
	if (StateEntrySTAT.Buffer != nullptr)
	{
		WriteMemory8(STAT_REGISTER, *StateEntrySTAT.Buffer, true);
	}

	state_entry StateEntrySCY = State.Get("SCY");
	if (StateEntrySCY.Buffer != nullptr)
	{
		WriteMemory8(SCY_REGISTER, *StateEntrySCY.Buffer, true);
	}

	state_entry StateEntrySCX = State.Get("SCX");
	if (StateEntrySCX.Buffer != nullptr)
	{
		WriteMemory8(SCX_REGISTER, *StateEntrySCX.Buffer, true);
	}

	state_entry StateEntryLY = State.Get("LY");
	if (StateEntryLY.Buffer != nullptr)
	{
		WriteMemory8(LY_REGISTER, *StateEntryLY.Buffer, true);
	}

	state_entry StateEntryLYC = State.Get("LYC");
	if (StateEntryLYC.Buffer != nullptr)
	{
		WriteMemory8(LYC_REGISTER, *StateEntryLYC.Buffer, true);
	}

	state_entry StateEntryWINX = State.Get("WINX");
	if (StateEntryWINX.Buffer != nullptr)
	{
		WriteMemory8(WX_REGISTER, *StateEntryWINX.Buffer, true);
	}

	state_entry StateEntryWINY = State.Get("WINY");
	if (StateEntryWINY.Buffer != nullptr)
	{
		WriteMemory8(WY_REGISTER, *StateEntryWINY.Buffer, true);
	}

	state_entry StateEntryDMA = State.Get("DMA");
	if (StateEntryDMA.Buffer != nullptr)
	{
		WriteMemory8(DMA_REGISTER, *StateEntryDMA.Buffer, true);
	}

	state_entry StateEntryOAM = State.Get("OAM");
	if (StateEntryOAM.Buffer != nullptr)
	{
		memcpy(&m_Memory[OAM_START_ADDR], StateEntryOAM.Buffer, OAM_TRANSFER_LEN);
	}

	state_entry StateEntryHRAM = State.Get("HRAM");
	if (StateEntryHRAM.Buffer != nullptr)
	{
		memcpy(&m_Memory[HRAM_START_ADDR], StateEntryHRAM.Buffer, HRAM_SIZE);
	}

	state_entry StateEntryWRAM = State.Get("WRAM");
	if (StateEntryWRAM.Buffer != nullptr)
	{
		memcpy(&m_Memory[WRAM_START_ADDR], StateEntryWRAM.Buffer, WRAM_SIZE);
	}

	state_entry StateEntryVRAM = State.Get("VRAM");
	if (StateEntryVRAM.Buffer != nullptr)
	{
		memcpy(&m_Memory[VRAM_START_ADDR], StateEntryVRAM.Buffer, VRAM_SIZE);
	}

	state_entry StateEntryRomBank = State.Get("ROMB");
	if (StateEntryRomBank.Buffer != nullptr)
	{
		m_Cartridge->SetRomBankNumber(*((uint32*)StateEntryRomBank.Buffer));
	}

	state_entry StateEntryRamBank = State.Get("SRAMB");
	if (StateEntryRamBank.Buffer != nullptr)
	{
		m_Cartridge->SetRamBankNumber(*((uint32*)StateEntryRamBank.Buffer));
	}
	
	state_entry StateEntryRamEnable = State.Get("sramenable");
	if (StateEntryRamEnable.Buffer != nullptr)
	{
		m_Cartridge->SetRamEnabled(*StateEntryRamEnable.Buffer);
	}

	state_entry StateEntryRamBuffer = State.Get("SRAM");
	if (StateEntryRamBuffer.Buffer != nullptr)
	{
		m_Cartridge->SetRamBuffer(StateEntryRamBuffer.Buffer, StateEntryRamBuffer.Size);
	}

	state_entry StateEntryBootrom = State.Get("bootrom");
	if (StateEntryBootrom.Size == 0)
	{
		m_BootRomEnabled = false;
	}
	else
	{
		m_BootRomEnabled = true;
	}
}

void GBMMU::SetCartridge(wxString RomFilePath)
{
	m_Cartridge->Initialize(RomFilePath);
}

byte GBMMU::GetRomBankNumber()
{
	return m_Cartridge->GetRomBankNumber();
}

byte GBMMU::GetRamBankNumber()
{
	return m_Cartridge->GetRamBankNumber();
}

bool GBMMU::GetBootRomEnabled()
{
	return m_BootRomEnabled;
}

void GBMMU::SetCPU(GBCPU *CPU)
{
	m_CPU = CPU;
}

void GBMMU::SetDIV(byte DivValue)
{
	m_Memory[DIV_REGISTER] = DivValue;
}

void GBMMU::DMATransfer()
{
	word SourceAddr = ReadMemory8(DMA_REGISTER) << 8;
	for (int ByteIter = 0; ByteIter < OAM_TRANSFER_LEN; ByteIter++)
	{
		WriteMemory8(OAM_START_ADDR + ByteIter, ReadMemory8(SourceAddr + ByteIter));
	}
}

word GBMMU::StackPop()
{
	word PopValue = (ReadMemory8(m_CPU->m_StackPointer + 1) << 8) + ReadMemory8(m_CPU->m_StackPointer);
	m_CPU->m_StackPointer += 2;
	return PopValue;
}

void GBMMU::StackPush(word value)
{
	m_CPU->m_StackPointer -= 2;
	WriteMemory8(m_CPU->m_StackPointer, value & 0xFF);
	WriteMemory8(m_CPU->m_StackPointer + 1, (value & 0xFF00) >> 8);
}

void GBMMU::WriteMemory8(word Address, byte Value, bool DirectWrite)
{
	// Address is in cartridge, let the cartridge handle the operation
	if ((Address >= 0x0000 && Address <= 0x7FFF) ||
		(Address >= 0xA000 && Address <= 0xBFFF))
	{
		m_Cartridge->WriteMemory8(Address, Value);
	}
	else
	{
		if (DirectWrite)
		{
			m_Memory[Address] = Value;
		}
		else
		{
			switch (Address)
			{
			case STAT_REGISTER:
			case SC_REGISTER:
				break;
			case TAC_REGISTER:
				if (m_Memory[TAC_REGISTER] != (0xF8 | Value))
				{
					m_Memory[TAC_REGISTER] = (0xF8 | Value);
					m_Memory[TIMA_REGISTER] = m_Memory[TMA_REGISTER];
					m_Emulator->ResetClockFrequency();
				}
				break;
			case IF_REGISTER:
				m_Memory[Address] = Value | 0xE0;
				break;
			case DIV_REGISTER:
				m_Memory[DIV_REGISTER] = 0x00;
				break;
			case DSROM_REGISTER:
				m_BootRomEnabled = false;
				m_Emulator->SetPendingInstructionListUpdate();
				break;
			default:
				m_Memory[Address] = Value;
				if (Address == DMA_REGISTER)
				{
					DMATransfer();
				}
				break;
			}
		}

	}
}

void GBMMU::WriteMemory16(word Address, word Value)
{
	WriteMemory8(Address, (Value & 0x00FF));
	WriteMemory8(Address + 1, (Value & 0xFF00) >> 8);
}

byte GBMMU::ReadMemory8(word Address)
{
	byte Value;

	if (m_BootRomEnabled && (Address >= 0 && Address <= 0x100))
	{
		Value = BootRom[Address];
	}
	else if ((Address >= 0x0000 && Address <= 0x7FFF) ||
		     (Address >= 0xA000 && Address <= 0xBFFF))
	{
		Value = m_Cartridge->ReadMemory8(Address);
	}
	else
	{
		// Adjust reads of echo ram to real location
		if (Address >= ECHO_RAM_ADDR && Address <= 0xFDFF)
		{
			Address -= 0x2000;
		}

		Value = m_Memory[Address];
		switch (Address)
		{
		case P1_REGISTER:
		{
			joypad_info Joypad = m_Emulator->GetJoypad();
			Value = (Value & 0x30) | 0xC0;
			if (M_TestBit(Value, BIT_4))
			{
				Value |= Joypad.A ? 0 : JOYPAD_A;
				Value |= Joypad.B ? 0 : JOYPAD_B;
				Value |= Joypad.Select ? 0 : JOYPAD_SELECT;
				Value |= Joypad.Start ? 0 : JOYPAD_START;


			}
			else if (M_TestBit(Value, BIT_5))
			{
				Value |= Joypad.Right ? 0 : JOYPAD_RIGHT;
				Value |= Joypad.Left ? 0 : JOYPAD_LEFT;
				Value |= Joypad.Up ? 0 : JOYPAD_UP;
				Value |= Joypad.Down ? 0 : JOYPAD_DOWN;
			}
		}
		break;
		}

	}

	return Value;
}

word GBMMU::ReadMemory16(word Address)
{
	return ReadMemory8(Address) | (ReadMemory8(Address + 1) << 8);
}

GBMMU::~GBMMU()
{
	delete m_Cartridge;
}
