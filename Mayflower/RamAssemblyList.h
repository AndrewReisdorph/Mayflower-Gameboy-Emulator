#pragma once
#include <wx/wx.h>
#include <wx/listctrl.h>
#include "images.h"
#include "EmulatorEngine.h"
#include "main.h"

#define BLANK_IMG         -1
#define PC_IMG             0
#define BREAKPOINT_IMG     1
#define BREAKPOINT_PC_IMG  2

#define COL_PC       0
#define COL_ADDRESS  1
#define COL_BYTES    2
#define COL_ASSEMBLY 3
#define COL_COMMENT  4

struct Instruction;
class EmulatorEngine;

class RamAssemblyList : public wxListCtrl
{
private:
	wxListItemAttr *m_ItemAttr;
	void InitUI();
	wxImageList *m_ImageList;
	wxString GetAddressHeading(int Address) const;
	EmulatorEngine *m_Emulator;
	wxListItemAttr m_ListItemAttr;

public:
	void SetEmulator(EmulatorEngine *Emulator);
	void UpdateProgramCounter(int OldPC, int NewPC);
	void ModifyBreakpoint(int BreakPointRow, bool SetBreakpoint);
	void OnItemActivate(wxListEvent& event);
	void Step();
	
	wxString OnGetItemText(long item, long column) const;
	int OnGetItemImage(long item) const;
	int OnGetItemColumnImage(long item, long column) const;
	wxListItemAttr* OnGetItemAttr(long item) const;

	RamAssemblyList(wxFrame *parent);
	~RamAssemblyList();
};

