#pragma once
#include <set>
#include <wx/wx.h>
#include <wx/filename.h>
#include <wx/longlong.h>
#include "GameboyScreenPanel.h"
#include "RamAssemblyList.h"
#include "MayflowerWindow.h"
#include "GBCPU.h"
#include "GBMMU.h"
#include "GBLCD.h"
#include "GBTypes.h"
#include "SaveState.h"


#define CYCLES_PER_UPDATE   69905
#define CLOCKSPEED        4194304

// Misc
#define M_ResetRegBit(reg, bit) {reg &= ~(bit);}
#define M_SetRegBit(reg, bit) {reg |= bit;}
#define M_ResetBitAtHL(bit) m_MMU->WriteMemory8(M_RegisterHL, m_MMU->ReadMemory8(M_RegisterHL) & (~(bit)))
#define M_SetBitAtHL(bit) m_MMU->WriteMemory8(M_RegisterHL, m_MMU->ReadMemory8(M_RegisterHL) | bit)
#define M_BitIsSet(val, bit) (val & (1<<bit))
#define M_ServiceInterrupt(irequested, ienabled, interrupt) ((ienabled & interrupt) && (irequested & interrupt))


// Cartridge
#define CARTRIDGE_TITLE_ADDR           0x0134
#define CARTRIDGE_CGB_FLAG_ADDR        0x0143
#define CARTRIDGE_NEW_LICENSEE_ADDR    0x0144
#define CARTRIDGE_SGB_FLAG_ADDR        0x0146
#define CARTRIDGE_TYPE_ADDR            0x0147
#define CARTRIDGE_ROM_SIZE_ADDR        0x0148
#define CARTRIDGE_RAM_SIZE_ADDR        0x0149
#define CARTRIDGE_DEST_CODE_ADDR       0x014A
#define CARTRIDGE_OLD_LICENSEE_ADDR    0x014B
#define CARTRIDGE_MASK_ROM_ADDR        0x014C
#define CARTRIDGE_HEADER_CHECKSUM_ADDR 0x014D
#define CARTRIDGE_GLOBAL_CHECKSUM_ADDR 0x014E


#define SPRITE_DATA    0x8000

enum EmulatorState
{
	EMULATOR_STATE_IDLE,
	EMULATOR_STATE_INIT,
	EMULATOR_STATE_RUN,
	EMULATOR_STATE_QUIT
};

enum JoypadMasks
{
	JOYPAD_A = BIT_0,
	JOYPAD_RIGHT = BIT_0,
	JOYPAD_B = BIT_1,
	JOYPAD_LEFT = BIT_1,
	JOYPAD_SELECT = BIT_2,
	JOYPAD_UP = BIT_2,
	JOYPAD_START = BIT_3,
	JOYPAD_DOWN = BIT_3
};

enum DebugMode
{
	DEBUG_OFF,
	DEBUG_STEP,
	DEBUG_RUN
};

enum RomAddressDataType
{
	ROM_ASM,
	ROM_DATA_1,
	ROM_DATA_2,
	ROM_DATA_3,
	ROM_DATA_4,
	ROM_DATA_7,
	ROM_DATA_9,
	ROM_DATA_15,
	ROM_DATA_16
};

typedef union timer_controller
{
	struct
	{
		byte frequency : 2;
		byte enabled : 1;
		byte reserved : 5;
	};
	byte all;
}timer_controller;

typedef struct joypad_info
{
	bool Up;
	bool Down;
	bool Left;
	bool Right;
	bool A;
	bool B;
	bool Start;
	bool Select;
}joypad_info;

class MayflowerWindow;
class GameboyScreenPanel;
class RamAssemblyList;
class GBCPU;
class GBMMU;
class GBLCD;

class EmulatorEngine
{
private:
	GBCPU *m_CPU;
	GBMMU *m_MMU;
	GBLCD *m_LCD;

	EmulatorState m_State = EMULATOR_STATE_IDLE;

	bool m_Stopped = false;
	bool m_Halted = false;

	// Timing
	unsigned short m_DivRegisterCounter = 0;
	word m_TimerCounter = 1024;

	// Buffers
	bool m_BreakPoints[0x10000] = { false };
	word m_NthInstructionAddress[0xFFFF];

