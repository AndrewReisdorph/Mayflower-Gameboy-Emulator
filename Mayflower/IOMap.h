#pragma once
#include <wx/wx.h>
#include "EmulatorEngine.h"
#include "GBTypes.h"
#include "GBMMU.h"
#include "images.h"

#define NUM_IO_ENTRIES   47
#define MAX_LCD_ENTRY    12
#define MAX_MISC_ENTRY   24
#define MAX_SOUND1_ENTRY 29
#define MAX_SOUND2_ENTRY 34
#define MAX_SOUND3_ENTRY 39
#define MAX_SOUND4_ENTRY 44

typedef struct io_entry
{
	wxTextCtrl *TextCtrl;
	wxString Label;
	word Address;
}io_entry;

typedef struct lcdstat_widgets
{
	wxCheckBox *LYCInt;
	wxCheckBox *OAMInt;
	wxCheckBox *VBlankInt;
	wxCheckBox *HBlankInt;
	wxCheckBox *LyLyc;
	wxTextCtrl *Mode;
}lcdstat_widgets;

typedef struct lcdcontrol_checkboxes
{
	wxCheckBox *LCDOff;
	wxCheckBox *WinAddr;
	wxCheckBox *WinOff;
	wxCheckBox *ChrAddr;
	wxCheckBox *BgAddr;
	wxCheckBox *ObjSize;
	wxCheckBox *ObjOff;
	wxCheckBox *BGOn;

}lcdcontrol_checkboxes;

typedef struct ieif_widgets
{
	wxCheckBox *IF_VblankCheckbox;
	wxCheckBox *IE_VblankCheckbox;
	wxTextCtrl *VblankTextCtrl;
	wxCheckBox *IF_LCDCheckbox;
	wxCheckBox *IE_LCDCheckbox;
	wxTextCtrl *LCDTextCtrl;
	wxCheckBox *IF_TimerCheckbox;
	wxCheckBox *IE_TimerCheckbox;
	wxTextCtrl *TimerTextCtrl;
	wxCheckBox *IF_SerialCheckbox;
	wxCheckBox *IE_SerialCheckbox;
	wxTextCtrl *SerialTextCtrl;
	wxCheckBox *IF_JoypadCheckbox;
	wxCheckBox *IE_JoypadCheckbox;
	wxTextCtrl *JoypadTextCtrl;

}ieif_widgets;

class IOMap: public wxDialog
{
private:
	EmulatorEngine *m_Emulator = nullptr;
	lcdstat_widgets m_LCDStatWidgets;
	lcdcontrol_checkboxes m_LCDCtrlCheckboxes;
	ieif_widgets m_IEIFWidgets;
	wxTextCtrl *m_MBCRomTextCtrl;
	wxTextCtrl *m_MBCRamTextCtrl;
	wxTextCtrl *m_WavePatternA;
	wxTextCtrl *m_WavePatternB;
	wxTextCtrl *m_WavePatternC;
	wxTextCtrl *m_WavePatternD;

	void InitUI();
	void OnRefreshButtonClick(wxCommandEvent& event);

public:
	void SetEmulator(EmulatorEngine *Emulator);
	void RefreshValues();
	IOMap(wxFrame *parent);
	~IOMap();
};


