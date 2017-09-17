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
	byte *m_ImageData;
	wxImage *m_ScreenImage;
	void BuildScreenImage();
public:
	void SetEmulator(EmulatorEngine *Emulator);
	GameboyScreenPanel(wxFrame *parent);
	~GameboyScreenPanel();
};

enum color_scheme
{
	COLOR_TRUE = 0,
	COLOR_BGB,
	NUM_COLOR_SCHEMES
};

struct gb_color
{
	byte Red;
	byte Green;
	byte Blue;
};

struct palette
{
	gb_color Darkest;
	gb_color Dark;
	gb_color Light;
	gb_color Lightest;
};

static const palette ColorPalettes[NUM_COLOR_SCHEMES] = {
	{ { 15, 56, 15}, {49,  98, 48}, {139, 172,  15}, {155, 188,  15} }, // COLOR_TRUE
	{ {  8, 24, 32}, {52, 104, 86}, {136, 192, 112}, {224, 248, 208} }  // COLOR_BGB
};