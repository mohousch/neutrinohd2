print("python: Hello World")

from neutrino import CMessageBox, CHelpBox, CHeaders, CWindow, CScrollBar, CItems2DetailsLine, CPictureViewer, CFrameBuffer, CRCInput, ClistBox, CMenuWidget, CMenuForwarder, ClistBoxEntry, CInfoBox, cPlayback, CAudioPlayerGui, CMoviePlayerGui, CPictureViewerGui, CPlugins, CFile

## CMessageBox
def messageBox():
	msg = CMessageBox("neutrino: python", "first test\ntesting CMessageBox\ndas ist alles ;-)")
	msg._exec(-1)

## CHelpBox
def helpBox():
	hbox = CHelpBox()
	hbox.addLine("neutrino: python")
	hbox.addSeparator()
	hbox.addLine("first test")
	hbox.addPageBreak()
	hbox.addLine("testing CHelpBox\ndas ist alles ;-)")
	hbox.show("CHelpBox: python")

## CHeaders
def headers():
	CHeaders().paintHead(150,10,550,35,"mp3","test", 1,0,None,0)
	CHeaders().paintFoot(150, 570,550,35,550)

## CWindow
def window():
	CWindow(150,45,550,525).paint()

## CScrollBar
def scrollBar():
	CScrollBar().paint(150,45,525,10,1)

## CItemsdetailsline
def items2Detailsline():
	CItems2DetailsLine().paint(150,10,550,615,80,35,30,1)

## CInfoBox
def infoBox():
	infoBox = CInfoBox("first test\ntesting CHintBox\ndas ist alles ;-)")
	infoBox.setText("first test\ntesting CHintBox\ndas ist alles ;-)")
	infoBox._exec()

## CFrameBuffer
def fb():
	frameBuffer = CFrameBuffer.getInstance()
	frameBuffer.paintBackground()

## ClistBox
def listBox():
	listbox = ClistBox("test", "mp3")
	listbox.enablePaintDate()
	listbox.addWidget(WIDGET_CLASSIC)
	listbox.addWidget(WIDGET_FRAME)
	listbox.enableWidgetChange()
	listbox._exec(None, "")
	listbox.hide()

## CMenuWidget
def menuWidget():
	menu = CMenuWidget("test", "mp3")
	menu.enableWidgetChange()
	menu.enableFootInfo()

	item1 = CMenuForwarder("item1", True, "python-test", None, "red_action")
	item2 = CMenuForwarder("item2", True, "python-test", None, "green_action")
	item3 = CMenuForwarder("item3", True, "python-test", None, "yellow_action")
	item4 = CMenuForwarder("item4", True, "python-test", None, "blue_action")

	menu.addItem(item1)
	menu.addItem(item2)
	menu.addItem(item3)
	menu.addItem(item4)

	menu._exec(None, "")
	menu.hide()

## ClistBoxEntry
def listEntry():
	listboxEntry = ClistBoxEntry(150,10,550,615)
	listboxEntry.setTitle("test", "mp3")
	listboxEntry.enablePaintHead()
	listboxEntry.enablePaintDate()
	listboxEntry.enablePaintFoot()
	listboxEntry.paint()

## cPlayback
def playBack():
	playback = cPlayback(1)
	playback.Close()
	playback.Open()
	playback.Start("/home/mohousch/Music/AUD-20160209-WA0000.mp3")
	playback.Play()

## CAudioPlayerGui
def audioPlayerGui():
	aplay = CAudioPlayerGui()
	aplay.addToPlaylist('/home/mohousch/Music/AUD-20160209-WA0000.mp3')
	aplay._exec(None,"")

## CMoviePlayerGui
def moviePlayerGui():
	mplay=CMoviePlayerGui()
	mplay.addToPlaylist('/home/mohousch/Videos/ProSieben_20150619_201430.ts')
	mplay._exec(None,"")

## CPictureViewerGui
def pictureViewerGui():
	pic=CPictureViewerGui()
	pic.addToPlaylist("/home/mohousch/Pictures/funart.png")
	pic._exec(None,"")

## CPlugins
def plugins():
	plugins = CPlugins()
	plugins.startPlugin("nfilm")

def rcInput():
	rc = CRCInput()
	rc.messageLoop()


#class main():
#	def __init__(self, session):
#		self.session = session
#
#	messageBox()
#
#def start(session, **kwargs):
#	session.open(main)

class main():
	def __init__(self):
		pass

	messageBox()

if __name__ == "__main__":
	main()









