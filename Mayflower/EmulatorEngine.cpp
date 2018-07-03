#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <wx/wx.h>
#include <wx/event.h>
#include "EmulatorEngine.h"

using namespace std;

EmulatorEngine::EmulatorEngine(GameboyScreenPanel *ScreenPanel, RamAssemblyList *AssemblyListCtrl, MayflowerWindow *App)
{
	m_MainApp = App;
	m_ScreenPanel = ScreenPanel;
	m_AssemblyListCtrl = AssemblyListCtrl;

	m_MMU = new GBMMU(this);
	m_CPU = new GBCPU(m_MMU, this);
	m_MMU->SetCPU(m_CPU);
	m_LCD = new GBLCD(m_MMU, m_ScreenPanel, this);

}

void EmulatorEngine::EmulatorStateMachine()
{
	bool KeepGoing = true;

	while (KeepGoing)
	{
		switch (m_State)
		{
		case EMULATOR_STATE_IDLE:
			break;
		case EMULATOR_STATE_INIT:
			InitializeMachine();
			break;
		case EMULATOR_STATE_RUN:
			Emulate();
			break;
		case EMULATOR_STATE_QUIT:
			KeepGoing = false;
			break;
		}
	}
}

void EmulatorEngine::InitializeMachine()
{
	m_MMU->Reset();
	m_MMU->SetCartridge(m_ROMFilePath);
	m_CPU->Reset();
	
	m_State = EMULATOR_STATE_RUN;
}

void EmulatorEngine::Emulate()
{
	using namespace std::chrono;

	microseconds next_call_time = duration_cast< microseconds >(system_clock::now().time_since_epoch());
	microseconds CurrentTime;

	while (true)
	{
		Update();

		next_call_time += microseconds(16750);
		CurrentTime = duration_cast<microseconds>(system_clock::now().time_since_epoch());

		while (CurrentTime < next_call_time)
		{
			CurrentTime = duration_cast<microseconds>(system_clock::now().time_since_epoch());
		}

		if (m_State != EMULATOR_STATE_RUN)
		{
			break;
		}
	}
		
}

void EmulatorEngine::Update()
{
	int CurrentCycles = 0;
	int CyclesUsed = 0;

	byte temp_last = m_MMU->ReadMemory8(0xD804);

	while (CurrentCycles < CYCLES_PER_UPDATE)
	{
		switch (m_DebugMode)
		{
		case DEBUG_OFF:
			break;
		case DEBUG_STEP:
			WaitForDebugger();
			break;
		case DEBUG_RUN:
			if (AddressIsBreakpoint(m_CPU->GetProgramCounter()))
			{
				m_MainApp->RefreshDebugger();
				m_WaitForDebugger = true;
				WaitForDebugger();
			}
			break;
		}

		if (m_State != EMULATOR_STATE_RUN)
		{
			break;
		}

		// Interrupts are checked before fetching a new instruction
		CyclesUsed = m_CPU->ExecuteNextOp();
		CurrentCycles += CyclesUsed;

		UpdateTimers(CyclesUsed);
		m_LCD->UpdateGraphics(CyclesUsed);
		m_CPU->HandleInterrupts();

		while (m_Halted && m_State == EMULATOR_STATE_RUN)
		{
			UpdateTimers(4);
			m_LCD->UpdateGraphics(4);
			m_CPU->HandleInterrupts();
		}
		
		while (m_Stopped && m_State == EMULATOR_STATE_RUN)
		{
			m_CPU->HandleInterrupts();
		}

		if (m_State != EMULATOR_STATE_RUN)
		{
			break;
		}
	}
	m_FrameReady = true;
	m_LCD->DrawScreen();
}

bool EmulatorEngine::LCDFrameReady()
{
	return m_FrameReady;
}

void EmulatorEngine::ClearFrameReady()
{
	m_FrameReady = false;
}

