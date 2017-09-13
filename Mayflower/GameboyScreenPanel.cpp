#include <wx/dcbuffer.h>
#include "GameboyScreenPanel.h"

GameboyScreenPanel::GameboyScreenPanel(wxFrame *parent) :wxPanel(parent, wxID_ANY)
{
	this->Connect(wxEVT_PAINT, wxPaintEventHandler(GameboyScreenPanel::OnPaint));
	SetBackgroundStyle(wxBG_STYLE_PAINT);
	SetMinSize(wxSize(SCREEN_WIDTH, SCREEN_HEIGHT));
}

void GameboyScreenPanel::OnPaint(wxPaintEvent& event)
{
	wxAutoBufferedPaintDC dc(this);
	int BufferIter = 0;
	int ColorScheme = COLOR_BGB;
	byte *ScreenBuffer = m_Emulator->GetLCD()->GetScreenBuffer();
	wxPen CurrentPen = wxPen(ColorPalettes[ColorScheme].Lightest);
	dc.SetPen(CurrentPen);

	for (int row = 0; row < SCREEN_HEIGHT; row++)
	{
		for (int col = 0; col < SCREEN_WIDTH; col++)
		{
			switch (ScreenBuffer[BufferIter++])
			{
			case 0:
				CurrentPen.SetColour(ColorPalettes[ColorScheme].Lightest);
				break;
			case 1:
				CurrentPen.SetColour(ColorPalettes[ColorScheme].Light);
				break;
			case 2:
				CurrentPen.SetColour(ColorPalettes[ColorScheme].Dark);
				break;
			case 3:
				CurrentPen.SetColour(ColorPalettes[ColorScheme].Darkest);
				break;
			default:
				break;
			}
			dc.SetPen(CurrentPen);
			dc.DrawPoint(col, row);
		}
	}
}

void GameboyScreenPanel::SetEmulator(EmulatorEngine *Emulator)
{
	m_Emulator = Emulator;
}

GameboyScreenPanel::~GameboyScreenPanel()
{
}