	// UI Elements
	MayflowerWindow *m_MainApp;
	RamAssemblyList *m_AssemblyListCtrl;
	GameboyScreenPanel *m_ScreenPanel;

	// Debug
	bool m_TraceOn = false;
	DebugMode m_DebugMode = DEBUG_STEP;
	bool m_WaitForDebugger = true;
	bool m_PendingInstructionListUpdate = false;

	// Misc
	joypad_info m_Joypad = { false };
	int m_MiscCycles = 0;
	int m_NumFramesRendered = 0;
	bool m_FrameReady = false;
	int m_ScanLineCounter = 0;
	wxString m_ROMFilePath;

	void Emulate();
	void Update();

	void CalcAddrInstructionList();
	bool AddressIsBreakpoint(word Address);
	void WaitForDebugger();
	void UpdateTimers(int cycles);

public:
	void CaptureSaveState();
	void SetState(EmulatorState State);
	void EmulatorStateMachine();
	void InitializeMachine();
	bool LCDFrameReady();
	void ClearFrameReady();
	joypad_info GetJoypad();
	void KeyDown(int KeyCode);
	void KeyUp(int KeyCode);
	bool GetHalted();
	void ClearHalted();
	void SetHalted();
	void SetStopped();
	void ClearStopped();
	void RequestInterrupt(byte interrupt_bit);
	DebugMode GetDebugMode();
	void SetDebugMode(DebugMode mode);
	void RemoveBreakpointAtNthInstruction(long n);
	void AddBreakpointAtNthInstruction(long n);
	bool NthInstructionIsBreakpoint(long n);
	void SetRomPath(wxString path);
	void ResetClockFrequency();
	void Step();
	void Run();
	int  GetRomBankNumber();
	word GetRegisterValue(RegisterID RegisterID);
	word GetProgramCounter();
	GBMMU *GetMMU();
	GBLCD *GetLCD();
	bool UsingBootRom();
	wxString GetCartridgeTitle();
	wxString GetAddressComment(unsigned short Address);
	RomAddressDataType GetRomAddressDataType(unsigned short Address);
	Instruction GetNthInstruction(long n, bool &IsCBPrefix, unsigned short &InstructionAddress);
	bool NthInstructionIsPC(long n);
	void SetPendingInstructionListUpdate();
	void OpenSaveState(wxString FilePath);
	EmulatorEngine(GameboyScreenPanel *ScreenPanel, RamAssemblyList *AssemblyListCtrl, MayflowerWindow *App);
	~EmulatorEngine();
};


static const int TimerFrequencies[4] = { 1024, 16, 64, 256 };

static const wxString CartridgeTypes[] = {
	"ROM ONLY",                      // 00
	"MBC1",                          // 01
	"MBC1+RAM",                      // 02
	"MBC1+RAM+BATTERY",              // 03
	"-",                             // 04
	"MBC2",                          // 05
	"MBC2+BATTERY",                  // 06
	"-",                             // 07
	"ROM+RAM",                       // 08
	"ROM+RAM+BATTERY",               // 09
	"-",                             // 0A
	"MMM01",                         // 0B
	"MMM01+RAM",                     // 0C
	"MMM01+RAM+BATTERY",             // 0D
	"-",                             // 0E
	"MBC3+TIMER+BATTERY",            // 0F
	"MBC3+TIMER+RAM+BATTERY",        // 10
	"MBC3",                          // 11
	"MBC3+RAM",                      // 12
	"MBC3+RAM+BATTERY",              // 13
	"-",                             // 14
	"-",                             // 15
	"-",                             // 16
	"-",                             // 17
	"-",                             // 18
	"MBC5",                          // 19
	"MBC5+RAM",                      // 1A
	"MBC5+RAM+BATTERY",              // 1B
	"MBC5+RUMBLE",                   // 1C
	"MBC5+RUMBLE+RAM",               // 1D
	"MBC5+RUMBLE+RAM+BATTERY",       // 1E
	"-",                             // 1F
	"MBC6",                          // 20
	"-",                             // 21
	"MBC7+SENSOR+RUMBLE+RAM+BATTERY" // 22
};