void EmulatorEngine::WaitForDebugger()
{
	if (true || m_PendingInstructionListUpdate)
	{
		cout << "Recalculating instruction list" << endl;
		CalcAddrInstructionList();
	}

	// Scroll to current instruction
	int ItemAtPC = 0;
	for (int i = 0; i < 0x10000; i++)
	{
		if (m_CPU->GetReferenceProgramCounter() == m_NthInstructionAddress[i])
		{
			ItemAtPC = i;
			break;
		}
	}
	m_AssemblyListCtrl->EnsureVisible(ItemAtPC);
	
	while (m_WaitForDebugger && (m_State == EMULATOR_STATE_RUN))
	{
		wxSleep(0);
	}

	m_WaitForDebugger = (m_DebugMode == DEBUG_STEP);
}

void EmulatorEngine::RequestInterrupt(byte interrupt_bit)
{
	m_MMU->WriteMemory8(IF_REGISTER, m_MMU->ReadMemory8(IF_REGISTER) | interrupt_bit);
}

void EmulatorEngine::KeyDown(int KeyCode)
{
	bool NeedInterrupt = false;

	switch (KeyCode)
	{
	case WXK_RETURN:
		if (!m_Joypad.Start)
		{
			NeedInterrupt = true;
		}
		m_Joypad.Start = true;
		break;
	case WXK_SHIFT:
		if (!m_Joypad.Select)
		{
			NeedInterrupt = true;
		}
		m_Joypad.Select = true;
		break;
	case WXK_LEFT:
		if (!m_Joypad.Left)
		{
			NeedInterrupt = true;
		}
		m_Joypad.Left = true;
		break;
	case WXK_RIGHT:
		if (!m_Joypad.Right)
		{
			NeedInterrupt = true;
		}
		m_Joypad.Right = true;
		break;
	case WXK_UP:
		if (!m_Joypad.Up)
		{
			NeedInterrupt = true;
		}
		m_Joypad.Up = true;
		break;
	case WXK_DOWN:
		if (!m_Joypad.Down)
		{
			NeedInterrupt = true;
		}
		m_Joypad.Down = true;
		break;
	case 'A':
		if (!m_Joypad.A)
		{
			NeedInterrupt = true;
		}
		m_Joypad.A = true;
		break;
	case 'S':
		if (!m_Joypad.B)
		{
			NeedInterrupt = true;
		}
		m_Joypad.B = true;
		break;
	}
	if (NeedInterrupt)
	{
		RequestInterrupt(JOYPAD_INTERRUPT);
	}
}

void EmulatorEngine::KeyUp(int KeyCode)
{
	switch (KeyCode)
	{
	case WXK_RETURN:
		m_Joypad.Start = false;
		break;
	case WXK_SHIFT:
		m_Joypad.Select = false;
		break;
	case WXK_LEFT:
		m_Joypad.Left = false;
		break;
	case WXK_RIGHT:
		m_Joypad.Right = false;
		break;
	case WXK_UP:
		m_Joypad.Up = false;
		break;
	case WXK_DOWN:
		m_Joypad.Down = false;
		break;
	case 'A':
		m_Joypad.A = false;
		break;
	case 'S':
		m_Joypad.B = false;
		break;
	}
}

joypad_info EmulatorEngine::GetJoypad()
{
	return m_Joypad;
}

void EmulatorEngine::SetDebugMode(DebugMode mode)
{
	m_DebugMode = mode;
}

void EmulatorEngine::SetStopped()
{
	m_Stopped = true;
}

void EmulatorEngine::ClearStopped()
{
	m_Stopped = false;
}

bool EmulatorEngine::GetHalted()
{
	return m_Halted;
}

void EmulatorEngine::SetHalted()
{
	m_Halted = true;
}

void EmulatorEngine::ClearHalted()
{
	m_Halted = false;
}

void EmulatorEngine::ResetClockFrequency()
{
	timer_controller TimerControl;
	TimerControl.all = m_MMU->ReadMemory8(TAC_REGISTER);
	m_TimerCounter = TimerFrequencies[TimerControl.frequency];
}

