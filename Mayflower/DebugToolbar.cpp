#include "DebugToolbar.h"



DebugToolbar::DebugToolbar(wxFrame *parent, MayflowerWindow *main_app) :wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize)
{
	m_MainApp = main_app;
	InitUI();
}

void DebugToolbar::SetEmulator(EmulatorEngine *Emulator)
{
	m_Emulator = Emulator;
}

void DebugToolbar::InitUI()
{
	wxBoxSizer *MainHSizer = new wxBoxSizer(wxHORIZONTAL);

	wxBitmap RestartBitmap = wxBITMAP_PNG_FROM_DATA(restart);
	wxBitmap SaveBitmap = wxBITMAP_PNG_FROM_DATA(disk);
	wxBitmap LoadStateBitmap = wxBITMAP_PNG_FROM_DATA(memory);
	wxBitmap FolderBitmap = wxBITMAP_PNG_FROM_DATA(open);
	wxBitmap DebugRunBitmap = wxBITMAP_PNG_FROM_DATA(debug_run);
	wxBitmap DebugStepBitmap = wxBITMAP_PNG_FROM_DATA(debug_step);
	wxBitmap DebugStepoverBitmap = wxBITMAP_PNG_FROM_DATA(debug_stepover);
	wxBitmap DebugStepoutBitmap = wxBITMAP_PNG_FROM_DATA(debug_stepout);
	wxBitmap SearchBitmap = wxBITMAP_PNG_FROM_DATA(binocular);
	wxBitmap ScreenshotBitmap = wxBITMAP_PNG_FROM_DATA(camera);
	wxBitmap IOMapBitmap = wxBITMAP_PNG_FROM_DATA(iomap);
	wxBitmap VRAMInspectorBitmap = wxBITMAP_PNG_FROM_DATA(vram);
	wxBitmap BreakpointsBitmap = wxBITMAP_PNG_FROM_DATA(breakpoints);

	wxBitmapButton *DebugRunButton = new wxBitmapButton(this, wxID_ANY, DebugRunBitmap);
	DebugRunButton->Bind(wxEVT_BUTTON, &DebugToolbar::OnDebugRunButtonClick, this);
	DebugRunButton->SetToolTip("Run");
	wxBitmapButton *DebugStepButton = new wxBitmapButton(this, wxID_ANY, DebugStepBitmap);
	DebugStepButton->Bind(wxEVT_BUTTON, &DebugToolbar::OnDebugStepButtonClick, this);
	DebugStepButton->SetToolTip("Step Into");
	wxBitmapButton *DebugStepoverButton = new wxBitmapButton(this, wxID_ANY, DebugStepoverBitmap);
	DebugStepoverButton->Bind(wxEVT_BUTTON, &DebugToolbar::OnDebugStepoverButtonClick, this);
	DebugStepoverButton->SetToolTip("Step Over");
	wxBitmapButton *DebugStepoutButton = new wxBitmapButton(this, wxID_ANY, DebugStepoutBitmap);
	DebugStepoutButton->Bind(wxEVT_BUTTON, &DebugToolbar::OnDebugStepoutButtonClick, this);
	DebugStepoutButton->SetToolTip("Step Out");
	wxBitmapButton *SearchButton = new wxBitmapButton(this, wxID_ANY, SearchBitmap);
	SearchButton->Bind(wxEVT_BUTTON, &DebugToolbar::OnSearchButtonClick, this);
	SearchButton->SetToolTip("Search");
	wxBitmapButton *ScreenshotButton = new wxBitmapButton(this, wxID_ANY, ScreenshotBitmap);
	ScreenshotButton->Bind(wxEVT_BUTTON, &DebugToolbar::OnScreenshotButtonClick, this);
	ScreenshotButton->SetToolTip("Screenshot");
	wxBitmapButton *IOMapButton = new wxBitmapButton(this, wxID_ANY, IOMapBitmap);
	IOMapButton->Bind(wxEVT_BUTTON, &DebugToolbar::OnIOMapButtonClick, this);
	IOMapButton->SetToolTip("IO Map");
	wxBitmapButton *VRAMInspectorButton = new wxBitmapButton(this, wxID_ANY, VRAMInspectorBitmap);
	VRAMInspectorButton->Bind(wxEVT_BUTTON, &DebugToolbar::OnVRAMButtonClick, this);
	VRAMInspectorButton->SetToolTip("Inspect VRAM");
	wxBitmapButton *OpenRomButton = new wxBitmapButton(this, wxID_ANY, FolderBitmap);
	OpenRomButton->Bind(wxEVT_BUTTON, &DebugToolbar::OnOpenRomButtonClick, this);
	OpenRomButton->SetToolTip("Open ROM");
	wxBitmapButton *SaveStateButton = new wxBitmapButton(this, wxID_ANY, SaveBitmap);
	SaveStateButton->Bind(wxEVT_BUTTON, &DebugToolbar::OnSaveStateButtonClick, this);
	SaveStateButton->SetToolTip("Save State");
	wxBitmapButton *LoadStateButton = new wxBitmapButton(this, wxID_ANY, LoadStateBitmap);
	LoadStateButton->Bind(wxEVT_BUTTON, &DebugToolbar::OnLoadStateButtonClick, this);
	LoadStateButton->SetToolTip("Load State");
	wxBitmapButton *RestartButton = new wxBitmapButton(this, wxID_ANY, RestartBitmap);
	RestartButton->Bind(wxEVT_BUTTON, &DebugToolbar::OnRestartButtonClick, this);
	RestartButton->SetToolTip("Reload ROM");
	wxBitmapButton *BreakpointsButton = new wxBitmapButton(this, wxID_ANY, BreakpointsBitmap);
	BreakpointsButton->Bind(wxEVT_BUTTON, &DebugToolbar::OnBreakpointsButtonClick, this);
	BreakpointsButton->SetToolTip("View Breakpoints");

	MainHSizer->Add(OpenRomButton);
	MainHSizer->Add(SaveStateButton);
	MainHSizer->Add(LoadStateButton);
	MainHSizer->Add(RestartButton);
	MainHSizer->Add(DebugRunButton);
	MainHSizer->Add(DebugStepButton);
	MainHSizer->Add(DebugStepoverButton);
	MainHSizer->Add(DebugStepoutButton);
	MainHSizer->Add(BreakpointsButton);
	MainHSizer->Add(SearchButton);
	MainHSizer->Add(ScreenshotButton);
	MainHSizer->Add(IOMapButton);
	MainHSizer->Add(VRAMInspectorButton);

	SetSizer(MainHSizer);
	MainHSizer->Fit(this);
}

