#include <sstream> 
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include "RamAssemblyList.h"

using namespace std;

RamAssemblyList::RamAssemblyList(wxFrame *parent) :wxListCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_VIRTUAL | wxLC_HRULES)
{
	SetMinSize(wxSize(500, 500));
	m_ItemAttr = new wxListItemAttr();
	InitUI();
	SetItemCount(0);
	Bind(wxEVT_LIST_ITEM_ACTIVATED, &RamAssemblyList::OnItemActivate, this);
}

void RamAssemblyList::SetEmulator(EmulatorEngine *Emulator)
{
	m_Emulator = Emulator;
}

wxString RamAssemblyList::OnGetItemText(long item, long column) const
{
	wxString ItemText;
	bool CBPrefix = false;
	unsigned short ItemAddress;
	Instruction Op = m_Emulator->GetNthInstruction(item, CBPrefix, ItemAddress);
	RomAddressDataType AddressDataType = m_Emulator->GetRomAddressDataType(ItemAddress);

	switch (column)
	{
	case COL_PC:
		ItemText = "";
		break;
	case COL_ADDRESS:
		ItemText = wxString::Format(wxT("%s:%04X"),GetAddressHeading(ItemAddress),ItemAddress);
		break;
	case COL_BYTES:
	{
		switch (AddressDataType)
		{
		case ROM_ASM:
			int NumArgBytes;
			switch (Op.OpArgType)
			{
			case ARG_SIGNED_8:
			case ARG_UNSIGNED_8:
				NumArgBytes = 1;
				break;
			case ARG_UNSIGNED_16:
				NumArgBytes = 2;
				break;
			default:
				NumArgBytes = 0;
				break;
			}

			switch (NumArgBytes)
			{
			case 0:
				if (CBPrefix)
				{
					ItemText = wxString::Format(wxT("CB %02X"), Op.OpCode);
				}
				else
				{
					ItemText = wxString::Format(wxT("%02X"), Op.OpCode);
				}
				break;
			case 1:
				ItemText = wxString::Format(wxT("%02X %02X"), Op.OpCode, Op.Argument.uint8);
				break;
			case 2:
				ItemText = wxString::Format(wxT("%02X %02X %02X"), Op.OpCode, (Op.Argument.uint16 & 0xFF00) >> 8, Op.Argument.uint16 & 0x00FF);
				break;
			}
			break;
		case ROM_DATA_1:
			ItemText = wxString::Format(wxT("%02X"), m_Emulator->GetMMU()->ReadMemory8(ItemAddress));
			break;
		case ROM_DATA_2:
			ItemText = wxString::Format(wxT("%02X %02X"), m_Emulator->GetMMU()->ReadMemory8(ItemAddress), m_Emulator->GetMMU()->ReadMemory8(ItemAddress + 1));
			break;
		case ROM_DATA_3:
			ItemText = wxString::Format(wxT("%02X %02X %02X"), m_Emulator->GetMMU()->ReadMemory8(ItemAddress), m_Emulator->GetMMU()->ReadMemory8(ItemAddress+1), m_Emulator->GetMMU()->ReadMemory8(ItemAddress+2));
			break;
		case ROM_DATA_4:
			ItemText = wxString::Format(wxT("%02X %02X %02X %02X"), m_Emulator->GetMMU()->ReadMemory8(ItemAddress), m_Emulator->GetMMU()->ReadMemory8(ItemAddress+1), m_Emulator->GetMMU()->ReadMemory8(ItemAddress+2), m_Emulator->GetMMU()->ReadMemory8(ItemAddress+3));
			break;
		case ROM_DATA_7:
		case ROM_DATA_9:
		case ROM_DATA_15:
		case ROM_DATA_16:
			ItemText = wxString::Format(wxT("%02X %02X %02X %02X+"), m_Emulator->GetMMU()->ReadMemory8(ItemAddress), m_Emulator->GetMMU()->ReadMemory8(ItemAddress+1), m_Emulator->GetMMU()->ReadMemory8(ItemAddress+2), m_Emulator->GetMMU()->ReadMemory8(ItemAddress+3));
			break;

		}
		break;
	}
	case COL_ASSEMBLY:
	{
		int NumArgBytes = 0;
		switch (AddressDataType)
		{
			case ROM_ASM:
			{
				wxString OpDescription = Op.Name;
				if (OpDescription.Find("d16") != wxNOT_FOUND)
				{
					NumArgBytes = 2;
					OpDescription.Replace("d16", wxString::Format(wxT("%04X"), Op.Argument.uint16));
				}
				else if (OpDescription.Find("a16") != wxNOT_FOUND)
				{
					NumArgBytes = 2;
					OpDescription.Replace("a16", wxString::Format(wxT("%04X"), Op.Argument.uint16));
				}
				else if (OpDescription.Find("r8") != wxNOT_FOUND)
				{
					NumArgBytes = 1;
					stringstream sa;
					if (Op.Type == OP_JUMP_CALL)
					{
						unsigned short JumpAddress = ItemAddress + 2 + Op.Argument.sint8;
						sa << setfill('0') << setw(4) << uppercase << hex << JumpAddress;
						OpDescription.Replace("r8", sa.str());
					}
				}
				else if (OpDescription.Find("d8") != wxNOT_FOUND)
				{
					NumArgBytes = 1;
					OpDescription.Replace("d8", wxString::Format(wxT("%02X"), Op.Argument.uint8));
				}
				else if (OpDescription.Find("a8") != wxNOT_FOUND)
				{
					NumArgBytes = 1;
					OpDescription.Replace("a8", wxString::Format(wxT("%02X"), Op.Argument.uint8));
				}
				else if (Op.Type == OP_EMPTY)
				{
					OpDescription = "--";
				}
				ItemText = OpDescription;
				break;
			}
			case ROM_DATA_1:
				ItemText = wxString::Format(wxT("%02X"),
					m_Emulator->GetMMU()->ReadMemory8(ItemAddress + 0));
				break;
			case ROM_DATA_2:
				ItemText = wxString::Format(wxT("%02X %02X"),
					m_Emulator->GetMMU()->ReadMemory8(ItemAddress + 0),
					m_Emulator->GetMMU()->ReadMemory8(ItemAddress + 1));
				break;
			case ROM_DATA_3:
				ItemText = wxString::Format(wxT("%02X %02X %02X"),
					m_Emulator->GetMMU()->ReadMemory8(ItemAddress + 0),
					m_Emulator->GetMMU()->ReadMemory8(ItemAddress + 1),
					m_Emulator->GetMMU()->ReadMemory8(ItemAddress + 2));
				break;
			case ROM_DATA_4:
				ItemText = wxString::Format(wxT("%02X %02X %02X %02X"),
					m_Emulator->GetMMU()->ReadMemory8(ItemAddress +0),
					m_Emulator->GetMMU()->ReadMemory8(ItemAddress +1),
					m_Emulator->GetMMU()->ReadMemory8(ItemAddress +2),
					m_Emulator->GetMMU()->ReadMemory8(ItemAddress +3) );
				break;
			case ROM_DATA_7:
				ItemText = wxString::Format(wxT("%02X %02X %02X %02X %02X %02X %02X"),
					m_Emulator->GetMMU()->ReadMemory8(ItemAddress + 0),
					m_Emulator->GetMMU()->ReadMemory8(ItemAddress + 1),
					m_Emulator->GetMMU()->ReadMemory8(ItemAddress + 2),
					m_Emulator->GetMMU()->ReadMemory8(ItemAddress + 3),
					m_Emulator->GetMMU()->ReadMemory8(ItemAddress + 4),
					m_Emulator->GetMMU()->ReadMemory8(ItemAddress + 5),
					m_Emulator->GetMMU()->ReadMemory8(ItemAddress + 6));
				break;
			case ROM_DATA_9:
				ItemText = wxString::Format(wxT("%02X %02X %02X %02X %02X %02X %02X %02X %02X"),
					m_Emulator->GetMMU()->ReadMemory8(ItemAddress + 0),
					m_Emulator->GetMMU()->ReadMemory8(ItemAddress + 1),
					m_Emulator->GetMMU()->ReadMemory8(ItemAddress + 2),
					m_Emulator->GetMMU()->ReadMemory8(ItemAddress + 3),
					m_Emulator->GetMMU()->ReadMemory8(ItemAddress + 4),
					m_Emulator->GetMMU()->ReadMemory8(ItemAddress + 5),
					m_Emulator->GetMMU()->ReadMemory8(ItemAddress + 6),
					m_Emulator->GetMMU()->ReadMemory8(ItemAddress + 7),
					m_Emulator->GetMMU()->ReadMemory8(ItemAddress + 8));
				break;
			case ROM_DATA_15:
				if (ItemAddress == CARTRIDGE_TITLE_ADDR)
				{
					ItemText = m_Emulator->GetCartridgeTitle();
				}
				else
				{
					ItemText = wxString::Format(wxT("%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X"),
						m_Emulator->GetMMU()->ReadMemory8(ItemAddress + 0),
						m_Emulator->GetMMU()->ReadMemory8(ItemAddress + 1),
						m_Emulator->GetMMU()->ReadMemory8(ItemAddress + 2),
						m_Emulator->GetMMU()->ReadMemory8(ItemAddress + 3),
						m_Emulator->GetMMU()->ReadMemory8(ItemAddress + 4),
						m_Emulator->GetMMU()->ReadMemory8(ItemAddress + 5),
						m_Emulator->GetMMU()->ReadMemory8(ItemAddress + 6),
						m_Emulator->GetMMU()->ReadMemory8(ItemAddress + 7),
						m_Emulator->GetMMU()->ReadMemory8(ItemAddress + 8),
						m_Emulator->GetMMU()->ReadMemory8(ItemAddress + 9),
						m_Emulator->GetMMU()->ReadMemory8(ItemAddress + 10),
						m_Emulator->GetMMU()->ReadMemory8(ItemAddress + 11),
						m_Emulator->GetMMU()->ReadMemory8(ItemAddress + 12),
						m_Emulator->GetMMU()->ReadMemory8(ItemAddress + 13),
						m_Emulator->GetMMU()->ReadMemory8(ItemAddress + 14));
				}
				break;
			case ROM_DATA_16:
				ItemText = wxString::Format(wxT("%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X"),
					m_Emulator->GetMMU()->ReadMemory8(ItemAddress + 0),
					m_Emulator->GetMMU()->ReadMemory8(ItemAddress + 1),
					m_Emulator->GetMMU()->ReadMemory8(ItemAddress + 2),
					m_Emulator->GetMMU()->ReadMemory8(ItemAddress + 3),
					m_Emulator->GetMMU()->ReadMemory8(ItemAddress + 4),
					m_Emulator->GetMMU()->ReadMemory8(ItemAddress + 5),
					m_Emulator->GetMMU()->ReadMemory8(ItemAddress + 6),
					m_Emulator->GetMMU()->ReadMemory8(ItemAddress + 7),
					m_Emulator->GetMMU()->ReadMemory8(ItemAddress + 8),
					m_Emulator->GetMMU()->ReadMemory8(ItemAddress + 9),
					m_Emulator->GetMMU()->ReadMemory8(ItemAddress + 10),
					m_Emulator->GetMMU()->ReadMemory8(ItemAddress + 11),
					m_Emulator->GetMMU()->ReadMemory8(ItemAddress + 12),
					m_Emulator->GetMMU()->ReadMemory8(ItemAddress + 13),
					m_Emulator->GetMMU()->ReadMemory8(ItemAddress + 14),
					m_Emulator->GetMMU()->ReadMemory8(ItemAddress + 15));
				break;
		}
		
		break;
	}
	case COL_COMMENT:
	{
		ItemText = m_Emulator->GetAddressComment(ItemAddress);
		break;
	}
	default:
		break;
	}
	

	return ItemText;
}

