#pragma once
#include <wx/wx.h>
#include "MemoryViewListCtrl.h"
#include "EmulatorEngine.h"
#include "GameboyScreenPanel.h"
#include "RamAssemblyList.h"
#include "RegisterView.h"
#include "MemoryViewListCtrl.h"
#include "DebugToolbar.h"
#include "IOMap.h"

class GameboyScreenPanel;
class EmulatorEngine;
class RamAssemblyList;
class RegisterView;
class MemoryViewListCtrl;
class DebugToolbar;
class IOMap;

class MayflowerWindow : public wxFrame
{
private:
	wxMenuBar *m_MenuBar;
	wxMenu *m_FileMenu;
	wxMenu *m_HelpMenu;
	wxMenu *m_DebugMenu;
	wxMenu *m_WindowMenu;
	EmulatorEngine *m_Emulator;
	GameboyScreenPanel *m_ScreenPanel;
	RamAssemblyList *m_RamAssemblyList;
	MemoryViewListCtrl *m_MemoryViewListCtrl;
	RegisterView *m_RegisterView;
	DebugToolbar *m_DebugControl;
	IOMap *m_IOMapWindow;

	void InitUI();
	void OnKeyDown(wxKeyEvent& event);
	void OnKeyUp(wxKeyEvent& event);

public:
	void OpenSaveState();
	void ShowIOMap();
	void RefreshGameboyScreen();
	void RefreshScreenPanel();
	MayflowerWindow();
	~MayflowerWindow();
	void OpenROM(wxCommandEvent& event);
	void OnCloseWindow(wxCloseEvent& event);
	void RefreshDebugger();

	DECLARE_EVENT_TABLE()
};