#include "IOMap.h"

io_entry m_IOEntries[] = {
	// LCD
	{ nullptr, "LCDC", 0xFF40 },
	{ nullptr, "STAT", 0xFF41 },
	{ nullptr, "SCY",  0xFF42 },
	{ nullptr, "SCX",  0xFF43 },
	{ nullptr, "LY",   0xFF44 },
	{ nullptr, "LYC",  0xFF45 },
	{ nullptr, "DMA",  0xFF46 },
	{ nullptr, "BGP",  0xFF47 },
	{ nullptr, "OBP0", 0xFF48 },
	{ nullptr, "OBP1", 0xFF49 },
	{ nullptr, "WY",   0xFF4A },
	{ nullptr, "WX",   0xFF4B },
	// Miscellaneous
	{ nullptr, "SVBK", 0xFF70 },
	{ nullptr, "VBK",  0xFF4F },
	{ nullptr, "KEY1", 0xFF4D },
	{ nullptr, "JOYP", 0xFF00 },
	{ nullptr, "SB",   0xFF01 },
	{ nullptr, "SC",   0xFF02 },
	{ nullptr, "DIV",  0xFF04 },
	{ nullptr, "TIMA", 0xFF05 },
	{ nullptr, "TMA",  0xFF06 },
	{ nullptr, "TAC",  0xFF07 },
	{ nullptr, "IF",   0xFF0F },
	{ nullptr, "IE",   0xFFFF },
	// Sound 1
	{ nullptr, "ENT1", 0xFF10 },
	{ nullptr, "LEN1", 0xFF11 },
	{ nullptr, "ENV1", 0xFF12 },
	{ nullptr, "FRQ1", 0xFF13 },
	{ nullptr, "KIK1", 0xFF14 },
	// Sound 2
	{ nullptr, "N/A",  0xFF15 },
	{ nullptr, "LEN2", 0xFF16 },
	{ nullptr, "ENV2", 0xFF17 },
	{ nullptr, "FRQ2", 0xFF18 },
	{ nullptr, "KIK2", 0xFF19 },
	// Sound 3
	{ nullptr, "ON_3", 0xFF1A },
	{ nullptr, "LEN3", 0xFF1B },
	{ nullptr, "ENV3", 0xFF1C },
	{ nullptr, "FRQ3", 0xFF1D },
	{ nullptr, "KIK3", 0xFF1E },
	// Sound 4
	{ nullptr, "N/A",  0xFF1F },
	{ nullptr, "LEN4", 0xFF20 },
	{ nullptr, "ENV4", 0xFF21 },
	{ nullptr, "FRQ4", 0xFF22 },
	{ nullptr, "KIK4", 0xFF23 },
	// Sound Control
	{ nullptr, "VOL", 0xFF24 },
	{ nullptr, "L/R", 0xFF25 },
	{ nullptr, "ON",  0xFF26 }
};


IOMap::IOMap(wxFrame *parent) :wxDialog(parent, wxID_ANY, "IO Map")
{
	InitUI();
}