int RamAssemblyList::OnGetItemImage(long item) const
{
	int Image = BLANK_IMG;

	return Image;
}

int RamAssemblyList::OnGetItemColumnImage(long item, long column) const
{
	int Image = BLANK_IMG;
	
	if (column == COL_PC)
	{
		bool RowIsPC = m_Emulator->NthInstructionIsPC(item);
		bool RowIsBreakPoint = m_Emulator->NthInstructionIsBreakpoint(item);

		if ( RowIsBreakPoint )
		{
			if (RowIsPC)
			{
				Image = BREAKPOINT_PC_IMG;
			}
			else
			{
				Image = BREAKPOINT_IMG;
			}
		}
		else
		{
			if (RowIsPC)
			{
				Image = PC_IMG;
			}
			else
			{
				Image = BLANK_IMG;
			}
		}
	}

	return Image;
}

wxListItemAttr* RamAssemblyList::OnGetItemAttr(long item)	const
{
	wxColour BackgroundColor;
	bool RowIsPC = m_Emulator->NthInstructionIsPC(item);
	bool RowIsBreakPoint = m_Emulator->NthInstructionIsBreakpoint(item);

	if (RowIsPC)
	{
		BackgroundColor = wxColour(164, 203, 242);
	}
	else if (RowIsBreakPoint)
	{
		BackgroundColor = wxColour(242, 164, 164);
	}
	else
	{
		BackgroundColor = wxColour(0xFF, 0xFF, 0xFF);
	}

	m_ItemAttr->SetBackgroundColour(BackgroundColor);

	return m_ItemAttr;
}