void EmulatorEngine::UpdateTimers(int cycles)
{
	m_DivRegisterCounter += cycles;
	m_MMU->SetDIV((m_DivRegisterCounter & 0xFF00) >> 8);

	timer_controller TimerControl;
	TimerControl.all = m_MMU->ReadMemory8(TAC_REGISTER);

	if (TimerControl.enabled)
	{
		if (cycles >= m_TimerCounter)
		{
			byte LeftoverCycles = cycles - m_TimerCounter;
			m_TimerCounter = TimerFrequencies[TimerControl.frequency];// -LeftoverCycles;

			byte CurrentTIMARegisterVal = m_MMU->ReadMemory8(TIMA_REGISTER);
			byte NewTIMARegisterVal;
			if (CurrentTIMARegisterVal == 255)
			{
				NewTIMARegisterVal = m_MMU->ReadMemory8(TMA_REGISTER);
				RequestInterrupt(TIMER_INTERRUPT);
			}
			else
			{
				NewTIMARegisterVal = CurrentTIMARegisterVal + 1;
			}
			m_MMU->WriteMemory8(TIMA_REGISTER, NewTIMARegisterVal);

		}
		else
		{
			m_TimerCounter -= cycles;
		}

	}

}

Instruction EmulatorEngine::GetNthInstruction(long n, bool &IsCBPrefix, unsigned short &InstructionAddress)
{
	word DataIter = m_NthInstructionAddress[n];
	InstructionAddress = DataIter;
	opcode NextOpCode = m_MMU->ReadMemory8(DataIter++);
	Instruction NextOp;

	if (NextOpCode == 0xCB)
	{
		IsCBPrefix = true;
		NextOpCode = m_MMU->ReadMemory8(DataIter++);
		NextOp = m_CPU->GetCBInstruction(NextOpCode);
	}
	else
	{
		NextOp = m_CPU->GetInstruction(NextOpCode);
	}

	switch (NextOp.OpArgType)
	{
	case ARG_UNSIGNED_8:
		NextOp.Argument.uint8 = (char)m_MMU->ReadMemory8(DataIter++);
		break;
	case ARG_SIGNED_8:
		NextOp.Argument.sint8 = (signed char)m_MMU->ReadMemory8(DataIter++);
		break;
	case ARG_UNSIGNED_16:
		NextOp.Argument.uint16 = (unsigned short)(m_MMU->ReadMemory8(DataIter+1) << 8) + m_MMU->ReadMemory8(DataIter);
		DataIter += 2;
		break;
	}

	return NextOp;
}

void EmulatorEngine::SetState(EmulatorState State)
{
	m_State = State;
}

void EmulatorEngine::SetRomPath(wxString path)
{
	m_ROMFilePath = path;
}

int EmulatorEngine::GetRomBankNumber()
{
	return m_MMU->GetRomBankNumber();
}

void EmulatorEngine::AddBreakpointAtNthInstruction(long n)
{
	m_BreakPoints[m_NthInstructionAddress[n]] = true;
}

void EmulatorEngine::RemoveBreakpointAtNthInstruction(long n)
{
	m_BreakPoints[m_NthInstructionAddress[n]] = false;
}

bool EmulatorEngine::AddressIsBreakpoint(word Address)
{
	return m_BreakPoints[Address];
}

bool EmulatorEngine::NthInstructionIsBreakpoint(long n)
{
	unsigned short address = m_NthInstructionAddress[n];
	return AddressIsBreakpoint(address);
}

bool EmulatorEngine::NthInstructionIsPC(long n)
{
	unsigned short address = m_NthInstructionAddress[n];
	return (address == m_CPU->GetReferenceProgramCounter());
}

bool EmulatorEngine::UsingBootRom()
{
	return m_MMU->GetBootRomEnabled();
}

void EmulatorEngine::Step()
{
	m_DebugMode = DEBUG_STEP;
	m_WaitForDebugger = false;
}

void EmulatorEngine::Run()
{
	m_WaitForDebugger = false;
	m_DebugMode = DEBUG_RUN;
}

