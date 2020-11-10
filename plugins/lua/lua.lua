-- 
--

local selected = 0
local PATH = "/"

-- CMessageBox
function messageBox()
	title = "luaTest"
	msg = "TEST"
	mBox = neutrino.CMessageBox(title, msg)
	mBox:exec(-1)
end

-- CHelpBox
function helpBox()
	hbox = neutrino.CHelpBox()
	hbox:addLine("neutrino: lua")
	hbox:addSeparator()
	hbox:addLine("first test")
	hbox:addLine("testing CHelpBox\ndas ist alles ;-)")
	hbox:show("CHelpBox: lua")
end

-- CHintBox
function hintBox()
	hint = neutrino.CHintBox("neutrino: lua","first test\ntesting CHintBox\ndas ist alles ;-)")
	hint:exec()
end

-- CInfoBox
function infoBox()
	info = neutrino.CInfoBox()
	info:setText("first test\ntesting CHintBox\ndas ist alles ;-)")
	info:exec()
end

-- CStringInput
function stringInput()
	title = "luaTest: CStringInputSMS"
	local value
	input = neutrino.CStringInputSMS(title, vale)
	input:exec(null, "")
end

fileBrowser = neutrino.CFileBrowser()
-- CAudioPlayerGui
function audioPlayer()
	fileFilter = neutrino.CFileFilter()

	fileFilter:addFilter("cdr")
	fileFilter:addFilter("mp3")
	fileFilter:addFilter("m2a")
	fileFilter:addFilter("mpa")
	fileFilter:addFilter("mp2")
	fileFilter:addFilter("ogg")
	fileFilter:addFilter("wav")
	fileFilter:addFilter("flac")
	fileFilter:addFilter("aac")
	fileFilter:addFilter("dts")
	fileFilter:addFilter("m4a")

	fileBrowser.Multi_Select = false
	fileBrowser.Dirs_Selectable = false
	fileBrowser.Filter = fileFilter

	fileBrowser:exec(PATH)

	PATH = fileBrowser:getCurrentDir()

	player = neutrino.CAudioPlayerGui()
	
	if fileBrowser:getSelectedFile() ~= null then
		player:addToPlaylist(fileBrowser:getSelectedFile())
		player:exec(None, "")
	end

	if fileBrowser:getExitPressed() ~= true then
		audioPlayer()
	end
end

-- CPictureViewerGui
function pictureViewer()
	fileFilter = neutrino.CFileFilter()

	fileFilter:addFilter("jpeg")
	fileFilter:addFilter("jpg")
	fileFilter:addFilter("png")
	fileFilter:addFilter("bmp")

	fileBrowser.Multi_Select = false
	fileBrowser.Dirs_Selectable = false
	fileBrowser.Filter = fileFilter

	fileBrowser:exec(PATH)

	PATH = fileBrowser:getCurrentDir()

	player = neutrino.CPictureViewerGui()
	
	if fileBrowser:getSelectedFile() ~= null then
		player:addToPlaylist(fileBrowser:getSelectedFile())
		player:exec(None, "")
	end

	if fileBrowser:getExitPressed() ~= true then
		pictureViewer()
	end
end

-- CMoviePlayerGui
function moviePlayer()
	fileFilter = neutrino.CFileFilter()

	fileFilter:addFilter("ts")
	fileFilter:addFilter("mpg")
	fileFilter:addFilter("mpeg")
	fileFilter:addFilter("divx")
	fileFilter:addFilter("avi")
	fileFilter:addFilter("mkv")
	fileFilter:addFilter("asf")
	fileFilter:addFilter("aiff")
	fileFilter:addFilter("m2p")
	fileFilter:addFilter("mpv")
	fileFilter:addFilter("m2ts")
	fileFilter:addFilter("vob")
	fileFilter:addFilter("mp4")
	fileFilter:addFilter("mov")	
	fileFilter:addFilter("flv")	
	fileFilter:addFilter("dat")
	fileFilter:addFilter("trp")
	fileFilter:addFilter("vdr")
	fileFilter:addFilter("mts")
	fileFilter:addFilter("wmv")
	fileFilter:addFilter("wav")
	fileFilter:addFilter("flac")
	fileFilter:addFilter("mp3")
	fileFilter:addFilter("wma")
	fileFilter:addFilter("ogg")

	fileBrowser.Multi_Select = false
	fileBrowser.Dirs_Selectable = false
	fileBrowser.Filter = fileFilter

	fileBrowser:exec(PATH)

	PATH = fileBrowser:getCurrentDir()

	player = neutrino.CMoviePlayerGui()
	
	if fileBrowser:getSelectedFile() ~= null then
		player:addToPlaylist(fileBrowser:getSelectedFile())
		player:exec(None, "")
	end

	if fileBrowser:getExitPressed() ~= true then
		moviePlayer()
	end
