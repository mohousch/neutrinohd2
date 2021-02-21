--[[
neutrinoHD2 lua sample plugin
]]

local selected = 0

-- CMessageBox
function messageBox()
	title = "CMessageBox"
	msg = "neutrino lua:\n testing lua CMessageBox\n"
	mBox = neutrino.CMessageBox(title, msg)
	mBox:exec()
end

-- CHelpBox
function helpBox()
	hbox = neutrino.CHelpBox()
	hbox:addLine("neutrino: lua")
	hbox:addSeparator()
	hbox:addLine("first test")
	hbox:addLine("testing CHelpBox ;-)\n")
	hbox:show("CHelpBox: lua")
end

-- CHintBox
function hintBox()
	hint = neutrino.CHintBox("CHintBox","neutrino lua:\n first test\ntesting CHintBox\ndas ist alles ;-)")
	hint:exec()
end

-- CInfoBox
function infoBox()
	info = neutrino.CInfoBox()
	info:setText("neutrino lua:\nfirst test\ntesting CHintBox ;-)\n")
	info:exec()
end

-- CStringInput
function stringInput()
	title = "luaTest: CStringInputSMS"
	local value = "neutrino lua:"
	local input = neutrino.CStringInputSMS(title, vale)
	input:exec(None, "")
end

-- CAudioPlayerGui
function audioPlayer()
	local fileBrowser = neutrino.CFileBrowser()
	local fileFilter = neutrino.CFileFilter()
	local PATH = "/"

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

	local player = neutrino.CAudioPlayerGui()
	local file = nil

	repeat
		fileBrowser:exec(PATH)

		PATH = fileBrowser:getCurrentDir()
		file = fileBrowser:getSelectedFile()

		if file ~= null then
			player:addToPlaylist(file)
			player:exec(None, "")
		end
	until fileBrowser:getExitPressed() == true
end

-- CPictureViewerGui
function pictureViewer()
	local fileBrowser = neutrino.CFileBrowser()
	local fileFilter = neutrino.CFileFilter()
	local PATH = "/"

	fileFilter:addFilter("jpeg")
	fileFilter:addFilter("jpg")
	fileFilter:addFilter("png")
	fileFilter:addFilter("bmp")

	fileBrowser.Multi_Select = false
	fileBrowser.Dirs_Selectable = false
	fileBrowser.Filter = fileFilter

	local player = neutrino.CPictureViewerGui()
	local file = nil

	repeat
		fileBrowser:exec(PATH)
		PATH = fileBrowser:getCurrentDir()
		file = fileBrowser:getSelectedFile()

		if file ~= null then
			player:addToPlaylist(file)
			player:exec(None, "")
		end
	until fileBrowser:getExitPressed() == true
end

-- CMoviePlayerGui
function moviePlayer()
	local fileBrowser = neutrino.CFileBrowser()
	local fileFilter = neutrino.CFileFilter()
	local PATH = "/"

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

	local player = neutrino.CMoviePlayerGui()
	local file = nil

	repeat
		fileBrowser:exec(PATH)
		PATH = fileBrowser:getCurrentDir()
		file = fileBrowser:getSelectedFile()

		if file ~= null then
			player:addToPlaylist(file)
			player:exec(None, "")
		end
	until fileBrowser:getExitPressed() == true
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
	if id == 5 then
		audioPlayer()
	end
	if id == 6 then
		pictureViewer()
	end
	if id == 7 then
		moviePlayer()
	end

	if id == 8 then
		testCWidget()
	end
end

--listBox = neutrino.ClistBox()
--testWidget = neutrino.CWidget()

function testCWidget()
	local testWidget = neutrino.CWidget()
	local listBox = neutrino.ClistBox()

	testWidget:enableCenterPos()

	listBox:setTitle("testCWidget(ClistBox)")
	listBox:enablePaintHead()
	listBox:enablePaintDate()
	listBox:enablePaintFoot()
	listBox:enableCenterPos()

