#include <wx/dcbuffer.h>
#include "GameboyScreenPanel.h"

GameboyScreenPanel::GameboyScreenPanel(wxFrame *parent) :wxPanel(parent, wxID_ANY)
{
	this->Connect(wxEVT_PAINT, wxPaintEventHandler(GameboyScreenPanel::OnPaint));
	SetBackgroundStyle(wxBG_STYLE_PAINT);
	SetMinSize(wxSize(SCREEN_WIDTH, SCREEN_HEIGHT));

	m_ImageData = (unsigned char*)calloc(SCREEN_WIDTH * SCREEN_HEIGHT * 3, 1);
	m_ScreenImage = new wxImage(SCREEN_WIDTH, SCREEN_HEIGHT, m_ImageData);
}

void GameboyScreenPanel::BuildScreenImage()
{
	byte *ScreenBuffer = m_Emulator->GetLCD()->GetScreenBuffer();
	int BufferIter = 0;
	int ImageDataIter = 0;
	int ColorScheme = COLOR_BGB;
	gb_color CurrentColor = ColorPalettes[ColorScheme].Lightest;

	for (int row = 0; row < SCREEN_HEIGHT; row++)
	{
		for (int col = 0; col < SCREEN_WIDTH; col++)
		{
			switch (ScreenBuffer[BufferIter++])
			{
			case 0:
				CurrentColor = ColorPalettes[ColorScheme].Lightest;
				break;
			case 1:
				CurrentColor = ColorPalettes[ColorScheme].Light;
				break;
			case 2:
				CurrentColor = ColorPalettes[ColorScheme].Dark;
				break;
			case 3:
				CurrentColor = ColorPalettes[ColorScheme].Darkest;
				break;
			default:
				break;
			}
			m_ImageData[ImageDataIter++] = CurrentColor.Red;
			m_ImageData[ImageDataIter++] = CurrentColor.Green;
			m_ImageData[ImageDataIter++] = CurrentColor.Blue;
		}
	}

}

void GameboyScreenPanel::OnPaint(wxPaintEvent& event)
{
	wxAutoBufferedPaintDC dc(this);
	dc.SetBrush(wxBrush(wxColour(0, 0, 0, 0), wxBRUSHSTYLE_SOLID));
	dc.Clear();
	if (m_Emulator->LCDFrameReady())
	{
		m_Emulator->ClearFrameReady();
		BuildScreenImage();
	}
	dc.DrawBitmap(wxBitmap(*m_ScreenImage), wxPoint(0, 0));

}

void GameboyScreenPanel::SetEmulator(EmulatorEngine *Emulator)
{
	m_Emulator = Emulator;
}

GameboyScreenPanel::~GameboyScreenPanel()
{
}
