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

class messageBox(CMessageBox):
	title = "pythonTest"
	msg = "TEST"
	def __init__(self):
		CMessageBox.__init__(self, self.title, self.msg)
		self._exec(-1)

class moviePlayer(CFileBrowser):
	settings = SNeutrinoSettings()
	PATH = settings.network_nfs_moviedir
	def __init__(self):
		CFileBrowser.__init__(self)
		fileFilter = CFileFilter()
		fileFilter.addFilter("ts")

		self.Multi_Select = False
		self.Dirs_Selectable = False
		self.Filter = fileFilter

		self._exec(self.PATH)
		
		cFile = self.getSelectedFile()

		player = CMoviePlayerGui()
	
		player.addToPlaylist(cFile)
		player._exec(None, "")

class testMenu(CMenuTarget):
	selected = 0
	listWidget = CMenuWidget("pythonTest:CMenuWidget", NEUTRINO_ICON_MOVIE)
	def showMenu(self):
		print("showMenu")
		
		self.listWidget.setSelected(self.selected)
		self.listWidget.setWidgetType(WIDGET_TYPE_CLASSIC)
		self.listWidget.enableWidgetChange()
		self.listWidget.enablePaintFootInfo()

		item1 = CMenuForwarder("messageBox", True, "", None, "red action")
		item1.setItemIcon(DATADIR + "/neutrino/icons/plugin.png")
		item1.setHelpText("testing CMessageBox")

		item2 = ClistBoxItem("item2", True, "", None, "green action")
		item2.setItemIcon(DATADIR + "/neutrino/icons/plugin.png")

		item3 = ClistBoxEntryItem("moviePlayer")
		item3.setItemIcon(DATADIR + "/neutrino/icons/plugin.png")
		item3.setHelpText("testing CMoviePlayerGui")

		self.listWidget.addItem(item1)
		self.listWidget.addItem(item2)
		self.listWidget.addItem(item3)

		self.listWidget._exec(None, "")

	def unPaint(self):
		print("unPaint")
		CSwigHelpers().paintBackground()
		CSwigHelpers().blit()
		
	def onAction(self, parent, actionKey):

		print("_exec")
		print(actionKey)

		self.selected = self.listWidget.getSelected()

		if(parent):
			CSwigHelpers().paintBackground()
			CSwigHelpers().blit()

		print(self.selected)

		if self.selected == 0:
			messageBox()
			return menu_return().RETURN_REPAINT

		elif self.selected == 1:
			return menu_return().RETURN_REPAINT

		elif self.selected == 2:
			moviePlayer()
			return menu_return().RETURN_REPAINT
		
	def __init__(self):
		#CMenuTarget.__init__(self, session)
		self.paint = self.showMenu()
		self._exec = self.onAction(None, "")
		self.hide = self.unPaint()

if __name__ == "__main__":
	testMenu()

#def main(session, **kwargs):
#	session.open(test)




















