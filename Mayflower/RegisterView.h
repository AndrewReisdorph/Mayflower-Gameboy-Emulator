#pragma once
#include <wx/wx.h>
#include <wx/checkbox.h>
#include <wx/listctrl.h>
#include "RegisterList.h"
#include "EmulatorEngine.h"

class EmulatorEngine;
class RegisterList;

class RegisterView :public wxPanel
{
private:
	wxCheckBox *m_FlagZCheckbox;
	wxCheckBox *m_FlagNCheckbox;
	wxCheckBox *m_FlagHCheckbox;
	wxCheckBox *m_FlagCCheckbox;
	RegisterList *m_RegisterList;
	EmulatorEngine *m_Emulator = nullptr;
	void InitUI();

public:
	void UpdateFlagCheckboxes(unsigned short FlagWord);
	void SetEmulator(EmulatorEngine *Emulator);
	RegisterView(wxFrame *parent);
	~RegisterView();
};

