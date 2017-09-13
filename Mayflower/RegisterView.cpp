#include "RegisterView.h"



RegisterView::RegisterView(wxFrame *parent) :wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_RAISED)
{
	InitUI();
}

void RegisterView::InitUI()
{
	wxBoxSizer *MainVSizer = new wxBoxSizer(wxVERTICAL);
	m_FlagZCheckbox = new wxCheckBox(this, wxID_ANY, "Z");
	m_FlagNCheckbox = new wxCheckBox(this, wxID_ANY, "N");
	m_FlagHCheckbox = new wxCheckBox(this, wxID_ANY, "H");
	m_FlagCCheckbox = new wxCheckBox(this, wxID_ANY, "C");
	wxBoxSizer *CheckBoxHSizer = new wxBoxSizer(wxHORIZONTAL);
	CheckBoxHSizer->Add(m_FlagZCheckbox);
	CheckBoxHSizer->Add(m_FlagNCheckbox);
	CheckBoxHSizer->Add(m_FlagHCheckbox);
	CheckBoxHSizer->Add(m_FlagCCheckbox);

	m_RegisterList = new RegisterList(this);


	MainVSizer->Add(m_RegisterList);
	MainVSizer->Add(CheckBoxHSizer, wxSizerFlags().Border(wxALL));
	SetSizer(MainVSizer);
	MainVSizer->Fit(this);
}

void RegisterView::UpdateFlagCheckboxes(unsigned short FlagWord)
{
	flag_reg flags;
	flags.all = FlagWord & 0x00FF;
	m_FlagZCheckbox->SetValue(flags.Z);
	m_FlagNCheckbox->SetValue(flags.N);
	m_FlagHCheckbox->SetValue(flags.H);
	m_FlagCCheckbox->SetValue(flags.C);

}

void RegisterView::SetEmulator(EmulatorEngine *Emulator)
{
	m_Emulator = Emulator;
	m_RegisterList->SetEmulator(m_Emulator);
}

RegisterView::~RegisterView()
{
}