void RamAssemblyList::OnItemActivate(wxListEvent& event)
{
	ModifyBreakpoint(event.GetIndex(), !m_Emulator->NthInstructionIsBreakpoint(event.GetIndex()));
}

void RamAssemblyList::InitUI()
{
	wxInitAllImageHandlers();

	m_ImageList = new wxImageList(32,16,true,3);
	wxBitmap ProgramCounterBitmap = wxBITMAP_PNG_FROM_DATA(program_counter);
	wxBitmap BreakpointBitmap = wxBITMAP_PNG_FROM_DATA(breakpoint);
	wxBitmap ProgramCounterAtBreakPointBitmap = wxBITMAP_PNG_FROM_DATA(pc_at_breakpoint);
	m_ImageList->Add(ProgramCounterBitmap);
	m_ImageList->Add(BreakpointBitmap);
	m_ImageList->Add(ProgramCounterAtBreakPointBitmap);

	SetImageList(m_ImageList, wxIMAGE_LIST_SMALL);

	InsertColumn(COL_PC, "", 0, 36);
	InsertColumn(COL_ADDRESS, "Address", 0, 100);
	InsertColumn(COL_BYTES, "Bytes", 0, 70);
	InsertColumn(COL_ASSEMBLY, "Assembly/Data", 0, 140);
	InsertColumn(COL_COMMENT, "Comment", 0, 180);
}

