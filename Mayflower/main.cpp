#include "main.h"
#include "MayflowerWindow.h"

wxIMPLEMENT_APP_CONSOLE(MayflowerApp);

bool MayflowerApp::OnInit()
{
	wxFrame* window = new MayflowerWindow();
	bool result = window->Show();
	return true;
}