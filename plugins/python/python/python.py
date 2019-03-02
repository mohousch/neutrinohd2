from neutrino import CMenuTarget
from neutrino import menu_return
from neutrino import CMessageBox, CHelpBox, CInfoBox, CHintBox, CStringInput, CStringInputSMS, CPINInput, CProgressWindow
from neutrino import ClistBox, CHeaders, CWindow, CTextBox, CListFrame, CProgressBar
from neutrino import ClistBoxWidget, CMenuWidget
from neutrino import CMenuForwarder, ClistBoxItem, ClistBoxEntryItem
from neutrino import CFrameBuffer, CRCInput
from neutrino import cPlayback
from neutrino import CAudioPlayerGui, CMoviePlayerGui, CPictureViewerGui
from neutrino import CPlugins
from neutrino import CBox, CFile, CSwigHelpers
from neutrino import DATADIR, CONFIGDIR
from neutrino import WIDGET_TYPE_CLASSIC, WIDGET_TYPE_FRAME
from neutrino import NEUTRINO_ICON_BUTTON_RED, NEUTRINO_ICON_BUTTON_GREEN, NEUTRINO_ICON_BUTTON_YELLOW, NEUTRINO_ICON_BUTTON_BLUE, NEUTRINO_ICON_MOVIE, NEUTRINO_ICON_PLUGIN, NEUTRINO_ICON_BUTTON_HELP
from neutrino import RC_nokey, RC_red, RC_green, RC_yellow, RC_blue, RC_home, RC_info
from neutrino import CFileBrowser, CFileFilter
from neutrino import SNeutrinoSettings
from neutrino import button_label_struct
from neutrino import NONEXISTANT_LOCALE
from neutrino import NeutrinoMessages

class main(CStringInputSMS):
	title = "pythonTest: CStringInputSMS"
	value = ''
	def __init__(self):
		CStringInputSMS.__init__(self, self.title, self.value)
		self._exec(None, "")

if __name__ == "__main__":
	main()










