#pragma once
#include <map>
#include <wx/wx.h>
#include <wx/filename.h>
#include <wx/longlong.h>
#include "GBTypes.h"

typedef struct state_entry
{
	byte *Buffer;
	uint32 Size;
}state_entry;

class SaveState
{
private:
	std::map<wxString, state_entry> m_DataMap;
public:
	state_entry &Get(wxString);
	void ParseSaveStateFile(wxString FilePath);
	SaveState(wxString FilePath);
	~SaveState();
};