--[[
	if selected < 0 then
		selected = 0
	end
]]

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
	--listBox:addItem(neutrino.CMenuSeparator(neutrino.LINE))
	listBox:addItem(item5)
	--listBox:addItem(neutrino.CMenuSeparator(neutrino.LINE))
	listBox:addItem(item6)
	listBox:addItem(item7)
	listBox:addItem(item8)

	testWidget:enablePaintMainFrame()

	testWidget:addItem(listBox)

	testWidget:addKey(neutrino.RC_info)

--[[
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
]]

	repeat
		testWidget:exec(null, "")
		selected = listBox:getSelected()
		if selected < 0 then
			selected = 0
		end
		key = testWidget:getKey()

		exec(selected, key)
	until testWidget:getExitPressed() == true
end

--listWidget = neutrino.ClistBoxWidget("ClistBoxWidget")
function testClistBoxWidget()
	local listBoxWidget = neutrino.ClistBoxWidget("ClistBoxWidget")
	listBoxWidget:setWidgetType(neutrino.WIDGET_TYPE_STANDARD)
	listBoxWidget:setMode(neutrino.MODE_LISTBOX)
	listBoxWidget:enableShrinkMenu()
	listBoxWidget:addWidget(neutrino.WIDGET_TYPE_CLASSIC)
	listBoxWidget:addWidget(neutrino.WIDGET_TYPE_EXTENDED)
	listBoxWidget:addWidget(neutrino.WIDGET_TYPE_FRAME)
	listBoxWidget:enableWidgetChange()
	listBoxWidget:enablePaintItemInfo()
--[[
	if selected < 0 then
		selected = 0
	end
]]

	listBoxWidget:setSelected(selected)

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

	listBoxWidget:addItem(item1)
	listBoxWidget:addItem(item2)
	listBoxWidget:addItem(item3)
	listBoxWidget:addItem(item4)
	--listBoxWidget:addItem(neutrino.CMenuSeparator(neutrino.LINE))
	listBoxWidget:addItem(item5)
	--listBoxWidget:addItem(neutrino.CMenuSeparator(neutrino.LINE))
	listBoxWidget:addItem(item6)
	listBoxWidget:addItem(item7)
	listBoxWidget:addItem(item8)

	listBoxWidget:addKey(neutrino.RC_info)

--[[
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
]]

	repeat
		listBoxWidget:exec(null, "")
		selected = listBoxWidget:getSelected()
		key = listBoxWidget:getKey()

		exec(selected, key)
		
	until listBoxWidget:getExitPressed() == true
end

function testClistBox()
	local listBox = neutrino.ClistBox()
	listBox:enableCenterPos()
	listBox:enablePaintHead()
	listBox:setTitle("ClistBox")
	listBox:enablePaintFoot()
	listBox:enableShrinkMenu()

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
	--listBox:addItem(neutrino.CMenuSeparator(neutrino.LINE))
	listBox:addItem(item5)
	--listBox:addItem(neutrino.CMenuSeparator(neutrino.LINE))
	listBox:addItem(item6)
	listBox:addItem(item7)
	listBox:addItem(item8)

	local m = neutrino.CWidget()
	m:addKey(neutrino.RC_ok)
	m:addKey(neutrino.RC_down)
	m:addKey(neutrino.RC_up)

	repeat
		listBox:paint()
		m:exec(null, "")
		selected = listBox:getSelected()
		local key = m:getKey()

		if key == neutrino.RC_down then
			listBox:scrollLineDown()
		end
		if key == neutrino.RC_up then
			listBox:scrollLineUp()
		end

		if key == neutrino.RC_ok then
			listBox:hide()
			exec(selected, -1)
		end
	until m:getExitPressed() == true

	listBox:hide()
end

function testCWindow()
	local box = neutrino.CBox()
	local fb = neutrino.CSwigHelpers()

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
	--testCWidget()
	--testClistBoxWidget()
	testClistBox()
	--testCWindow()
end

main()





