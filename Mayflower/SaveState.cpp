#include "SaveState.h"

using namespace std;

SaveState::SaveState(wxString FilePath)
{
	m_DataMap["null"] = {nullptr, 0};
	ParseSaveStateFile(FilePath);
}

void SaveState::ParseSaveStateFile(wxString FilePath)
{
	wxFile SaveStateFile(FilePath);
	unsigned long FileSize = wxFileName::GetSize(FilePath).ToULong();
	byte *SaveStateData = new byte[FileSize];
	char KeyNameBuffer[32] = { 0 };
	SaveStateFile.Read(SaveStateData, FileSize);
	SaveStateFile.Close();
	int DataIndex = 0;
	uint32 NextDataSize = 0;

	// File starts with 0xB bytes of application name
	m_DataMap["app"] = {new byte[0xB], 0xB};
	memcpy(m_DataMap["app"].Buffer, SaveStateData, 0xB);
	DataIndex = 0xB;

	while (true)
	{
		// Read and store next key value pair
		strcpy(KeyNameBuffer, (char *)&SaveStateData[DataIndex]);
		DataIndex += strlen(KeyNameBuffer) + 1;

		NextDataSize = *((uint32 *)(&SaveStateData[DataIndex]));
		DataIndex += sizeof(uint32);

		byte *NextPayload = new byte[NextDataSize];
		memcpy(NextPayload, &SaveStateData[DataIndex], NextDataSize);
		DataIndex += NextDataSize;

		m_DataMap[wxString(KeyNameBuffer)] = { NextPayload, NextDataSize };

		if (DataIndex >= FileSize)
		{
			break;
		}
	}

	delete SaveStateData;
}

state_entry &SaveState::Get(wxString Key)
{
	if (m_DataMap.find(Key) != m_DataMap.end())
	{
		return m_DataMap[Key];
	}

	return m_DataMap["null"];
}

SaveState::~SaveState()
{
	for (std::map<wxString, state_entry>::iterator it = m_DataMap.begin(); it != m_DataMap.end(); ++it)
	{
		delete it->second.Buffer;
	}
	m_DataMap.clear();
}
