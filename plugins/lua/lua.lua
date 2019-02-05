print("lua: Hello World")

neutrino = require 'neutrino'

-- messagebox
function messageBox()
	mBox = neutrino.CMessageBox("CMessageBox: lua","first test\ntesting CMessageBox\ndas ist alles ;-)")
	mBox:exec(-1)
end

-- helpbox
function helpBox()
	hbox = neutrino.CHelpBox()
	hbox:addLine("neutrino: lua")
	hbox:addSeparator()
	hbox:addLine("first test")
	hbox:addLine("testing CHelpBox\ndas ist alles ;-)")
	hbox:show("CHelpBox: lua")
end

-- filebrowser
function fileBrowser()
	fbox= neutrino.CFileBrowser('/')
	fbox:exec('/')
end

-- headers
function headers()
	head = neutrino.CHeaders()
	head:paintHead(150,10,550,35,neutrino.NEUTRINO_ICON_MP3,"test", true,0,null,false)
	head:paintFoot(150, 570,550,35,550)
end

-- window
function window()
	w = neutrino.CWindow(150,45,550,525)
	w:paint()
end

-- scrollbar
function scrollBar()
	sb = neutrino.CScrollBar()
	sb:paint(150,45,525,10,1)
end

-- itemsdetailsline
function itemsDetailsline()
	il = neutrino.CItems2DetailsLine()
	il:paint(150,10,550,615,80,35,30,1)
end

-- hintbox
function hintBox()
	hint = neutrino.CHintBox("neutrino: lua","first test\ntesting CHintBox\ndas ist alles ;-)")
	hint:exec()
end

-- infobox
function infoBox()
	info = neutrino.CInfoBox("first test\ntesting CHintBox\ndas ist alles ;-)")
	info:setText("first test\ntesting CHintBox\ndas ist alles ;-)")
	info:exec()
end

-- framebuffer
function fb()
	frameBuffer = neutrino.CFrameBuffer()
	frameBuffer:getInstance():paintBackground()
end

-- listbox
function listBox()
	listbox = neutrino.ClistBox("test", neutrino.NEUTRINO_ICON_MOVIE)
	listbox:enablePaintDate()
	listbox:addWidget(neutrino.WIDGET_CLASSIC)
	listbox:addWidget(neutrino.WIDGET_FRAME)
	listbox:enableWidgetChange()
	listbox:exec(null, "")
	listbox:hide()
end

-- menuwidget
function menuWidget()
	menu = neutrino.CMenuWidget("test", neutrino.NEUTRINO_ICON_MOVIE)
	menu:enableWidgetChange()

	item1 = neutrino.CMenuForwarder("item1", true, "python-test", nil, "red_action")
	item1:setIconName(neutrino.NEUTRINO_ICON_BUTTON_RED)
	item1:setItemIcon(neutrino.NEUTRINO_ICON_BUTTON_RED)

	item2 = neutrino.CMenuForwarder("item2", true, "python-test", nil, "green_action")
	item3 = neutrino.CMenuForwarder("item3", true, "python-test", nil, "yellow_action")
	item4 = neutrino.CMenuForwarder("item4", true, "python-test", nil, "blue_action")

	menu:addItem(item1)
	menu:addItem(item2)
	menu:addItem(item3)
	menu:addItem(item4)

	menu:exec(null, "")
	menu:hide()
end

-- listboxEntry
function listEntry()
	listboxEntry = neutrino.ClistBoxEntry(150,10,550,615)
	listboxEntry:setTitle("test", neutrino.NEUTRINO_ICON_MOVIE)
	listboxEntry:enablePaintHead()
	listboxEntry:enablePaintDate()
	listboxEntry:enablePaintFoot()
	listboxEntry:paint()
end

-- playback
function player()
	playback = neutrino.cPlayback()
	playback:Close()
	playback:Open()
	playback:Start("/home/mohousch/Music/AUD-20160209-WA0000.mp3")
	playback:Play()
end

-- audioplayergui
function audioPlayerGui()
	aplay = neutrino.CAudioPlayerGui()
	aplay:addToPlaylist('/home/mohousch/Music/AUD-20160209-WA0000.mp3')
	aplay:exec(null,"")
end

-- movieplayergui
function moviePlayerGui()
	mplay=neutrino.CMoviePlayerGui()
	mplay:addToPlaylist('/home/mohousch/Videos/ProSieben_20150619_201430.ts')
	mplay:exec(null,"")
end

-- pictureviewergui
function pictureViewerGui()
	pic=neutrino.CPictureViewerGui()
	pic:addToPlaylist("/home/mohousch/Pictures/funart.png")
	pic:exec(null,"")
end

-- CPlugins
function startPlugin()
	plugins = neutrino.CPlugins()
	plugins:startPlugin("youtube")
end

-- CRCInput
function rcInput()
	rc = neutrino.CRCInput()
	rc:messageLoop()
end

-- main
function main()
	messageBox()
end

main()





