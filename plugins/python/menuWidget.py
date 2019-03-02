from neutrino import CMenuTarget
from neutrino import menu_return
from neutrino import CMessageBox, CHelpBox, CInfoBox, CHintBox, CTextBox
from neutrino import CHeaders, CWindow 
from neutrino import ClistBoxWidget, CMenuWidget, CMenuForwarder, ClistBoxItem
from neutrino import ClistBox, ClistBoxEntryItem
from neutrino import CFrameBuffer, CRCInput
from neutrino import cPlayback
from neutrino import CAudioPlayerGui, CMoviePlayerGui, CPictureViewerGui
from neutrino import CPlugins
from neutrino import CBox, CFile, CSwigHelpers
from neutrino import DATADIR
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

class moviePlayer(CMoviePlayerGui):
	mFile = "/home/mohousch/Videos/ProSieben_20150619_201430.ts" 
	def __init__(self):
		CMoviePlayerGui.__init__(self)
		self.addToPlaylist(self.mFile)
		self._exec(None, "")

class main(CMenuTarget):
	selected = 0
	def showMenu(self):
		print("showMenu")
		listWidget = CMenuWidget("pythonTest:CMenuWidget", NEUTRINO_ICON_MOVIE)

		listWidget.setSelected(self.selected)
		listWidget.setWidgetType(WIDGET_TYPE_CLASSIC)
		listWidget.enableWidgetChange()

		item1 = CMenuForwarder("item1", True, "", None, "red action")
		item2 = CMenuForwarder("item2")
		item3 = CMenuForwarder("item3")
		item4 = CMenuForwarder("item4")

		listWidget.addItem(item1)
		listWidget.addItem(item2)
		listWidget.addItem(item3)
		listWidget.addItem(item4)

		listWidget._exec(None, "")
		self.selected = listWidget.getSelected()

	def unPaint(self):
		print("unPaint")
		CFrameBuffer().getInstance().paintBackground()
		
	def onAction(self, parent, actionKey):

		print("_exec")
		print(actionKey)

		if(actionKey == "red action"):
			print("red action")

		if(actionKey == "green action"):
			print("green action")

		if(actionKey == "yellow action"):
			print("yellow action")

		if(actionKey == "blue action"):
			print("blue action")

		if(parent):
			CFrameBuffer().getInstance().paintBackground()

		print(self.selected)
		#print(self["actionKey"])

		if self.selected == 0:
			messageBox()
			#return menu_return().RETURN_REPAINT
			return self.onAction()

		if self.selected == 3:
			moviePlayer()
			#return menu_return().RETURN_REPAINT
			return self.onAction()

		self.showMenu()
		

	def __init__(self):
		#CMenuTarget.__init__(self)
		self.paint = self.showMenu()
		#self.hide = self.unPaint()
		self._exec = self.onAction(None, "")
	
	#def __getitem__(self, actionKey):
        #	print ("Inside `__getitem__` method!")
        #	return getattr(self, actionKey)
	#def __getitem__(self, key):
        	#actionKey = CMenuTarget.__getitem__(self, key)
        	#return actionKey

	#def __setitem__(self, key, actionKey):
        	#CMenutarget.__setitem__(self, key, actionKey)

if __name__ == "__main__":
	main()










