#include "RegisterList.h"



RegisterList::RegisterList(RegisterView *parent) :wxListCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_VIRTUAL | wxLC_HRULES)
{
	m_RegisterView = parent;
	SetItemCount(8);
	SetMinSize(wxSize(200, 200));
	InsertColumn(0, "Register");
	InsertColumn(1, "Value");
	InsertColumn(2, "Register");
	InsertColumn(3, "Value");

	SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
	SetColumnWidth(1, wxLIST_AUTOSIZE_USEHEADER);
	SetColumnWidth(2, wxLIST_AUTOSIZE_USEHEADER);
	SetColumnWidth(3, wxLIST_AUTOSIZE_USEHEADER);

}

wxString RegisterList::OnGetItemText(long item, long column) const
{
	wxString ItemText = "";

	if (column == COL_NAME_A)
	{
		switch (item)
		{
		case ROW_REG_AF:
			ItemText = "AF";
			break;
		case ROW_REG_BC:
			ItemText = "BC";
			break;
		case ROW_REG_DE:
			ItemText = "DE";
			break;
		case ROW_REG_HL:
			ItemText = "HL";
			break;
		case ROW_REG_SP:
			ItemText = "SP";
			break;
		case ROW_REG_PC:
			ItemText = "PC";
			break;
		case ROW_REG_IME:
			ItemText = "IME";
			break;
		case ROW_REG_IMA:
			ItemText = "IMA";
			break;
		}
	}
	else if (column == COL_NAME_B)
	{
		switch (item)
		{
		case ROW_REG_LCDC:
			ItemText = "LCDC";
			break;
		case ROW_REG_STAT:
			ItemText = "STAT";
			break;
		case ROW_REG_LY:
			ItemText = "LY";
			break;
		case ROW_REG_IE:
			ItemText = "IE";
			break;
		case ROW_REG_IF:
			ItemText = "IF";
			break;
		case ROW_REG_ROM:
			ItemText = "ROM";
			break;
		case ROW_REG_DIV:
			ItemText = "DIV";
			break;
		case ROW_REG_TIMA:
			ItemText = "TIMA";
			break;

		}
	}
	else if (column == COL_VALUE_A && m_Emulator != nullptr)
	{
		switch (item)
		{
		case ROW_REG_AF:
		{
			unsigned short AFRegister = m_Emulator->GetRegisterValue(REGISTER_AF);
			ItemText = wxString::Format(wxT("%04X"), AFRegister);
			// Update Flag Checkboxes
			m_RegisterView->UpdateFlagCheckboxes(AFRegister);
			break;
		}
		case ROW_REG_BC:
			ItemText = wxString::Format(wxT("%04X"), m_Emulator->GetRegisterValue(REGISTER_BC));
			break;
		case ROW_REG_DE:
			ItemText = wxString::Format(wxT("%04X"), m_Emulator->GetRegisterValue(REGISTER_DE));
			break;
		case ROW_REG_HL:
			ItemText = wxString::Format(wxT("%04X"), m_Emulator->GetRegisterValue(REGISTER_HL));
			break;
		case ROW_REG_SP:
			ItemText = wxString::Format(wxT("%04X"), m_Emulator->GetRegisterValue(REGISTER_SP));
			break;
		case ROW_REG_PC:
			ItemText = wxString::Format(wxT("%04X"), m_Emulator->GetRegisterValue(REGISTER_PC));
			break;
		case ROW_REG_IME:
			ItemText = wxString::Format(wxT("%s"), m_Emulator->GetRegisterValue(REGISTER_IME)?"True":"False");
			break;
		case ROW_REG_IMA:
			ItemText = wxString::Format(wxT("%04X"), m_Emulator->GetRegisterValue(REGISTER_IMA));
			break;
		}
	}
	else if (column == COL_VALUE_B && m_Emulator != nullptr)
	{
		switch (item)
		{
		case ROW_REG_LCDC:
			ItemText = wxString::Format(wxT("%02X"), m_Emulator->GetRegisterValue(REGISTER_LCDC));
			break;
		case ROW_REG_STAT:
			ItemText = wxString::Format(wxT("%02X"), m_Emulator->GetRegisterValue(REGISTER_STAT));
			break;
		case ROW_REG_LY:
			ItemText = wxString::Format(wxT("%02X"), m_Emulator->GetRegisterValue(REGISTER_LY));
			break;
		case ROW_REG_IE:
			ItemText = wxString::Format(wxT("%02X"), m_Emulator->GetRegisterValue(REGISTER_IE));
			break;
		case ROW_REG_IF:
			ItemText = wxString::Format(wxT("%02X"), m_Emulator->GetRegisterValue(REGISTER_IF));
			break;
		case ROW_REG_ROM:
			ItemText = wxString::Format(wxT("%02X"), m_Emulator->GetRegisterValue(REGISTER_ROM));
			break;
		case ROW_REG_DIV:
			ItemText = wxString::Format(wxT("%02X"), m_Emulator->GetRegisterValue(REGISTER_DIV));
			break;
		case ROW_REG_TIMA:
			ItemText = wxString::Format(wxT("%04X"), m_Emulator->GetRegisterValue(REGISTER_TIMA));
			break;
		}
	}


	return ItemText;
}

void RegisterList::SetEmulator(EmulatorEngine *Emulator)
{
	m_Emulator = Emulator;
}

RegisterList::~RegisterList()
{
}