unsigned short EmulatorEngine::GetRegisterValue(RegisterID RegisterID)
{
	unsigned short RegisterValue;

	switch (RegisterID)
	{
	case REGISTER_AF:
		RegisterValue = m_CPU->GetRegisterValue(REGISTER_AF);
		break;
	case REGISTER_BC:
		RegisterValue = m_CPU->GetRegisterValue(REGISTER_BC);
		break;
	case REGISTER_DE:
		RegisterValue = m_CPU->GetRegisterValue(REGISTER_DE);
		break;
	case REGISTER_HL:
		RegisterValue = m_CPU->GetRegisterValue(REGISTER_HL);
		break;
	case REGISTER_SP:
		RegisterValue = m_CPU->m_StackPointer;
		break;
	case REGISTER_PC:
		RegisterValue = m_CPU->GetReferenceProgramCounter();
		break;
	case REGISTER_IE:
		RegisterValue = m_MMU->ReadMemory8(IE_REGISTER);
		break;
	case REGISTER_IF:
		RegisterValue = m_MMU->ReadMemory8(IF_REGISTER);
		break;
	case REGISTER_IME:
		RegisterValue = m_CPU->GetRegisterValue(REGISTER_IME);
		break;
	case REGISTER_IMA:
		RegisterValue = m_CPU->GetRegisterValue(REGISTER_IMA);
		break;
	case REGISTER_LCDC:
		RegisterValue = m_MMU->ReadMemory8(LCDC_REGISTER);
		break;
	case REGISTER_STAT:
		RegisterValue = m_MMU->ReadMemory8(STAT_REGISTER);
		break;
	case REGISTER_LY:
		RegisterValue = m_MMU->ReadMemory8(LY_REGISTER);
		break;
	case REGISTER_ROM:
		RegisterValue = m_MMU->GetRomBankNumber();
		break;
	case REGISTER_DIV:
		RegisterValue = m_MMU->ReadMemory8(DIV_REGISTER);
		break;
	case REGISTER_TIMA:
		RegisterValue = m_MMU->ReadMemory8(TIMA_REGISTER);
		break;
	}

	return RegisterValue;
}

unsigned short EmulatorEngine::GetProgramCounter()
{
	return m_CPU->GetReferenceProgramCounter();
}

DebugMode EmulatorEngine::GetDebugMode()
{
	return m_DebugMode;
}

RomAddressDataType EmulatorEngine::GetRomAddressDataType(unsigned short Address)
{
	RomAddressDataType Type;

	if (Address < 0x0104)
	{
		Type = ROM_ASM;
	}
	else if (Address < 0x0134)
	{
		Type = ROM_DATA_16;
	}
	else if (Address < 0x0143)
	{
		Type = ROM_DATA_15;
	}
	else if (Address == 0x0143)
	{
		Type = ROM_DATA_1;
	}
	else if (Address == 0x0144)
	{
		Type = ROM_DATA_2;
	}
	else if (Address < 0x014E)
	{
		Type = ROM_DATA_1;
	}
	else if (Address == 0x014E)
	{
		Type = ROM_DATA_2;
	}
	else if (Address < 0x8000)
	{
		Type = ROM_ASM;
	}
	else if (Address < 0xA000)
	{
		Type = ROM_DATA_16;
	}
	else if (Address < 0xFE00)
	{
		Type = ROM_ASM;
	}
	else if (Address < 0xFF00)
	{
		Type = ROM_DATA_16;
	}
	else if (Address == 0xFF08)
	{
		Type = ROM_DATA_7;
	}
	else if (Address == 0xFF27)
	{
		Type = ROM_DATA_9;
	}
	else if (Address == 0xFF30)
	{
		Type = ROM_DATA_16;
	}
	else if (Address == 0xFF57)
	{
		Type = ROM_DATA_16;
	}
	else if (Address == 0xFF6C)
	{
		Type = ROM_DATA_4;
	}
	else if (Address == 0xFF71)
	{
		Type = ROM_DATA_15;
	}
	else
	{
		Type = ROM_ASM;
	}

	return Type;
}

