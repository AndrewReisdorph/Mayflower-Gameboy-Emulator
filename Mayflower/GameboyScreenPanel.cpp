#include <wx/dcbuffer.h>
#include "GameboyScreenPanel.h"



GameboyScreenPanel::GameboyScreenPanel(wxFrame *parent) :wxPanel(parent, wxID_ANY)
{
	this->Connect(wxEVT_PAINT, wxPaintEventHandler(GameboyScreenPanel::OnPaint));
	SetBackgroundStyle(wxBG_STYLE_PAINT);
	SetMinSize(wxSize(160,144));

}

void GameboyScreenPanel::OnPaint(wxPaintEvent& event)
{
	wxAutoBufferedPaintDC dc(this);
	int buffer_iter = 0;
	wxPen CurrentPen = wxPen(wxColour(15, 56, 15));
	dc.SetPen(CurrentPen);

	for (int row = 0; row < 144; row++)
	{
		for (int col = 0; col < 160; col++)
		{
			switch (m_Emulator->GetLCD()->GetScreenBuffer()[++buffer_iter])
			{
			case 3:
				CurrentPen.SetColour(wxColour(15, 56, 15));
				break;
			case 2:
				CurrentPen.SetColour(wxColour(48, 98, 48));
				break;
			case 1:
				CurrentPen.SetColour(wxColour(139, 172, 15));
				break;
			case 0:
				CurrentPen.SetColour(wxColour(155, 188, 15));
				break;
			default:
				break;
				//std::cout << "me stupid" << std::endl;
				//throw std::exception();
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
