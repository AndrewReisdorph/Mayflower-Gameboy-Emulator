#pragma once
#include <wx/wx.h>

typedef unsigned char byte;

class MayflowerApp : public wxApp
{
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