void IOMap::InitUI()
{
	wxInitAllImageHandlers();

	wxBoxSizer *MainHBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *SoundVSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *RightHalfVSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *SecondRowHSizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *MBCAndRefreshButtonVSizer = new wxBoxSizer(wxVERTICAL);
	wxStaticBoxSizer *LCDVBoxSizer = new wxStaticBoxSizer(wxVERTICAL, this, "LCD");
	wxStaticBoxSizer *MiscBoxSizer = new wxStaticBoxSizer(wxVERTICAL, this, "Miscellaneous");
	wxStaticBoxSizer *Sound1BoxSizer = new wxStaticBoxSizer(wxVERTICAL, this, "Sound 1");
	wxStaticBoxSizer *Sound2BoxSizer = new wxStaticBoxSizer(wxVERTICAL, this, "Sound 2");
	wxStaticBoxSizer *Sound3BoxSizer = new wxStaticBoxSizer(wxVERTICAL, this, "Sound 3");
	wxStaticBoxSizer *Sound4BoxSizer = new wxStaticBoxSizer(wxVERTICAL, this, "Sound 4");
	wxStaticBoxSizer *SoundControlBoxSizer = new wxStaticBoxSizer(wxVERTICAL, this, "Sound Control");
	wxStaticBoxSizer *WavePatternBoxSizer = new wxStaticBoxSizer(wxVERTICAL, this, "Wave Pattern (FF3X)");
	wxStaticBoxSizer *LCDCStaticBoxSizer = new wxStaticBoxSizer(wxVERTICAL, this, "LCDC (FF40)");
	wxStaticBoxSizer *LCDSTATStaticBoxSizer = new wxStaticBoxSizer(wxVERTICAL, this, "STAT (FF41)");
	wxStaticBoxSizer *IFIEStaticBoxSizer = new wxStaticBoxSizer(wxVERTICAL, this, "IF - IE");
	wxStaticBoxSizer *MBCStaticBoxSizer = new wxStaticBoxSizer(wxVERTICAL, this, "MBC");

	wxFlexGridSizer *LCDGridSizer = new wxFlexGridSizer(12, 2, 5, 1);
	wxFlexGridSizer *MiscEntryGridSizer = new wxFlexGridSizer(12, 2, 5, 1);
	wxFlexGridSizer *SoundGridSizer = new wxFlexGridSizer(1, 5, 1, 5);
	wxFlexGridSizer *Sound1GridSizer = new wxFlexGridSizer(5, 2, 8, 0);
	wxFlexGridSizer *Sound2GridSizer = new wxFlexGridSizer(5, 2, 8, 0);
	wxFlexGridSizer *Sound3GridSizer = new wxFlexGridSizer(5, 2, 8, 0);
	wxFlexGridSizer *Sound4GridSizer = new wxFlexGridSizer(5, 2, 8, 0);
	wxFlexGridSizer *SoundControlGridSizer = new wxFlexGridSizer(3, 2, 1, 0);
	wxFlexGridSizer *WavePatternBoxGridSizer = new wxFlexGridSizer(2, 2, 5, 5);
	wxFlexGridSizer *LCDCGridSizer = new wxFlexGridSizer(8, 2, 0, 0);
	wxFlexGridSizer *LCDSTATGridSizer = new wxFlexGridSizer(6, 1, 3, 0);
	wxFlexGridSizer *IFIEGridSizer = new wxFlexGridSizer(5, 4, 2, 2);
	wxFlexGridSizer *MBCGridSizer = new wxFlexGridSizer(2, 2, 2, 2);

	for (int EntryIter = 0; EntryIter < NUM_IO_ENTRIES; EntryIter++)
	{
		wxFlexGridSizer *TargetSizer;
		wxStaticText *IOEntryText = new wxStaticText(this, wxID_ANY, wxString::Format(wxT("%4X %s"), m_IOEntries[EntryIter].Address, m_IOEntries[EntryIter].Label));
		m_IOEntries[EntryIter].TextCtrl = new wxTextCtrl(this, wxID_ANY,"00");
		m_IOEntries[EntryIter].TextCtrl->SetMinSize(wxSize(25, 20));

		if (EntryIter < MAX_LCD_ENTRY)
		{
			TargetSizer = LCDGridSizer;
		}
		else if (EntryIter < MAX_MISC_ENTRY)
		{
			TargetSizer = MiscEntryGridSizer;
		}
		else if (EntryIter < MAX_SOUND1_ENTRY)
		{
			TargetSizer = Sound1GridSizer;
		}
		else if (EntryIter < MAX_SOUND2_ENTRY)
		{
			TargetSizer = Sound2GridSizer;
		}
		else if (EntryIter < MAX_SOUND3_ENTRY)
		{
			TargetSizer = Sound3GridSizer;
		}
		else if (EntryIter < MAX_SOUND4_ENTRY)
		{
			TargetSizer = Sound4GridSizer;
		}
		else
		{
			TargetSizer = SoundControlGridSizer;
		}

		TargetSizer->Add(m_IOEntries[EntryIter].TextCtrl);
		TargetSizer->Add(IOEntryText, wxSizerFlags().CenterVertical());
	}

	m_LCDCtrlCheckboxes.LCDOff = new wxCheckBox(this, wxID_ANY, "LCD");
	wxStaticText *LCDC_LCDOffText = new wxStaticText(this, wxID_ANY, "Off");
	LCDCGridSizer->Add(m_LCDCtrlCheckboxes.LCDOff);
	LCDCGridSizer->Add(LCDC_LCDOffText);
	m_LCDCtrlCheckboxes.WinAddr = new wxCheckBox(this, wxID_ANY, "WIN");
	wxStaticText *LCDC_WinAddrText = new wxStaticText(this, wxID_ANY, "9800-9BFF");
	LCDCGridSizer->Add(m_LCDCtrlCheckboxes.WinAddr);
	LCDCGridSizer->Add(LCDC_WinAddrText);
	m_LCDCtrlCheckboxes.WinOff = new wxCheckBox(this, wxID_ANY, "WIN");
	wxStaticText *LCDC_WinOffText = new wxStaticText(this, wxID_ANY, "Off");
	LCDCGridSizer->Add(m_LCDCtrlCheckboxes.WinOff);
	LCDCGridSizer->Add(LCDC_WinOffText);
	m_LCDCtrlCheckboxes.ChrAddr = new wxCheckBox(this, wxID_ANY, "CHR");
	wxStaticText *LCDC_ChrAddrText = new wxStaticText(this, wxID_ANY, "8800-97FF");
	LCDCGridSizer->Add(m_LCDCtrlCheckboxes.ChrAddr);
	LCDCGridSizer->Add(LCDC_ChrAddrText);
	m_LCDCtrlCheckboxes.BgAddr = new wxCheckBox(this, wxID_ANY, "BG");
	wxStaticText *LCDC_BgAddrText = new wxStaticText(this, wxID_ANY, "9800-9BFF");
	LCDCGridSizer->Add(m_LCDCtrlCheckboxes.BgAddr);
	LCDCGridSizer->Add(LCDC_BgAddrText);
	m_LCDCtrlCheckboxes.ObjSize = new wxCheckBox(this, wxID_ANY, "OBJ");
	wxStaticText *LCDC_SpriteSizeText = new wxStaticText(this, wxID_ANY, "8x8");
	LCDCGridSizer->Add(m_LCDCtrlCheckboxes.ObjSize);
	LCDCGridSizer->Add(LCDC_SpriteSizeText);
	m_LCDCtrlCheckboxes.ObjOff = new wxCheckBox(this, wxID_ANY, "OBJ");
	wxStaticText *LCDC_SpriteOnText = new wxStaticText(this, wxID_ANY, "Off");
	LCDCGridSizer->Add(m_LCDCtrlCheckboxes.ObjOff);
	LCDCGridSizer->Add(LCDC_SpriteOnText);
	m_LCDCtrlCheckboxes.BGOn = new wxCheckBox(this, wxID_ANY, "BG");
	wxStaticText *LCDC_BGOnText = new wxStaticText(this, wxID_ANY, "Off");
	LCDCGridSizer->Add(m_LCDCtrlCheckboxes.BGOn);
	LCDCGridSizer->Add(LCDC_BGOnText);
	LCDCStaticBoxSizer->Add(LCDCGridSizer);

	SecondRowHSizer->Add(LCDCStaticBoxSizer, wxSizerFlags().Expand().Border(wxRIGHT));

	m_LCDStatWidgets.LYCInt = new wxCheckBox(this, wxID_ANY, "b6 LY=LYC int");
	LCDSTATGridSizer->Add(m_LCDStatWidgets.LYCInt);
	m_LCDStatWidgets.OAMInt = new wxCheckBox(this, wxID_ANY, "b5 OAM (Mode 2) int");
	LCDSTATGridSizer->Add(m_LCDStatWidgets.OAMInt);
	m_LCDStatWidgets.VBlankInt = new wxCheckBox(this, wxID_ANY, "b4 V-Blank (Mode 1) int");
	LCDSTATGridSizer->Add(m_LCDStatWidgets.VBlankInt);
	m_LCDStatWidgets.HBlankInt = new wxCheckBox(this, wxID_ANY, "b3 H-Blank (Mode 0) int");
	LCDSTATGridSizer->Add(m_LCDStatWidgets.HBlankInt);
	m_LCDStatWidgets.LyLyc = new wxCheckBox(this, wxID_ANY, "b2 LY=LYC");
	LCDSTATGridSizer->Add(m_LCDStatWidgets.LyLyc);
	wxStaticText *LCDStatModeLabel = new wxStaticText(this, wxID_ANY, "Mode");
	m_LCDStatWidgets.Mode = new wxTextCtrl(this, wxID_ANY, "0");
	m_LCDStatWidgets.Mode->SetMinSize(wxSize(20, 20));
	wxBoxSizer *LCDSTATModeHSizer = new wxBoxSizer(wxHORIZONTAL);
	LCDSTATModeHSizer->Add(m_LCDStatWidgets.Mode);
	LCDSTATModeHSizer->Add(LCDStatModeLabel, wxSizerFlags().CenterVertical().Border(wxLEFT));
	LCDSTATGridSizer->Add(LCDSTATModeHSizer);
	LCDSTATStaticBoxSizer->Add(LCDSTATGridSizer);

	SecondRowHSizer->Add(LCDSTATStaticBoxSizer, wxSizerFlags().Expand());

	m_WavePatternA = new wxTextCtrl(this, wxID_ANY, "00000000");
	m_WavePatternA->SetMinSize(wxSize(65, -1));
	m_WavePatternB = new wxTextCtrl(this, wxID_ANY, "00000000");
	m_WavePatternB->SetMinSize(wxSize(65, -1));
	m_WavePatternC = new wxTextCtrl(this, wxID_ANY, "00000000");
	m_WavePatternC->SetMinSize(wxSize(65, -1));
	m_WavePatternD = new wxTextCtrl(this, wxID_ANY, "00000000");
	m_WavePatternD->SetMinSize(wxSize(65, -1));

	WavePatternBoxGridSizer->Add(m_WavePatternA);
	WavePatternBoxGridSizer->Add(m_WavePatternB);
	WavePatternBoxGridSizer->Add(m_WavePatternC);
	WavePatternBoxGridSizer->Add(m_WavePatternD);
	WavePatternBoxSizer->Add(WavePatternBoxGridSizer);
	
	m_IEIFWidgets.IF_VblankCheckbox = new wxCheckBox(this, wxID_ANY, "");
	IFIEGridSizer->Add(m_IEIFWidgets.IF_VblankCheckbox, wxSizerFlags().CenterVertical());
	m_IEIFWidgets.IE_VblankCheckbox = new wxCheckBox(this, wxID_ANY, "");
	IFIEGridSizer->Add(m_IEIFWidgets.IE_VblankCheckbox, wxSizerFlags().CenterVertical());
	wxStaticText *VblankText = new wxStaticText(this, wxID_ANY, "40 V-Blank");
	IFIEGridSizer->Add(VblankText, wxSizerFlags().CenterVertical());
	m_IEIFWidgets.VblankTextCtrl = new wxTextCtrl(this, wxID_ANY, "-");
	m_IEIFWidgets.VblankTextCtrl->SetMinSize(wxSize(65, -1));
	IFIEGridSizer->Add(m_IEIFWidgets.VblankTextCtrl);
	m_IEIFWidgets.IF_LCDCheckbox = new wxCheckBox(this, wxID_ANY, "");
	IFIEGridSizer->Add(m_IEIFWidgets.IF_LCDCheckbox, wxSizerFlags().CenterVertical());
	m_IEIFWidgets.IE_LCDCheckbox = new wxCheckBox(this, wxID_ANY, "");
	IFIEGridSizer->Add(m_IEIFWidgets.IE_LCDCheckbox, wxSizerFlags().CenterVertical());
	wxStaticText *LCDText = new wxStaticText(this, wxID_ANY, "48 LCD");
	IFIEGridSizer->Add(LCDText, wxSizerFlags().CenterVertical());
	m_IEIFWidgets.LCDTextCtrl = new wxTextCtrl(this, wxID_ANY, "-");
	m_IEIFWidgets.LCDTextCtrl->SetMinSize(wxSize(65, -1));
	IFIEGridSizer->Add(m_IEIFWidgets.LCDTextCtrl);
	m_IEIFWidgets.IF_TimerCheckbox = new wxCheckBox(this, wxID_ANY, "");
	IFIEGridSizer->Add(m_IEIFWidgets.IF_TimerCheckbox, wxSizerFlags().CenterVertical());
	m_IEIFWidgets.IE_TimerCheckbox = new wxCheckBox(this, wxID_ANY, "");
	IFIEGridSizer->Add(m_IEIFWidgets.IE_TimerCheckbox, wxSizerFlags().CenterVertical());
	wxStaticText *TimerText = new wxStaticText(this, wxID_ANY, "50 Timer");
	IFIEGridSizer->Add(TimerText, wxSizerFlags().CenterVertical());
	m_IEIFWidgets.TimerTextCtrl = new wxTextCtrl(this, wxID_ANY, "-");
	m_IEIFWidgets.TimerTextCtrl->SetMinSize(wxSize(65, -1));
	IFIEGridSizer->Add(m_IEIFWidgets.TimerTextCtrl);
	m_IEIFWidgets.IF_SerialCheckbox = new wxCheckBox(this, wxID_ANY, "");
	IFIEGridSizer->Add(m_IEIFWidgets.IF_SerialCheckbox, wxSizerFlags().CenterVertical());
	m_IEIFWidgets.IE_SerialCheckbox = new wxCheckBox(this, wxID_ANY, "");
	IFIEGridSizer->Add(m_IEIFWidgets.IE_SerialCheckbox, wxSizerFlags().CenterVertical());
	wxStaticText *SerialText = new wxStaticText(this, wxID_ANY, "58 Serial");
	IFIEGridSizer->Add(SerialText, wxSizerFlags().CenterVertical());
	m_IEIFWidgets.SerialTextCtrl = new wxTextCtrl(this, wxID_ANY, "-");
	m_IEIFWidgets.SerialTextCtrl->SetMinSize(wxSize(65, -1));
	IFIEGridSizer->Add(m_IEIFWidgets.SerialTextCtrl);
	m_IEIFWidgets.IF_JoypadCheckbox = new wxCheckBox(this, wxID_ANY, "");
	IFIEGridSizer->Add(m_IEIFWidgets.IF_JoypadCheckbox, wxSizerFlags().CenterVertical());
	m_IEIFWidgets.IE_JoypadCheckbox = new wxCheckBox(this, wxID_ANY, "");
	IFIEGridSizer->Add(m_IEIFWidgets.IE_JoypadCheckbox, wxSizerFlags().CenterVertical());
	wxStaticText *JoypadText = new wxStaticText(this, wxID_ANY, "60 Joypad");
	IFIEGridSizer->Add(JoypadText, wxSizerFlags().CenterVertical());
	m_IEIFWidgets.JoypadTextCtrl = new wxTextCtrl(this, wxID_ANY, "0");
	m_IEIFWidgets.JoypadTextCtrl->SetMinSize(wxSize(65, -1));
	IFIEGridSizer->Add(m_IEIFWidgets.JoypadTextCtrl);
	IFIEStaticBoxSizer->Add(IFIEGridSizer);

	SecondRowHSizer->Add(IFIEStaticBoxSizer, wxSizerFlags().Border(wxLEFT).Expand());

	wxStaticText *MBCROMText = new wxStaticText(this, wxID_ANY, "ROM");
	wxStaticText *MBCRAMText = new wxStaticText(this, wxID_ANY, "RAM");
	m_MBCRomTextCtrl = new wxTextCtrl(this, wxID_ANY, "1");
	m_MBCRomTextCtrl->SetMinSize(wxSize(40, -1));
	m_MBCRamTextCtrl = new wxTextCtrl(this, wxID_ANY, "0");
	m_MBCRamTextCtrl->SetMinSize(wxSize(40, -1));
	MBCGridSizer->Add(m_MBCRomTextCtrl);
	MBCGridSizer->Add(MBCROMText, wxSizerFlags().CenterVertical());
	MBCGridSizer->Add(m_MBCRamTextCtrl);
	MBCGridSizer->Add(MBCRAMText, wxSizerFlags().CenterVertical());
	MBCStaticBoxSizer->Add(MBCGridSizer);
	MBCAndRefreshButtonVSizer->Add(MBCStaticBoxSizer);

	wxBitmap RestartBitmap = wxBITMAP_PNG_FROM_DATA(refreshio);
	wxBitmapButton *RefreshIOMapButton = new wxBitmapButton(this, wxID_ANY, RestartBitmap);
	RefreshIOMapButton->Bind(wxEVT_BUTTON, &IOMap::OnRefreshButtonClick, this);
	RefreshIOMapButton->SetToolTip("Run");
	RefreshIOMapButton->SetFocus();
	MBCAndRefreshButtonVSizer->Add(RefreshIOMapButton, wxSizerFlags().Border(wxUP).Center());


	SecondRowHSizer->Add(MBCAndRefreshButtonVSizer, wxSizerFlags().Border(wxLEFT).Expand());

	LCDVBoxSizer->Add(LCDGridSizer);
	MiscBoxSizer->Add(MiscEntryGridSizer);

	Sound1BoxSizer->Add(Sound1GridSizer);
	Sound2BoxSizer->Add(Sound2GridSizer);
	Sound3BoxSizer->Add(Sound3GridSizer);
	Sound4BoxSizer->Add(Sound4GridSizer);
	SoundGridSizer->Add(Sound1BoxSizer, wxSizerFlags().Expand());
	SoundGridSizer->Add(Sound2BoxSizer, wxSizerFlags().Expand());
	SoundGridSizer->Add(Sound3BoxSizer, wxSizerFlags().Expand());
	SoundGridSizer->Add(Sound4BoxSizer, wxSizerFlags().Expand());

	SoundControlBoxSizer->Add(SoundControlGridSizer);

	SoundVSizer->Add(SoundControlBoxSizer, wxSizerFlags().Expand());
	SoundVSizer->Add(WavePatternBoxSizer);
	SoundGridSizer->Add(SoundVSizer);

	RightHalfVSizer->Add(SoundGridSizer, wxSizerFlags().Expand());
	RightHalfVSizer->Add(SecondRowHSizer, wxSizerFlags().Expand());
	
	MainHBoxSizer->Add(LCDVBoxSizer, wxSizerFlags().Border(wxLEFT | wxDOWN));
	MainHBoxSizer->Add(MiscBoxSizer, wxSizerFlags().Border(wxLEFT | wxDOWN));
	MainHBoxSizer->Add(RightHalfVSizer, wxSizerFlags().Border(wxLEFT | wxRIGHT).Expand());

	SetSizer(MainHBoxSizer);
	MainHBoxSizer->Fit(this);
}

