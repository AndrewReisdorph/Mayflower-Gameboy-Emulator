#include "MayflowerWindow.h"
#include "main.h"
#include <thread>

MayflowerWindow::MayflowerWindow(): wxFrame(NULL, wxID_ANY, "Mayflower Gameboy Emulator")
{
	m_MenuBar = new wxMenuBar;
	m_FileMenu = new wxMenu;
	m_FileMenu->Append(ID_OpenROM, wxT("&Open ROM"));
	m_FileMenu->Append(ID_Quit, wxT("&Quit"));
	m_MenuBar->Append(m_FileMenu, wxT("&File"));
	m_DebugMenu = new wxMenu;
	m_DebugMenu->Append(ID_Step, wxT("&Step"));
	m_MenuBar->Append(m_DebugMenu, wxT("&Debug"));
	m_WindowMenu = new wxMenu;
	m_WindowMenu->Append(ID_VramView, wxT("&Inspect VRAM"));
	m_MenuBar->Append(m_WindowMenu, wxT("&Window"));

	m_HelpMenu = new wxMenu;
	m_HelpMenu->Append(ID_About, wxT("&About"));
	m_MenuBar->Append(m_HelpMenu, wxT("&Help"));

	SetMenuBar(m_MenuBar);
	Centre();

	InitUI();

	m_Emulator = new EmulatorEngine(m_ScreenPanel, m_RamAssemblyList, this);
	m_ScreenPanel->SetEmulator(m_Emulator);
	m_RamAssemblyList->SetEmulator(m_Emulator);
	m_RegisterView->SetEmulator(m_Emulator);
	m_MemoryViewListCtrl->SetEmulator(m_Emulator);
	m_DebugControl->SetEmulator(m_Emulator);

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
	wxBoxSizer *RamViewHSizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *RegisterAndRamVSizer = new wxBoxSizer(wxVERTICAL);

	m_DebugControl = new DebugToolbar(this, this);
	m_MemoryViewListCtrl = new MemoryViewListCtrl(this);
	m_RegisterView = new RegisterView(this);

	RegisterAndRamVSizer->Add(m_RegisterView, wxSizerFlags().Expand());
	RegisterAndRamVSizer->Add(m_MemoryViewListCtrl, wxSizerFlags().Expand().Proportion(1));

	RamViewHSizer->Add(m_RamAssemblyList, wxSizerFlags().Expand().Proportion(1));
	RamViewHSizer->Add(RegisterAndRamVSizer, wxSizerFlags().Expand());

	mainVSizer->Add(m_DebugControl, wxSizerFlags().Expand());
	mainVSizer->Add(m_ScreenPanel, wxSizerFlags().Center().Border());
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