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

void GBMMU::WriteMemory8(word Address, byte Value)
{
	// Address is in cartridge, let the cartridge handle the operation
	if ((Address >= 0x0000 && Address <= 0x7FFF) ||
		(Address >= 0xA000 && Address <= 0xBFFF))
	{
		m_Cartridge->WriteMemory8(Address, Value);
	}
	else
	{
		switch (Address)
		{
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

	}

	return Value;
}

word GBMMU::ReadMemory16(word Address)
{
	return ReadMemory8(Address) | (ReadMemory8(Address + 1) << 8);
}

GBMMU::~GBMMU()
{
}