void IOMap::SetEmulator(EmulatorEngine *Emulator)
{
	m_Emulator = Emulator;
}

void IOMap::OnRefreshButtonClick(wxCommandEvent& event)
{
	RefreshValues();
}

void IOMap::RefreshValues()
{
	if (m_Emulator != nullptr)
	{
		GBMMU *MMU = m_Emulator->GetMMU();

		for (int EntryIter = 0; EntryIter < NUM_IO_ENTRIES; EntryIter++)
		{
			m_IOEntries[EntryIter].TextCtrl->SetValue(wxString::Format(wxT("%02X"), MMU->ReadMemory8(m_IOEntries[EntryIter].Address)));
		}

		byte LCDStat = MMU->ReadMemory8(STAT_REGISTER);
		m_LCDStatWidgets.LYCInt->SetValue(M_TestBit(LCDStat, BIT_6));
		m_LCDStatWidgets.OAMInt->SetValue(M_TestBit(LCDStat, BIT_5));
		m_LCDStatWidgets.VBlankInt->SetValue(M_TestBit(LCDStat, BIT_4));
		m_LCDStatWidgets.HBlankInt->SetValue(M_TestBit(LCDStat, BIT_3));
		m_LCDStatWidgets.LyLyc->SetValue(M_TestBit(LCDStat, BIT_2));
		m_LCDStatWidgets.Mode->SetValue(wxString::Format(wxT("%X"), (LCDStat & 0x3)));

		byte LCDControl = MMU->ReadMemory8(LCDC_REGISTER);
		m_LCDCtrlCheckboxes.LCDOff->SetValue(M_TestBit(LCDControl, BIT_7));
		m_LCDCtrlCheckboxes.WinAddr->SetValue(M_TestBit(LCDControl, BIT_6));
		m_LCDCtrlCheckboxes.WinOff->SetValue(M_TestBit(LCDControl, BIT_5));
		m_LCDCtrlCheckboxes.ChrAddr->SetValue(M_TestBit(LCDControl, BIT_4));
		m_LCDCtrlCheckboxes.BgAddr->SetValue(M_TestBit(LCDControl, BIT_3));
		m_LCDCtrlCheckboxes.ObjSize->SetValue(M_TestBit(LCDControl, BIT_2));
		m_LCDCtrlCheckboxes.ObjOff->SetValue(M_TestBit(LCDControl, BIT_1));
		m_LCDCtrlCheckboxes.BGOn->SetValue(M_TestBit(LCDControl, BIT_0));

		m_MBCRamTextCtrl->SetValue(wxString::Format(wxT("%X"), MMU->GetRamBankNumber()));
		m_MBCRomTextCtrl->SetValue(wxString::Format(wxT("%X"), MMU->GetRomBankNumber()));

		byte InterruptsEnabled = MMU->ReadMemory8(IE_REGISTER);
		byte InterruptsRequested = MMU->ReadMemory8(IF_REGISTER);
		m_IEIFWidgets.IF_VblankCheckbox->SetValue(M_TestBit(InterruptsRequested, VBLANK_INTERRUPT));
		m_IEIFWidgets.IF_LCDCheckbox->SetValue(M_TestBit(InterruptsRequested, LCD_STAT_INTERRUPT));
		m_IEIFWidgets.IF_TimerCheckbox->SetValue(M_TestBit(InterruptsRequested, TIMER_INTERRUPT));
		m_IEIFWidgets.IF_SerialCheckbox->SetValue(M_TestBit(InterruptsRequested, SERIAL_INTERRUPT));
		m_IEIFWidgets.IF_JoypadCheckbox->SetValue(M_TestBit(InterruptsRequested, JOYPAD_INTERRUPT));
		m_IEIFWidgets.IE_VblankCheckbox->SetValue(M_TestBit(InterruptsEnabled, VBLANK_INTERRUPT));
		m_IEIFWidgets.IE_LCDCheckbox->SetValue(M_TestBit(InterruptsEnabled, LCD_STAT_INTERRUPT));
		m_IEIFWidgets.IE_TimerCheckbox->SetValue(M_TestBit(InterruptsEnabled, TIMER_INTERRUPT));
		m_IEIFWidgets.IE_SerialCheckbox->SetValue(M_TestBit(InterruptsEnabled, SERIAL_INTERRUPT));
		m_IEIFWidgets.IE_JoypadCheckbox->SetValue(M_TestBit(InterruptsEnabled, JOYPAD_INTERRUPT));

		m_WavePatternA->SetValue(wxString::Format(wxT("%02X%02X%02X%02X"), MMU->ReadMemory8(0xFF30), MMU->ReadMemory8(0xFF31), MMU->ReadMemory8(0xFF32), MMU->ReadMemory8(0xFF33)));
		m_WavePatternB->SetValue(wxString::Format(wxT("%02X%02X%02X%02X"), MMU->ReadMemory8(0xFF34), MMU->ReadMemory8(0xFF35), MMU->ReadMemory8(0xFF36), MMU->ReadMemory8(0xFF37)));
		m_WavePatternC->SetValue(wxString::Format(wxT("%02X%02X%02X%02X"), MMU->ReadMemory8(0xFF38), MMU->ReadMemory8(0xFF39), MMU->ReadMemory8(0xFF3A), MMU->ReadMemory8(0xFF3B)));
		m_WavePatternD->SetValue(wxString::Format(wxT("%02X%02X%02X%02X"), MMU->ReadMemory8(0xFF3C), MMU->ReadMemory8(0xFF3D), MMU->ReadMemory8(0xFF3E), MMU->ReadMemory8(0xFF3F)));
	}
}

IOMap::~IOMap()
{
}
