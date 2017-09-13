#pragma once
#include <wx/wx.h>
#include "MemoryViewListCtrl.h"
#include "EmulatorEngine.h"
#include "GameboyScreenPanel.h"
#include "RamAssemblyList.h"
#include "RegisterView.h"
#include "MemoryViewListCtrl.h"

class GameboyScreenPanel;
class EmulatorEngine;
class RamAssemblyList;
class RegisterView;
class MemoryViewListCtrl;

class MayflowerWindow : public wxFrame
{
private:
	wxMenuBar *m_MenuBar;
	wxMenu *FileMenu;
	EmulatorEngine *m_Emulator;
	GameboyScreenPanel *m_ScreenPanel;
	RamAssemblyList *m_RamAssemblyList;
	MemoryViewListCtrl *m_MemoryViewListCtrl;
	RegisterView *m_RegisterView;

	void InitUI();
	void OnRunButtonPress(wxCommandEvent& event);
	void OnStepButtonPress(wxCommandEvent& event);

public:
	void RefreshGameboyScreen();
	void RefreshScreenPanel();
	MayflowerWindow();
	void OpenROM(wxCommandEvent& event);
	void RefreshDebugger();

	DECLARE_EVENT_TABLE()
};