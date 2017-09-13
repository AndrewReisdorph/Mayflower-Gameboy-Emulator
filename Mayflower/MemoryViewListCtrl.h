#pragma once
#include <wx/wx.h>
#include <wx/listctrl.h>
#include "EmulatorEngine.h"

#define COL_ADDRESS 0
#define COL_VALUE   1

class EmulatorEngine;

class MemoryViewListCtrl: public wxListCtrl
{
private:

	EmulatorEngine *m_Emulator;
	wxString OnGetItemText(long item, long column) const;

public:
	void SetEmulator(EmulatorEngine *Emulator);
	
	MemoryViewListCtrl(wxFrame *parent);
	~MemoryViewListCtrl();
};

