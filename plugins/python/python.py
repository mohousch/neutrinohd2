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

		self.PATH = self.getCurrentDir()

		player = CMoviePlayerGui()
	
		if self.getSelectedFile() is not None:
			player.addToPlaylist(self.getSelectedFile())
			player._exec(None, "")

		if self.getExitPressed() is not True:
			self.__init__()

class headers(CHeaders):
	def __init__(self):
		CHeaders.__init__(self)

		buttons = button_label_struct()
		buttons.button = NEUTRINO_ICON_BUTTON_HELP
		buttons.locale = NONEXISTANT_LOCALE
		buttons.localename = None

		fButtons = button_label_struct()
		fButtons.button = NEUTRINO_ICON_BUTTON_RED
		fButtons.locale = NONEXISTANT_LOCALE
		fButtons.localename = "red"
		
		self.enablePaintDate()
		self.enableLogo()

		self.setHeaderButtons(buttons, 1)

		self.paintHead(150, 50, 750, 40, "pythonTest", NEUTRINO_ICON_MOVIE)
		self.paintFoot(150, 570, 750, 40, 1, fButtons)

		CSwigHelpers().blit()

		SEC_TIMER_ID = 0
		SEC_TIMER_ID = CSwigHelpers().addTimer(1*1000*1000, False);

		msg = -1
		data = -1
		loop = True

		while loop:
			msg = CSwigHelpers().getRCCode()
			data = CSwigHelpers().getRCData()

			if msg == NeutrinoMessages().EVT_TIMER and data == SEC_TIMER_ID:
				self.paintHead(150, 50, 750, 40, "pythonTest", NEUTRINO_ICON_MOVIE)

			elif msg == RC_home:
				loop = False
				break

		CFrameBuffer.getInstance().paintBackground()

class helpBox(CHelpBox):
	line1 = "Test"
	line2 = "Huhu"
	line3 = "alles gut"
	def __init__(self):
		CHelpBox.__init__(self)
		self.addLine(self.line1)
		self.addLine(self.line2)
		self.addLine(self.line3)
		self.show("python: CHelpBox")

class hintBox(CHintBox):
	title = "python: CHintBox:"
	msg = "alles gut"
	def __init__(self):
		CHintBox.__init__(self, self.title, self.msg)
		self._exec()

class infoBox(CInfoBox):
	msgTitle = "pythonTest:"
	msgText = "first test\ntesting CInfoBox\nthat's all Folk!"
	def __init__(self):
		CInfoBox.__init__(self)
		self.setText(self.msgTitle + "\n" + self.msgText)
		self._exec(-1)

class stringInput(CStringInputSMS):
	title = "pythonTest: CStringInputSMS"
	value = ''
	def __init__(self):
		CStringInputSMS.__init__(self, self.title, self.value)
		self._exec(None, "")

class textBox(CTextBox):
	msgText = "first test\ntesting CMessageBox\nthat's all Folk!"
	def __init__(self):
		CTextBox.__init__(self)
		self.setText(self.msgText)
		self.paint()
		CFrameBuffer.getInstance().blit()

		msg = -1
		loop = True
		bigFonts = False

		while loop:
			msg = CSwigHelpers().getRCCode()
			#data = CSwigHelpers().getRCData()

			if msg == RC_info:
				if bigFonts == False : 
					bigFonts = True
				elif bigFonts == True:
					bigFonts = False
				self.setBigFonts(bigFonts)
				self.paint()

			if msg == RC_home:
				loop = False
				break

		self.hide()
		CFrameBuffer.getInstance().blit()

class audioPlayer(CFileBrowser):
	PATH = "/"
	def __init__(self):
		CFileBrowser.__init__(self)
		fileFilter = CFileFilter()
		fileFilter.addFilter("mp3")

		self.Multi_Select = False
		self.Dirs_Selectable = False
		self.Filter = fileFilter

		self._exec(self.PATH)

		self.PATH = self.getCurrentDir()

		player = CAudioPlayerGui()

		if self.getSelectedFile() is not None:
			player.addToPlaylist(self.getSelectedFile())
			player._exec(None, "")

		if self.getExitPressed() is not True:
			self.__init__()

class pictureViewer(CFileBrowser):
	PATH = "/"
	def __init__(self):
		CFileBrowser.__init__(self)
		fileFilter = CFileFilter()
		fileFilter.addFilter("jpeg")
		fileFilter.addFilter("jpg")

		self.Multi_Select = False
		self.Dirs_Selectable = False
		self.Filter = fileFilter

		self._exec(self.PATH)

		self.PATH = self.getCurrentDir()

		player = CPictureViewerGui()

		if self.getSelectedFile() is not None:
			player.addToPlaylist(self.getSelectedFile())
			player._exec(None, "")

		if self.getExitPressed() is not True:
			self.__init__()