end

function exec(id, msg)
	print("exec:id:", id)

	-- handle keys
	if msg == neutrino.RC_info then
		infoBox()
	end

	-- handle others
	if id == 0 then
		messageBox()
	end
	if id == 1 then
		helpBox()
	end
	if id == 2 then
		hintBox()
	end
	if id == 3 then
		infoBox()
	end
	if id == 4 then
		stringInput()
	end
	if id == 7 then
		audioPlayer()
	end
	if id == 8 then
		pictureViewer()
	end
	if id == 9 then
		moviePlayer()
	end
end

listBox = neutrino.ClistBox()
testWidget = neutrino.CWidget()

function testCWidget()
	testWidget:enableCenterPos()

	listBox:setTitle("testCWidget(ClistBox)")
	listBox:enablePaintHead()
	listBox:enablePaintDate()
	listBox:enablePaintFoot()
	listBox:enableCenterPos()

	if selected < 0 then
		selected = 0
	end

	listBox:setSelected(selected)

	listBox:setWidgetMode(neutrino.MODE_LISTBOX)
	listBox:enableShrinkMenu()
	listWidget:setWidgetType(neutrino.WIDGET_TYPE_CLASSIC)

	-- CMessageBox
	item1 = neutrino.CMenuForwarder("CMessageBox", true, "", self, "red action")
	item1:setItemIcon(neutrino.DATADIR .. "/neutrino/icons/plugin.png")
	item1:setHelpText("testing CMessageBox")
	item1:setInfo1("testing CMessageBox")

	-- CHelpBox
	item2 = neutrino.CMenuForwarder("CHelpBox")
	item2:setItemIcon(neutrino.DATADIR .. "/neutrino/icons/plugin.png")
	item2:setHelpText("testing CHelpBox")
	item2:setInfo1("testing CHelpBox")

	-- CHintBox
	item3 = neutrino.CMenuForwarder("CHintBox")
	item3:setItemIcon(neutrino.DATADIR .. "/neutrino/icons/plugin.png")
	item3:setHelpText("testing CHintBox")
	item3:setInfo1("testing CHintBox")

	-- CInfoBox
	item4 = neutrino.CMenuForwarder("CInfoBox")
	item4:setItemIcon(neutrino.DATADIR .. "/neutrino/icons/plugin.png")
	item4:setHelpText("testing CInfoBox")
	item4:setInfo1("testing CInfoBox")

	-- CStringInput
	item5 = neutrino.CMenuForwarder("CStringInput", false)
	item5:setItemIcon(neutrino.DATADIR .. "/neutrino/icons/plugin.png")
	item5:setHelpText("testing CStringInput")
	item5:setInfo1("testing CStringInput")

	-- CAudioPlayerGui
	item6 = neutrino.CMenuForwarder("CAudioPlayerGui")
	item6:setItemIcon(neutrino.DATADIR .. "/neutrino/icons/plugin.png")
	item6:setHelpText("testing CAudioPlayerGui")
	item6:setInfo1("testing CAudioPlayerGui")

	-- CPictureViewerGui
	item7 = neutrino.CMenuForwarder("CPictureViewerGui")
	item7:setItemIcon(neutrino.DATADIR .. "/neutrino/icons/plugin.png")
	item7:setHelpText("testing CPictureViewerGui")
	item7:setInfo1("testing CPictureViewerGui")

	-- CMoviePlayerGui
	item8 = neutrino.CMenuForwarder("CMoviePlayerGui")
	item8:setItemIcon(neutrino.DATADIR .. "/neutrino/icons/plugin.png")
	item8:setHelpText("testing CMoviePlayerGui")
	item8:setInfo1("testing CMoviePlayerGui")

	listBox:addItem(item1)
	listBox:addItem(item2)
	listBox:addItem(item3)
	listBox:addItem(item4)
	listBox:addItem(neutrino.CMenuSeparator(neutrino.LINE))
	listBox:addItem(item5)
	listBox:addItem(neutrino.CMenuSeparator(neutrino.LINE))
	listBox:addItem(item6)
	listBox:addItem(item7)
	listBox:addItem(item8)

	testWidget:enablePaintMainFrame()

	testWidget:addItem(listBox)

	testWidget:addKey(neutrino.RC_info)

	testWidget:exec(null, "")

	-- handle
	selected = listBox:getSelected()

	key = testWidget:getKey()

	if testWidget:getExitPressed() == true or key == neutrino.RC_info then
		selected = -1
	end

	exec(selected, key)

	-- repaint or exit
	if testWidget:getExitPressed() == false then
		listBox:clearItems()
		testWidget:clearItems()
		testCWidget()
	end
