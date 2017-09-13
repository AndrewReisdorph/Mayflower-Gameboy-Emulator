#include "MemoryViewListCtrl.h"



MemoryViewListCtrl::MemoryViewListCtrl(wxFrame *parent) :wxListCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_VIRTUAL | wxLC_HRULES)
{
	SetItemCount(1 + 0xFFFF/2);
	InsertColumn(0, "Address", 0, 70);
	InsertColumn(1, "Value", 0, 100);
	SetMinSize(wxSize(200, -1));
}

void MemoryViewListCtrl::SetEmulator(EmulatorEngine *Emulator)
{
	m_Emulator = Emulator;
}

wxString MemoryViewListCtrl::OnGetItemText(long item, long column) const
{
	wxString ItemText = "--";
	unsigned short Address = 0xFFFE - item * 2;

	if (column == COL_ADDRESS)
	{
		ItemText = wxString::Format(wxT("%04X"), Address);
	}
	else if( column == COL_VALUE )
	{
		if (m_Emulator != nullptr)
		{
			ItemText = wxString::Format(wxT("%04X"), m_Emulator->GetMMU()->ReadMemory16(Address));
		}
	}

	return ItemText;
}

MemoryViewListCtrl::~MemoryViewListCtrl()
{
}
