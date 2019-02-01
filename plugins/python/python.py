print("python: Hello World")

from neutrino import CMessageBox, CHelpBox, CHeaders, CWindow, CScrollBar, CItems2DetailsLine, CPictureViewer, CFrameBuffer, ClistBox, CMenuWidget, cPlayback

msg = CMessageBox("neutrino: python", "first test\ntesting CMessageBox\ndas ist alles ;-)")
msg._exec(-1)

## helpbox
#hbox = CHelpBox()
#hbox.addLine("neutrino: python")
#hbox.addSeparator()
#hbox.addLine("first test")
#hbox.addPageBreak()
#hbox.addLine("testing CHelpBox\ndas ist alles ;-)")
#hbox.show("CHelpBox: python")

## headers
#CHeaders().paintHead(150,10,550,35,"mp3","test", 1,0,None,0)
#CHeaders().paintFoot(150, 570,550,35,550)

## window
#CWindow(150,45,550,525).paint()

## scrollbar
#CScrollBar().paint(150,45,525,10,1)

## itemsdetailsline
#CItems2DetailsLine().paint(150,10,550,615,80,35,30,1)

## pictureviewer
#CPictureViewer().showImage("/home/mohousch/funart.png")

## framebuffer
#CFrameBuffer().paintBackground()

## listbox
#listbox = ClistBox("test", "mp3")
#listbox.enablePaintDate()
#listbox._exec(None, "")

## menuwidget
#listbox = CMenuWidget("test", "mp3")
#listbox.enableWidgetChange()
#listbox.enableFootInfo()
#listbox._exec(None, "")

## playback
#playback = cPlayback(1)
#playback.Close()
#playback.Open()
#playback.Start("/home/Music/AUD-20160209-WA0000.mp3")
#playback.Play()