end

listWidget = neutrino.ClistBoxWidget("ClistBoxWidget")
function testClistBoxWidget()
	--listWidget = neutrino.ClistBoxWidget("ClistBoxWidget")

	listWidget:setWidgetType(neutrino.WIDGET_TYPE_STANDARD)
	listWidget:setMode(neutrino.MODE_LISTBOX)
	listWidget:enableShrinkMenu()
	listWidget:addWidget(neutrino.WIDGET_TYPE_CLASSIC)
	listWidget:addWidget(neutrino.WIDGET_TYPE_EXTENDED)
	listWidget:addWidget(neutrino.WIDGET_TYPE_FRAME)
	listWidget:enableWidgetChange()
	listWidget:enablePaintFootInfo()

	if selected < 0 then
		selected = 0
	end

	listWidget:setSelected(selected)

	-- CMessageBox
	item1 = neutrino.CMenuForwarder("CMessageBox", true, "", self, "red action")
	item1:setItemIcon(neutrino.DATADIR .. "/neutrino/icons/plugin.png")
	item1:setHelpText("testing CMessageBox")
	item1:setInfo1("testing CMessageBox")

	-- CHelpBox
	item2 = neutrino.CMenuForwarder("CHelpBox")
	item2:setItemIcon(neutrino.DATADIR .. "/neutrino/icons/plugin.png")
	item2:setHelpText("testing CHelpBox")
	item2:setInfo1("testing CHelpBox")

	-- CHintBox
	item3 = neutrino.CMenuForwarder("CHintBox")
	item3:setItemIcon(neutrino.DATADIR .. "/neutrino/icons/plugin.png")
	item3:setHelpText("testing CHintBox")
	item3:setInfo1("testing CHintBox")

	-- CInfoBox
	item4 = neutrino.CMenuForwarder("CInfoBox")
	item4:setItemIcon(neutrino.DATADIR .. "/neutrino/icons/plugin.png")
	item4:setHelpText("testing CInfoBox")
	item4:setInfo1("testing CInfoBox")

	-- CStringInput
	item5 = neutrino.CMenuForwarder("CStringInput", false)
	item5:setItemIcon(neutrino.DATADIR .. "/neutrino/icons/plugin.png")
	item5:setHelpText("testing CStringInput")
	item5:setInfo1("testing CStringInput")

	-- CAudioPlayerGui
	item6 = neutrino.CMenuForwarder("CAudioPlayerGui")
	item6:setItemIcon(neutrino.DATADIR .. "/neutrino/icons/plugin.png")
	item6:setHelpText("testing CAudioPlayerGui")
	item6:setInfo1("testing CAudioPlayerGui")

	-- CPictureViewerGui
	item7 = neutrino.CMenuForwarder("CPictureViewerGui")
	item7:setItemIcon(neutrino.DATADIR .. "/neutrino/icons/plugin.png")
	item7:setHelpText("testing CPictureViewerGui")
	item7:setInfo1("testing CPictureViewerGui")

	-- CMoviePlayerGui
	item8 = neutrino.CMenuForwarder("CMoviePlayerGui")
	item8:setItemIcon(neutrino.DATADIR .. "/neutrino/icons/plugin.png")
	item8:setHelpText("testing CMoviePlayerGui")
	item8:setInfo1("testing CMoviePlayerGui")

	listWidget:addItem(item1)
	listWidget:addItem(item2)
	listWidget:addItem(item3)
	listWidget:addItem(item4)
	listWidget:addItem(neutrino.CMenuSeparator(neutrino.LINE))
	listWidget:addItem(item5)
	listWidget:addItem(neutrino.CMenuSeparator(neutrino.LINE))
	listWidget:addItem(item6)
	listWidget:addItem(item7)
	listWidget:addItem(item8)

	listWidget:addKey(neutrino.RC_info)

	listWidget:exec(null, "")

	-- handle
	selected = listWidget:getSelected()
	key = listWidget:getKey()

	exec(selected, key)

	-- repaint or exit
	if listWidget:getExitPressed() == false then
		listWidget:clearItems()
		testClistBoxWidget()
	end