wxString RamAssemblyList::GetAddressHeading(int Address) const
{
	wxString Heading;
	if (Address >= 0 && Address <= 0x3FFF)
	{
		if (m_Emulator->UsingBootRom() && Address < 0x100)
		{
			Heading = "BOOT";
		}
		else
		{
			Heading = "ROM0";
		}
	}
	else if (Address > 0x3FFF && Address <= 0x7FFF)
	{
		Heading = wxString::Format(wxT("ROM%i"), m_Emulator->GetRomBankNumber());
	}
	else if (Address >= 0x8000 && Address < 0xA000)
	{
		Heading = "VRA0";
	}
	else if (Address >= 0xA000 && Address < 0xC000)
	{
		Heading = "SRA0";
	}
	else if (Address >= 0xC000 && Address < 0xD000)
	{
		Heading = "WRA0";
	}
	else if (Address >= 0xD000 && Address < 0xE000)
	{
		Heading = "WRA1";
	}
	else if (Address >= 0xE000 && Address < 0xF000)
	{
		Heading = "ECH0";
	}
	else if (Address >= 0xF000 && Address < 0xFE00)
	{
		Heading = "ECH1";
	}
	else if (Address >= 0xFE00 && Address <= 0xFE90)
	{
		Heading = "OAM";
	}
	else if (Address > 0xFEA0 && Address < 0xFF00)
	{
		Heading = "----";
	}
	else if (Address >= 0xFF00 && Address < 0xFF80)
	{
		Heading = "I/O ";
	}
	else if (Address >= 0xFF80 && Address < 0xFFFF)
	{
		Heading = "HRAM";
	}
	else if(Address == 0xFFFF)
	{
		Heading = "I/O ";
	}
	else
	{
		Heading = "FUCK";
	}

	return Heading;
}

void RamAssemblyList::UpdateProgramCounter(int OldPC, int NewPC)
{
	// Clear image and blue background
	SetItem(OldPC, 0, "", -1);
	SetItemBackgroundColour(OldPC, wxColour(0xFF, 0xFF, 0xFF));

	SetItem(NewPC, 0, "", 0);
	SetItemBackgroundColour(NewPC, wxColour(158, 200, 239));
}

void RamAssemblyList::ModifyBreakpoint(int BreakPointRow, bool SetBreakpoint)
{
	if (SetBreakpoint)
	{
		m_Emulator->AddBreakpointAtNthInstruction(BreakPointRow);
		//SetItemBackgroundColour(BreakPointRow, wxColour(239, 163, 158));
		
	}
	else
	{
		m_Emulator->RemoveBreakpointAtNthInstruction(BreakPointRow);
	}
	SetItemState(BreakPointRow, 0, wxLIST_STATE_SELECTED);
}

RamAssemblyList::~RamAssemblyList()
{
	delete m_ItemAttr;
}
