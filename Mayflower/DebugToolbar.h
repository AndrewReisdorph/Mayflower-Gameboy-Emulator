#pragma once
#include <wx/wx.h>
#include "images.h"
#include "MayflowerWindow.h"
#include "EmulatorEngine.h"

class MayflowerWindow;

class DebugToolbar :public wxPanel
{
private:
	MayflowerWindow *m_MainApp;
	EmulatorEngine *m_Emulator;

	void InitUI();
	void OnDebugRunButtonClick(wxCommandEvent& event);
	void OnDebugStepButtonClick(wxCommandEvent& event);
	void OnDebugStepoverButtonClick(wxCommandEvent& event);
	void OnDebugStepoutButtonClick(wxCommandEvent& event);
	void OnSearchButtonClick(wxCommandEvent& event);
	void OnScreenshotButtonClick(wxCommandEvent& event);
	void OnIOMapButtonClick(wxCommandEvent& event);
	void OnVRAMButtonClick(wxCommandEvent& event);
	void OnOpenRomButtonClick(wxCommandEvent& event);
	void OnSaveStateButtonClick(wxCommandEvent& event);
	void OnLoadStateButtonClick(wxCommandEvent& event);
	void OnRestartButtonClick(wxCommandEvent& event);
	void OnBreakpointsButtonClick(wxCommandEvent& event);

public:
	void SetEmulator(EmulatorEngine *Emulator);
	DebugToolbar(wxFrame *parent, MayflowerWindow *main_app);
	~DebugToolbar();
};