class testMenu(CMenuTarget):
	selected = 0
	listWidget = ClistBoxWidget("pythonTest:ClistBoxWidget", NEUTRINO_ICON_MOVIE)

	def __init__(self):
		CMenuTarget.__init__
		self.showMenu()

	def showMenu(self):
		print("showMenu")
		
		#self.listWidget.setSelected(self.selected)
		self.listWidget.setWidgetType(WIDGET_TYPE_CLASSIC)
		self.listWidget.setMode(MODE_MENU)
		self.listWidget.enableShrinkMenu()
		#self.listWidget.enableWidgetChange()
		self.listWidget.enablePaintFootInfo()

		# messageBox
		item1 = CMenuForwarder("CMessageBox", True, "", None, "red action")
		item1.setItemIcon(DATADIR + "/neutrino/icons/plugin.png")
		item1.setHelpText("testing CMessageBox")

		# CFileBrowser | CMoviePlayerGui
		item2 = CMenuForwarder("CFileBrowser|CMoviePlayerGui")
		item2.setItemIcon(DATADIR + "/neutrino/icons/plugin.png")
		item2.setHelpText("testing CMoviePlayerGui")

		# CHeaders
		item3 = CMenuForwarder("CHeaders")
		item3.setItemIcon(DATADIR + "/neutrino/icons/plugin.png")
		item3.setHelpText("testing CHeaders")

		# CHelpBox
		item4 = CMenuForwarder("CHelpBox")
		item4.setItemIcon(DATADIR + "/neutrino/icons/plugin.png")
		item4.setHelpText("testing CHelpBox")

		# CHintBox
		item5 = CMenuForwarder("CHintBox")
		item5.setItemIcon(DATADIR + "/neutrino/icons/plugin.png")
		item5.setHelpText("testing CHintBox")

		# CInfoBox
		item6 = CMenuForwarder("CInfoBox")
		item6.setItemIcon(DATADIR + "/neutrino/icons/plugin.png")
		item6.setHelpText("testing CInfoBox")

		# CStringInput
		item7 = CMenuForwarder("CStringInput")
		item7.setItemIcon(DATADIR + "/neutrino/icons/plugin.png")
		item7.setHelpText("testing CStringInput")

		# CTextBox
		item8 = CMenuForwarder("CTextBox")
		item8.setItemIcon(DATADIR + "/neutrino/icons/plugin.png")
		item8.setHelpText("testing CTextBox")

		# CAudioPlayerGui
		item9 = CMenuForwarder("CAudioPlayerGui")
		item9.setItemIcon(DATADIR + "/neutrino/icons/plugin.png")
		item9.setHelpText("testing CAudioPlayerGui")

		# CPictureViewerGui
		item10 = CMenuForwarder("CPictureViewerGui")
		item10.setItemIcon(DATADIR + "/neutrino/icons/plugin.png")
		item10.setHelpText("testing CPictureViewerGui")

		self.listWidget.addItem(item1)
		self.listWidget.addItem(item2)
		self.listWidget.addItem(item3)
		self.listWidget.addItem(item4)
		self.listWidget.addItem(item5)
		self.listWidget.addItem(item6)
		self.listWidget.addItem(item7)
		self.listWidget.addItem(item8)
		self.listWidget.addItem(item9)
		self.listWidget.addItem(item10)

		self.listWidget.addKey(RC_info)

		self.listWidget._exec(None, "")

		self.selected = self.listWidget.getSelected()
		key = self.listWidget.getKey()

		# first handle keys
		if key == RC_info:
			messageBox()

		# handle selected line
		if self.selected == 0:
			messageBox()
		elif self.selected == 1:
			moviePlayer()
		elif self.selected == 2:
			headers()
		elif self.selected == 3:
			helpBox()
		elif self.selected == 4:
			hintBox()
		elif self.selected == 5:
			infoBox()
		elif self.selected == 6:
			stringInput()
		elif self.selected == 7:
			textBox()
		elif self.selected == 8:
			audioPlayer()
		elif self.selected == 9:
			pictureViewer()

		# exit pressed
		if self.listWidget.getExitPressed() is False:
			self.listWidget.clearItems()
			self.showMenu()

if __name__ == "__main__":
	testMenu()





















