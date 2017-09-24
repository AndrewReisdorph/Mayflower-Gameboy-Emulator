#include "main.h"

wxIMPLEMENT_APP_CONSOLE(MayflowerApp);

bool MayflowerApp::OnInit()
{
	m_MayflowerWindow = new MayflowerWindow();
	bool result = m_MayflowerWindow->Show();
	return true;
}