void DebugToolbar::OnDebugRunButtonClick(wxCommandEvent& event)
{
	m_Emulator->Run();
}

void DebugToolbar::OnDebugStepButtonClick(wxCommandEvent& event)
{
	m_Emulator->Step();
	m_MainApp->RefreshDebugger();
}

void DebugToolbar::OnDebugStepoverButtonClick(wxCommandEvent& event)
{

}

void DebugToolbar::OnDebugStepoutButtonClick(wxCommandEvent& event)
{

}

void DebugToolbar::OnSearchButtonClick(wxCommandEvent& event)
{

}

void DebugToolbar::OnScreenshotButtonClick(wxCommandEvent& event)
{

}

void DebugToolbar::OnIOMapButtonClick(wxCommandEvent& event)
{

}

void DebugToolbar::OnVRAMButtonClick(wxCommandEvent& event)
{

}

void DebugToolbar::OnOpenRomButtonClick(wxCommandEvent& event)
{
	wxCommandEvent DummyEvent;
	m_MainApp->OpenROM(DummyEvent);
}

void DebugToolbar::OnSaveStateButtonClick(wxCommandEvent& event)
{

}

void DebugToolbar::OnLoadStateButtonClick(wxCommandEvent& event)
{

}

void DebugToolbar::OnRestartButtonClick(wxCommandEvent& event)
{

}

void DebugToolbar::OnBreakpointsButtonClick(wxCommandEvent& event)
{

}

DebugToolbar::~DebugToolbar()
{
}
