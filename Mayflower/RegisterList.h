#pragma once
#include <wx/wx.h>
#include <wx/listctrl.h>
#include "EmulatorEngine.h"
#include "RegisterView.h"

#define COL_NAME_A  0
#define COL_VALUE_A 1
#define COL_NAME_B  2
#define COL_VALUE_B 3

#define ROW_REG_AF 0
#define ROW_REG_BC 1
#define ROW_REG_DE 2
#define ROW_REG_HL 3
#define ROW_REG_SP 4
#define ROW_REG_PC 5
#define ROW_REG_IME 6
#define ROW_REG_IMA 7

#define ROW_REG_LCDC 0
#define ROW_REG_STAT 1
#define ROW_REG_LY   2
#define ROW_REG_IE   3
#define ROW_REG_IF   4
#define ROW_REG_ROM  5
#define ROW_REG_DIV  6
#define ROW_REG_TIMA 7

class EmulatorEngine;
class RegisterView;

class RegisterList : public wxListCtrl
{
private:
	RegisterView *m_RegisterView;
	EmulatorEngine *m_Emulator = nullptr;
	wxString OnGetItemText(long item, long column) const;
public:
	void SetEmulator(EmulatorEngine *Emulator);
	RegisterList(RegisterView *parent);
	~RegisterList();
};

