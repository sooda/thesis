#include "gfxui.h"
#include "gphotogrid.h"

bool GfxUi::OnInit()
{
	wxInitAllImageHandlers();
	// note that frames/panes are owned by wx, not leaked
	wxFrame *mainframe = new MainFrame("Hello World", app);
	mainframe->Show(true);
	return true;
}

wxIMPLEMENT_APP(GfxUi);
