from neutrino import *

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
	listWidget = ClistBoxWidget("pythonTest:ClistBoxWidget", NEUTRINO_ICON_MOVIE)

	def __init__(self):
		CMenuTarget.__init__
		self.showMenu()
		self._exec = self.onAction(None, "")
		self.hide = self.unPaint()

	def showMenu(self):
		print("showMenu")
		
		self.listWidget.setSelected(self.selected)
		self.listWidget.setMode(MODE_MENU)
		self.listWidget.enableShrinkMenu()
		self.listWidget.enableWidgetChange()
		self.listWidget.enablePaintFootInfo()

		item1 = CMenuForwarder("messageBox", True, "", None, "red action")
		item1.setItemIcon(DATADIR + "/neutrino/icons/plugin.png")
		item1.setHelpText("testing CMessageBox")

		item2 = ClistBoxItem("item2", True, "", None, "green action")
		item2.setItemIcon(DATADIR + "/neutrino/icons/plugin.png")

		item3 = ClistBoxItem("moviePlayer")
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
			#return menu_return().RETURN_REPAINT
			#self.unPaint()
			#self.showMenu()
			#self.onAction()

		elif self.selected == 1:
			return menu_return().RETURN_REPAINT

		elif self.selected == 2:
			moviePlayer()
			return menu_return().RETURN_REPAINT

if __name__ == "__main__":
	testMenu()





















