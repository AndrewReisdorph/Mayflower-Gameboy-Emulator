#pragma once
#include <wx/wx.h>
#include "MayflowerWindow.h"

typedef unsigned char byte;

class MayflowerWindow;

class MayflowerApp : public wxApp
{
private:
	MayflowerWindow *m_MayflowerWindow;

public:
	bool OnInit();
};

enum
{
	ID_OpenROM = wxID_HIGHEST + 1,
	ID_Quit,
	ID_About,
	ID_Step,
	ID_VramView
};