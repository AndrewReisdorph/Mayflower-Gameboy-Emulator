#include "MayflowerWindow.h"
#include "main.h"
#include <thread>

MayflowerWindow::MayflowerWindow(): wxFrame(NULL, wxID_ANY, "Mayflower Gameboy Emulator")
{
	m_MenuBar = new wxMenuBar;
	FileMenu = new wxMenu;
	FileMenu->Append(ID_OpenROM, wxT("&Open ROM"));
	FileMenu->Append(ID_Quit, wxT("&Quit"));
	m_MenuBar->Append(FileMenu, wxT("&File"));
	SetMenuBar(m_MenuBar);
	Centre();

	InitUI();

	m_Emulator = new EmulatorEngine(m_ScreenPanel, m_RamAssemblyList, this);
	m_ScreenPanel->SetEmulator(m_Emulator);
	m_RamAssemblyList->SetEmulator(m_Emulator);
	m_RegisterView->SetEmulator(m_Emulator);
	m_MemoryViewListCtrl->SetEmulator(m_Emulator);

	std::thread emu_thread(&EmulatorEngine::EmulatorStateMachine, m_Emulator);
	emu_thread.detach();

	Bind(wxEVT_KEY_DOWN, &MayflowerWindow::OnKeyDown, this);
	Bind(wxEVT_KEY_UP, &MayflowerWindow::OnKeyUp, this);
}

void MayflowerWindow::InitUI()
{
	m_ScreenPanel = new GameboyScreenPanel(this);
	m_RamAssemblyList = new RamAssemblyList(this);
	wxBoxSizer *mainVSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *DebugControlHSizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *RamViewHSizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *RegisterAndRamVSizer = new wxBoxSizer(wxVERTICAL);

	wxButton *StepButton = new wxButton(this, wxID_ANY, "Step");
	StepButton->Bind(wxEVT_BUTTON, &MayflowerWindow::OnStepButtonPress, this);
	wxButton *RunButton = new wxButton(this, wxID_ANY, "Run");
	RunButton->Bind(wxEVT_BUTTON, &MayflowerWindow::OnRunButtonPress, this);
	DebugControlHSizer->Add(StepButton);
	DebugControlHSizer->Add(RunButton);

	m_MemoryViewListCtrl = new MemoryViewListCtrl(this);
	m_RegisterView = new RegisterView(this);

	RegisterAndRamVSizer->Add(m_RegisterView, wxSizerFlags().Expand());
	RegisterAndRamVSizer->Add(m_MemoryViewListCtrl, wxSizerFlags().Expand().Proportion(1));

	RamViewHSizer->Add(m_RamAssemblyList, wxSizerFlags().Expand().Proportion(1));
	RamViewHSizer->Add(RegisterAndRamVSizer, wxSizerFlags().Expand());

	mainVSizer->Add(m_ScreenPanel, wxSizerFlags().Center());
	mainVSizer->Add(DebugControlHSizer, wxSizerFlags().Expand());
	mainVSizer->Add(RamViewHSizer, wxSizerFlags().Expand().Proportion(1));

	SetSizer(mainVSizer);
	mainVSizer->Fit(this);

}

void MayflowerWindow::OnKeyDown(wxKeyEvent& event)
{
	switch (event.GetKeyCode())
	{
	case WXK_RETURN:
	case WXK_SHIFT:
	case WXK_LEFT:
	case WXK_RIGHT:
	case WXK_UP:
	case WXK_DOWN:
	case 'A':
	case 'S':
		m_Emulator->KeyDown(event.GetKeyCode());
		break;
	}
	event.Skip();
}

void MayflowerWindow::OnKeyUp(wxKeyEvent& event)
{
	switch (event.GetKeyCode())
	{
	case WXK_RETURN:
	case WXK_SHIFT:
	case WXK_LEFT:
	case WXK_RIGHT:
	case WXK_UP:
	case WXK_DOWN:
	case 'A':
	case 'S':
		m_Emulator->KeyUp(event.GetKeyCode());
		break;
	}
	event.Skip();
}

void MayflowerWindow::OnStepButtonPress(wxCommandEvent& event)
{
	m_Emulator->Step();
	RefreshDebugger();
}

void MayflowerWindow::RefreshScreenPanel()
{
	m_ScreenPanel->Refresh();
}

void MayflowerWindow::RefreshGameboyScreen()
{
	wxTheApp->GetTopWindow()->GetEventHandler()->CallAfter(&MayflowerWindow::RefreshScreenPanel);
}

void MayflowerWindow::RefreshDebugger()
{
	m_RamAssemblyList->Refresh();
	m_RegisterView->Refresh();
	m_MemoryViewListCtrl->Refresh();
}

void MayflowerWindow::OnRunButtonPress(wxCommandEvent& event)
{
	m_Emulator->Run();
}

void MayflowerWindow::OpenROM(wxCommandEvent& event)
{
	wxFileDialog *OpenRomDialog = new wxFileDialog(this, _("Choose ROM"), wxEmptyString, wxEmptyString, wxEmptyString, wxFD_OPEN, wxDefaultPosition);
	if (OpenRomDialog->ShowModal() == wxID_OK)
	{
		m_Emulator->SetRomPath(OpenRomDialog->GetPath());
		m_Emulator->SetState(EMULATOR_STATE_INIT);
	}
	OpenRomDialog->Destroy();
}

BEGIN_EVENT_TABLE(MayflowerWindow, wxFrame)
EVT_MENU(ID_OpenROM, MayflowerWindow::OpenROM)
END_EVENT_TABLE()