end

function testClistBox()
	listBox:enableCenterPos()
	listBox:enablePaintHead()
	listBox:setTitle("ClistBox")
	listBox:enablePaintFoot()

	listBox:setSelected(selected)

	-- CMessageBox
	item1 = neutrino.CMenuForwarder("CMessageBox", true, "", self, "red action")
	item1:setItemIcon(neutrino.DATADIR .. "/neutrino/icons/plugin.png")
	item1:setHelpText("testing CMessageBox")
	item1:setInfo1("testing CMessageBox")

	-- CHelpBox
	item2 = neutrino.CMenuForwarder("CHelpBox")
	item2:setItemIcon(neutrino.DATADIR .. "/neutrino/icons/plugin.png")
	item2:setHelpText("testing CHelpBox")
	item2:setInfo1("testing CHelpBox")

	-- CHintBox
	item3 = neutrino.CMenuForwarder("CHintBox")
	item3:setItemIcon(neutrino.DATADIR .. "/neutrino/icons/plugin.png")
	item3:setHelpText("testing CHintBox")
	item3:setInfo1("testing CHintBox")

	-- CInfoBox
	item4 = neutrino.CMenuForwarder("CInfoBox")
	item4:setItemIcon(neutrino.DATADIR .. "/neutrino/icons/plugin.png")
	item4:setHelpText("testing CInfoBox")
	item4:setInfo1("testing CInfoBox")

	-- CStringInput
	item5 = neutrino.CMenuForwarder("CStringInput", false)
	item5:setItemIcon(neutrino.DATADIR .. "/neutrino/icons/plugin.png")
	item5:setHelpText("testing CStringInput")
	item5:setInfo1("testing CStringInput")

	-- CAudioPlayerGui
	item6 = neutrino.CMenuForwarder("CAudioPlayerGui")
	item6:setItemIcon(neutrino.DATADIR .. "/neutrino/icons/plugin.png")
	item6:setHelpText("testing CAudioPlayerGui")
	item6:setInfo1("testing CAudioPlayerGui")

	-- CPictureViewerGui
	item7 = neutrino.CMenuForwarder("CPictureViewerGui")
	item7:setItemIcon(neutrino.DATADIR .. "/neutrino/icons/plugin.png")
	item7:setHelpText("testing CPictureViewerGui")
	item7:setInfo1("testing CPictureViewerGui")

	-- CMoviePlayerGui
	item8 = neutrino.CMenuForwarder("CMoviePlayerGui")
	item8:setItemIcon(neutrino.DATADIR .. "/neutrino/icons/plugin.png")
	item8:setHelpText("testing CMoviePlayerGui")
	item8:setInfo1("testing CMoviePlayerGui")

	listBox:addItem(item1)
	listBox:addItem(item2)
	listBox:addItem(item3)
	listBox:addItem(item4)
	listBox:addItem(neutrino.CMenuSeparator(neutrino.LINE))
	listBox:addItem(item5)
	listBox:addItem(neutrino.CMenuSeparator(neutrino.LINE))
	listBox:addItem(item6)
	listBox:addItem(item7)
	listBox:addItem(item8)

	listBox:paint()
	
	neutrino.CWidget():exec(null, "")

	listBox:hide()
end

function testCWindow()
	box = neutrino.CBox()
	fb = neutrino.CSwigHelpers()

	box.iX = fb:getScreenX()
	box.iY = fb:getScreenY()
	box.iWidth = fb:getScreenWidth()
	box.iHeight = fb:getScreenHeight()

	window = neutrino.CWindow(box)

	window:enableCenterPos()
	window:paint()
	
	neutrino.CWidget():exec(null, "")

	window:hide()
end

-- main
function main()
	testCWidget()
	--testClistBoxWidget()
	--testClistBox()
	--testCWindow()
end

main()





