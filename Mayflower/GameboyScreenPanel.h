#pragma once
#include <wx/wx.h>
#include "MayflowerWindow.h"
#include "EmulatorEngine.h"

class EmulatorEngine;

class GameboyScreenPanel : public wxPanel
{
private:
	EmulatorEngine *m_Emulator;
	void OnPaint(wxPaintEvent& event);
	byte m_PixelData[23040];
public:
	void SetEmulator(EmulatorEngine *Emulator);
	void UpdateBuffer(byte *buffer);
	GameboyScreenPanel(wxFrame *parent);
	~GameboyScreenPanel();
};