wxString EmulatorEngine::GetAddressComment(unsigned short Address)
{
	wxString Comment = "";

	switch (Address)
	{
	case VBLANK_INT_ADDR:
		Comment = "VBLANK Interrupt";
		break;
	case LCD_STAT_INT_ADDR:
		Comment = "LCD Interrupt";
		break;
	case TIMER_INT_ADDR:
		Comment = "Timer Interrupt";
		break;
	case SERIAL_INT_ADDR:
		Comment = "Serial Interrupt";
		break;
	case JOYPAD_INT_ADDR:
		Comment = "Joypad Interrupt";
		break;
	case CARTRIDGE_TITLE_ADDR:
		Comment = "Title";
		break;
	case CARTRIDGE_CGB_FLAG_ADDR:
		if (m_MMU->ReadMemory8(CARTRIDGE_CGB_FLAG_ADDR) == 0x00)
		{
			Comment = "DMG: Classic Gameboy";
		}
		else
		{
			Comment = "CGB Flag";
		}

		break;
	case CARTRIDGE_NEW_LICENSEE_ADDR:
		Comment = "New License";
		break;
	case CARTRIDGE_SGB_FLAG_ADDR:
		if (m_MMU->ReadMemory8(CARTRIDGE_SGB_FLAG_ADDR) == 0x00)
		{
			Comment = "SGB Flag: No SGB Support";
		}
		else
		{
			Comment = "SGB Flag: SGB Capable";
		}
		break;
	case CARTRIDGE_TYPE_ADDR:
	{
		byte CartType = m_MMU->ReadMemory8(CARTRIDGE_TYPE_ADDR);
		switch (CartType)
		{
		case 0xFC:
			Comment = "Cartridge Type: POCKET CAMERA";
			break;
		case 0xFD:
			Comment = "Cartridge Type: BANDAI TAMA5";
			break;
		case 0xFE:
			Comment = "Cartridge Type: HuC3";
			break;
		case 0xFF:
			Comment = "Cartridge Type: HuC1+RAM+BATTERY";
			break;
		default:
			Comment = wxString::Format(wxT("Cartridge Type: %s"), CartridgeTypes[CartType]);
			break;
		}

	}
	break;
	case CARTRIDGE_ROM_SIZE_ADDR:
		switch (m_MMU->ReadMemory8(CARTRIDGE_ROM_SIZE_ADDR))
		{
		case 0x00:
			Comment = "Rom Size: 32KByte (no ROM banking)";
			break;
		case 0x01:
			Comment = "Rom Size: 64KByte (4 banks)";
			break;
		case 0x02:
			Comment = "Rom Size: 128KByte (8 banks)";
			break;
		case 0x03:
			Comment = "Rom Size: 256KByte (16 banks)";
			break;
		case 0x04:
			Comment = "Rom Size: 512KByte (32 banks)";
			break;
		case 0x05:
			Comment = "Rom Size: 1MByte (64 banks)";
			break;
		case 0x06:
			Comment = "Rom Size: 2MByte (128 banks)";
			break;
		case 0x07:
			Comment = "Rom Size: 4MByte (256 banks)";
			break;
		case 0x08:
			Comment = "Rom Size: 8MByte (512 banks)";
			break;
		case 0x52:
			Comment = "Rom Size: 1.1MByte (72 banks)";
			break;
		case 0x53:
			Comment = "Rom Size: 1.2MByte (80 banks)";
			break;
		case 0x54:
			Comment = "Rom Size: 1.5MByte (96 banks)";
			break;
		}
		break;
	case CARTRIDGE_RAM_SIZE_ADDR:
		switch (m_MMU->ReadMemory8(CARTRIDGE_RAM_SIZE_ADDR))
		{
		case 0x00:
			Comment = "Ram Size: None";
			break;
		case 0x01:
			Comment = "Ram Size: 2 KBytes";
			break;
		case 0x02:
			Comment = "Ram Size: 8 KBytes";
			break;
		case 0x03:
			Comment = "Ram Size: 32 KBytes";
			break;
		case 0x04:
			Comment = "Ram Size: 128 KBytes";
			break;
		case 0x05:
			Comment = "Ram Size: 64 KBytes";
			break;
		}
		break;
	case CARTRIDGE_DEST_CODE_ADDR:
		if (m_MMU->ReadMemory8(CARTRIDGE_DEST_CODE_ADDR))
		{
			Comment = "Destination Code: Non-Japanese";
		}
		else
		{
			Comment = "Destination Code: Japanese";
		}
		break;
	case CARTRIDGE_OLD_LICENSEE_ADDR:
		Comment = "Old Licensee Code";
		break;
	case CARTRIDGE_MASK_ROM_ADDR:
		Comment = "Mask ROM Version number";
		break;
	case CARTRIDGE_HEADER_CHECKSUM_ADDR:
		Comment = "Header Checksum";
		break;
	case CARTRIDGE_GLOBAL_CHECKSUM_ADDR:
		Comment = "Global Checksum";
		break;
	case DSROM_REGISTER:
		Comment = "Disable Boot Rom";
		break;
	}

	return Comment;
}

wxString EmulatorEngine::GetCartridgeTitle()
{
	char Title[18] = { 0 };
	char CurrentChar;

	Title[0] = '"';
	Title[16] = '"';
	for (int i = 0; i < 15; i++)
	{
		CurrentChar = m_MMU->ReadMemory8(CARTRIDGE_TITLE_ADDR + i);
		if (CurrentChar == '\0')
		{
			CurrentChar = ' ';
		}
		Title[i+1] = CurrentChar;
	}

	return wxString(Title);
}

void EmulatorEngine::CalcAddrInstructionList()
{
	int InstructionIter = 0;
	int InstructionCounter = 0;
	opcode NextOpCode;
	Instruction NextOp;
	int OpAddress;
	RomAddressDataType AddressDataType;

	while (InstructionIter < 0xFFFF)
	{
		OpAddress = InstructionIter;
		m_NthInstructionAddress[InstructionCounter++] = OpAddress;
		AddressDataType = GetRomAddressDataType(InstructionIter);
		switch (AddressDataType)
		{
		case ROM_ASM:
			NextOpCode = m_MMU->ReadMemory8(InstructionIter++);

			if (NextOpCode == 0xCB)
			{
				NextOpCode = m_MMU->ReadMemory8(InstructionIter++);
				NextOp = m_CPU->GetCBInstruction(NextOpCode);
			}
			else
			{
				NextOp = m_CPU->GetInstruction(NextOpCode);
			}

			switch (NextOp.OpArgType)
			{
			case ARG_UNSIGNED_8:
			case ARG_SIGNED_8:
				InstructionIter++;
				break;
			case ARG_UNSIGNED_16:
				InstructionIter += 2;
				break;
			}

			break;
		case ROM_DATA_1:
			InstructionIter += 1;
			break;
		case ROM_DATA_2:
			InstructionIter += 2;
			break;
		case ROM_DATA_3:
			InstructionIter += 3;
			break;
		case ROM_DATA_4:
			InstructionIter += 4;
			break;
		case ROM_DATA_7:
			InstructionIter += 7;
			break;
		case ROM_DATA_9:
			InstructionIter += 9;
			break;
		case ROM_DATA_15:
			InstructionIter += 15;
			break;
		case ROM_DATA_16:
			InstructionIter += 16;
			break;
		}
	}

	m_PendingInstructionListUpdate = false;

	m_AssemblyListCtrl->SetItemCount(InstructionCounter);
}

GBMMU *EmulatorEngine::GetMMU()
{
	return m_MMU;
}

GBLCD *EmulatorEngine::GetLCD()
{
	return m_LCD;
}

void EmulatorEngine::SetPendingInstructionListUpdate()
{
	m_PendingInstructionListUpdate = true;
}

void EmulatorEngine::OpenSaveState(wxString FilePath)
{
	SaveState State(FilePath);
	wxFileName RomFileName(FilePath);
	state_entry RomFileEntry = State.Get("romfile");
	RomFileName.SetFullName(wxString((char*)RomFileEntry.Buffer, (char*)RomFileEntry.Buffer + RomFileEntry.Size));
	
	if (!wxFileExists(RomFileName.GetFullPath()))
	{
		// If the rom file associated with the save state is not in the same
		// directory, request it from the user
		wxFileDialog *OpenRomDialog = new wxFileDialog(m_MainApp, _("Locate ROM for save state"), wxEmptyString, wxEmptyString, "GB files (*.gb)|*.gb", wxFD_OPEN, wxDefaultPosition);
		int ModalStatus = OpenRomDialog->ShowModal();
		if (ModalStatus == wxID_OK)
		{
			RomFileName.Assign(OpenRomDialog->GetPath());
		}
		OpenRomDialog->Destroy();

		if (ModalStatus != wxID_OK)
		{
			return;
		}
	}

	SetRomPath(RomFileName.GetFullPath());
	InitializeMachine();
	m_CPU->LoadState(State);
	m_MMU->LoadState(State);
}

void EmulatorEngine::CaptureSaveState()
{
}

EmulatorEngine::~EmulatorEngine()
{
	delete m_MMU;
	delete m_CPU;
	delete m_LCD;
}
