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
	hint:exec(10)
end

-- CInfoBox
function infoBox()
	info = neutrino.CInfoBox()
	info:setText("neutrino lua:\nfirst test\ntesting CHintBox ;-)\n")
	info:exec()
end

-- CStringInput
function stringInput()
	local title = "luaTest: CStringInputSMS"
	local value = "neutrino lua:"
	local input = neutrino.CStringInputSMS(title, vale)
	input:exec(None, "")
end

-- CAudioPlayerGui
function audioPlayer()
	local fileBrowser = neutrino.CFileBrowser()
	local fileFilter = neutrino.CFileFilter()
	
	config = neutrino.CConfigFile('\t')

	config:loadConfig(neutrino.CONFIGDIR .. "/neutrino.conf")

	local PATH = config:getString("network_nfs_audioplayerdir")

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
	
	config = neutrino.CConfigFile('\t')

	config:loadConfig(neutrino.CONFIGDIR .. "/neutrino.conf")

	local PATH = config:getString("network_nfs_picturedir")

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

	config = neutrino.CConfigFile('\t')

	config:loadConfig(neutrino.CONFIGDIR .. "/neutrino.conf")

	local PATH = config:getString("network_nfs_recordingdir")

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

-- exec_actionKey
function exec(id, msg, actionKey)
	print("lua sample: exec: actionKey: (" .. actionKey ..")")
	if actionKey == "msgBox" then
		messageBox()
	elseif actionKey == "helpBox" then
		helpBox()
	elseif actionKey == "hintBox" then
		hintBox()
	elseif actionKey == "cStringInput" then
		stringInput()
	elseif actionKey == "audioPlayer" then
		audioPlayer()
	elseif actionKey == "pictureViewer" then
		pictureViewer()
	elseif actionKey == "moviePlayer" then
		moviePlayer()
	elseif actionKey == "infoBox" then
		infoBox()
	elseif msg == neutrino.RC_info then
		infoBox()
	elseif id == 0 then
		messageBox()
	elseif id == 1 then
		helpBox()
	elseif id == 2 then
		hintBox()
	elseif id == 3 then
		infoBox()
	elseif id == 4 then
		stringInput()
	elseif id == 5 then
		audioPlayer()
	elseif id == 6 then
		pictureViewer()
	elseif id == 7 then
		moviePlayer()
	end
end

-- CWidget
function testCWidget()
	local ret = neutrino.RETURN_REPAINT

	local testWidget = neutrino.CWidget()
	local listBox = neutrino.ClistBox()

	testWidget:enableCenterPos()

	listBox:setTitle("CWidget|ClistBox")
	listBox:enablePaintHead()
	listBox:enablePaintDate()
	listBox:enablePaintFoot()
	listBox:enableCenterPos()
	listBox:enableShrinkMenu()

	listBox:setWidgetMode(neutrino.MODE_LISTBOX)
	listBox:setWidgetType(neutrino.WIDGET_TYPE_CLASSIC)

	-- CMessageBox
	item1 = neutrino.CMenuForwarder("CMessageBox", true, "", null, "msgBox")
	item1:setItemIcon(neutrino.DATADIR .. "/neutrino/icons/plugin.png")
	item1:setHelpText("testing CMessageBox")
	item1:setInfo1("testing CMessageBox")

	-- CHelpBox
	item2 = neutrino.CMenuForwarder("CHelpBox")
	item2:setItemIcon(neutrino.DATADIR .. "/neutrino/icons/plugin.png")
	item2:setHelpText("testing CHelpBox")
	item2:setInfo1("testing CHelpBox")
	item2:setActionKey(null, "helpBox")

	-- CHintBox
	item3 = neutrino.CMenuForwarder("CHintBox")
	item3:setItemIcon(neutrino.DATADIR .. "/neutrino/icons/plugin.png")
	item3:setHelpText("testing CHintBox")
	item3:setInfo1("testing CHintBox")
	item3:setActionKey(null, "hintBox")

	-- CInfoBox
	item4 = neutrino.CMenuForwarder("CInfoBox")
	item4:setItemIcon(neutrino.DATADIR .. "/neutrino/icons/plugin.png")
	item4:setHelpText("testing CInfoBox")
	item4:setInfo1("testing CInfoBox")
	item4:setActionKey(null, "infoBox")

	-- CStringInput
	local data = ""
	item5 = neutrino.CMenuForwarder("CStringInput", true)
	item5:setItemIcon(neutrino.DATADIR .. "/neutrino/icons/plugin.png")
	item5:setHelpText("testing CStringInput")
	item5:setInfo1("testing CStringInput")
	item5:setActionKey(null, "cStringInput")

	-- CAudioPlayerGui
	item6 = neutrino.CMenuForwarder("CAudioPlayerGui")
	item6:setItemIcon(neutrino.DATADIR .. "/neutrino/icons/plugin.png")
	item6:setHelpText("testing CAudioPlayerGui")
	item6:setInfo1("testing CAudioPlayerGui")
	item6:setActionKey(null, "audioPlayer")

	-- CPictureViewerGui
	item7 = neutrino.CMenuForwarder("CPictureViewerGui")
	item7:setItemIcon(neutrino.DATADIR .. "/neutrino/icons/plugin.png")
	item7:setHelpText("testing CPictureViewerGui")
	item7:setInfo1("testing CPictureViewerGui")
	item7:setActionKey(null, "pictureViewer")

	-- CMoviePlayerGui
	item8 = neutrino.CMenuForwarder("CMoviePlayerGui")
	item8:setItemIcon(neutrino.DATADIR .. "/neutrino/icons/plugin.png")
	item8:setHelpText("testing CMoviePlayerGui")
	item8:setInfo1("testing CMoviePlayerGui")
	item8:setActionKey(null, "moviePlayer")

	listBox:addItem(item1)
	listBox:addItem(item2)
	listBox:addItem(item3)
	listBox:addItem(item4)
	listBox:addItem(item5)
	listBox:addItem(item6)
	listBox:addItem(item7)
	listBox:addItem(item8)

	if selected < 0 then
		selected = 0
	end

	listBox:setSelected(selected)

	--testWidget:enablePaintMainFrame()
	testWidget:addItem(listBox)
	testWidget:addKey(neutrino.RC_info, self, "info")

	ret = testWidget:exec(null, "")

	selected = listBox:getSelected()
	local key = testWidget:getKey()
	local actionKey = testWidget:getActionKey()

	exec(selected, key, actionKey)

	if testWidget:getExitPressed() ~= true and ret == neutrino.RETURN_REPAINT then
		testCWidget()
	end

	return ret
end

-- ClistBoxWidget
function testClistBoxWidget()
	local ret = neutrino.RETURN_REPAINT

	local listBoxWidget = neutrino.ClistBoxWidget("ClistBoxWidget")
	listBoxWidget:setWidgetType(neutrino.WIDGET_TYPE_STANDARD)
	listBoxWidget:setMode(neutrino.MODE_MENU)
	listBoxWidget:enablePaintFootInfo()
	listBoxWidget:enableShrinkMenu()

	-- CMessageBox
	item1 = neutrino.CMenuForwarder("CMessageBox", true, "", self, "msgBox")
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
	item5 = neutrino.CMenuForwarder("CStringInput", true)
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
	listBoxWidget:addItem(item5)
	listBoxWidget:addItem(item6)
	listBoxWidget:addItem(item7)
	listBoxWidget:addItem(item8)

	if selected < 0 then
		selected = 0
	end

	listBoxWidget:setSelected(selected)

	listBoxWidget:addKey(neutrino.RC_info)

	ret = listBoxWidget:exec(null, "")

	selected = listBoxWidget:getSelected()
	local key = listBoxWidget:getKey()
	local actionKey = listBoxWidget:getActionKey()
	

	exec(selected, key, actionKey)
		
	if listBoxWidget:getExitPressed() ~= true and ret == neutrino.RETURN_REPAINT then
		testClistBoxWidget()
	end

	return ret
end

-- ClistBox
function testClistBox()
	local ret = neutrino.RETURN_REPAINT

	local listBox = neutrino.ClistBox()
	listBox:enableCenterPos()
	listBox:enablePaintHead()
	listBox:setTitle("ClistBox")
	listBox:enablePaintFoot()
	listBox:enableShrinkMenu()

	-- CMessageBox
	item1 = neutrino.CMenuForwarder("CMessageBox", true, "", self, "msgBox")
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
	listBox:addItem(item5)
	listBox:addItem(item6)
	listBox:addItem(item7)
	listBox:addItem(item8)

	local m = neutrino.CWidget()

	m:addKey(neutrino.RC_ok)
	m:addKey(neutrino.RC_down)
	m:addKey(neutrino.RC_up)
	m:addKey(neutrino.RC_info)

	repeat
		listBox:paint()
		ret = m:exec(null, "")
		selected = listBox:getSelected()
		local key = m:getKey()
		local actionKey = m:getActionKey()

		if key == neutrino.RC_down then
			listBox:scrollLineDown()
		end
		if key == neutrino.RC_up then
				listBox:scrollLineUp()
		end
		
		if key == neutrino.RC_ok or key == neutrino.RC_info then
			listBox:hide()
			if selected >=0 then
				exec(selected, key, actionKey)
			end
		end
	until m:getExitPressed() == true

	listBox:setSelected(selected)

	listBox:hide()

	return ret
end

-- CWindow
function testCWindow()
	local ret = neutrino.RETURN_REPAINT

	local fb = neutrino.CSwigHelpers()

	box = neutrino.CBox()
	box.iWidth = fb:getScreenWidth()
	box.iHeight = fb:getScreenHeight()
	box.iX = fb:getScreenX()
	box.iY = fb:getScreenY()

	local headBox = neutrino.CBox()
	headBox.iWidth = box.iWidth
	headBox.iHeight = 40
	headBox.iX = box.iX
	headBox.iY = box.iY

	local footBox = neutrino.CBox()
	footBox.iWidth = box.iWidth
	footBox.iHeight = 40
	footBox.iX = box.iX
	footBox.iY = box.iY + box.iHeight - footBox.iHeight

	local picBox = neutrino.CBox()
	picBox.iWidth = 200
	picBox.iHeight = 320
	picBox.iX = box.iX + box.iWidth - 10 - picBox.iWidth
	picBox.iY = box.iY + headBox.iHeight + 10

	local listbox = neutrino.CBox()
	listbox.iWidth = 200
	listbox.iHeight = box.iHeight - headBox.iHeight - footBox.iHeight
	listbox.iX = box.iX
	listbox.iY = box.iY + headBox.iHeight

	local textbox = neutrino.CBox()
	textbox.iWidth = 350
	textbox.iHeight = 350
	textbox.iX = box.iX + box.iWidth - 10 - picBox.iWidth - 10 - 100 - 10 - textbox.iWidth
	textbox.iY = box.iY + headBox.iHeight + 10 + 60 + 25

	local pluginBox = neutrino.CBox()
	pluginBox.iWidth = 100
	pluginBox.iHeight = 60
	pluginBox.iX = box.iX + box.iWidth - 10 - picBox.iWidth - 10 - pluginBox.iWidth
	pluginBox.iY = box.iY + headBox.iHeight + 10

	local iconBox = neutrino.CBox()
	iconBox.iWidth = 100
	iconBox.iHeight = 40
	iconBox.iX = box.iX + box.iWidth - 10 - 200 - 10 - 100 - 10 - 350
	iconBox.iY = box.iY + box.iHeight - 10 - 40 - 40

	local frame1Box = neutrino.CBox()
	frame1Box.iWidth = 250
	frame1Box.iHeight = 60
	frame1Box.iX = box.iX + box.iWidth - 10 - 200 - 10 - 100 - 10 - 350 + 10 + 100
	frame1Box.iY = box.iY + box.iHeight - 10 - 40 - 60

	-- window
	window = neutrino.CWindow(box)
	window:enableCenterPos()

	-- head
	head = neutrino.CHeaders(headBox, "lua sample Window|Widget", neutrino.NEUTRINO_ICON_MOVIE)
	head:enablePaintDate()

	btn = neutrino.button_label_struct()

	btn.button = neutrino.NEUTRINO_ICON_AUDIO
	btn.locale = neutrino.NONEXISTANT_LOCALE
	btn.localename = ""
	head:setButtons(btn)

	info = neutrino.button_label_struct()

	info.button = neutrino.NEUTRINO_ICON_BUTTON_HELP
	info.locale = neutrino.NONEXISTANT_LOCALE
	info.localename = ""
	head:setButtons(info)

	-- foot
	foot = neutrino.CFooters(footBox)

	red = neutrino.button_label_struct()
	red.button = neutrino.NEUTRINO_ICON_BUTTON_RED
	red.locale = neutrino.NONEXISTANT_LOCALE
	red.localename = "audioPlayer"
	foot:setButtons(red, 1)

	green = neutrino.button_label_struct()
	green.button = neutrino.NEUTRINO_ICON_BUTTON_GREEN
	green.locale = neutrino.NONEXISTANT_LOCALE
	green.localename = "pictureViewer"
	foot:setButtons(green)

	yellow = neutrino.button_label_struct()
	yellow.button = neutrino.NEUTRINO_ICON_BUTTON_YELLOW
	yellow.locale = neutrino.NONEXISTANT_LOCALE
	yellow.localename = "Focus"
	foot:setButtons(yellow)

	blue = neutrino.button_label_struct()
	blue.button = neutrino.NEUTRINO_ICON_BUTTON_BLUE
	blue.locale = neutrino.NONEXISTANT_LOCALE
	blue.localename = "InfoBox"
	foot:setButtons(blue)

	-- frame
	frame1 = neutrino.CFrame(neutrino.FRAME_BOX)
	frame1:setPosition(frame1Box)
	frame1:setTitle("Mediaplayer")
	frame1:setIconName(neutrino.NEUTRINO_ICON_MOVIE)
	frame1:setOption("spielt Media Dateien")
	frame1:setActionKey(null, "frame1")

	-- Icon
	frame2 = neutrino.CFrame(neutrino.FRAME_ICON)
	frame2:setPosition(iconBox)
	frame2:setTitle("Exit")
	frame2:setIconName(neutrino.NEUTRINO_ICON_BUTTON_RED)
	frame2:setActionKey(null, "exit")

	-- picture
	config = neutrino.CConfigFile('\t')
	config:loadConfig(neutrino.CONFIGDIR .. "/neutrino.conf")
	local PATH = config:getString("network_nfs_recordingdir")

	local mFile = PATH .. "/ProSieben_20121225_201400.ts"

	local m_movieInfo = neutrino.CMovieInfo()

	local movieInfo = m_movieInfo:loadMovieInfo(mFile)

	frame3 = neutrino.CFrame(neutrino.FRAME_PICTURE)
	frame3:setPosition(picBox)
	frame3:setTitle(movieInfo.epgTitle)
	frame3:setIconName(movieInfo.tfile)
	frame3:setActionKey(null, "frame3")

	--title
	titleFrame = neutrino.CFrame(neutrino.FRAME_TEXT_LINE_NOTSELECTABLE)
	titleFrame:setPosition(textbox.iX, box.iY + headBox.iHeight + 10, 350, 40)
	titleFrame:setTitle(movieInfo.epgTitle)

	--icon
	iconFrame1= neutrino.CFrame(neutrino.FRAME_ICON_NOTSELECTABLE)
	iconFrame1:setPosition(textbox.iX, box.iY + headBox.iHeight + 50, 25, 25)
	iconFrame1:setIconName(neutrino.NEUTRINO_ICON_STAR_ON)
	iconFrame1:disablePaintFrame()

	iconFrame2= neutrino.CFrame(neutrino.FRAME_ICON_NOTSELECTABLE)
	iconFrame2:setPosition(textbox.iX + 25, box.iY + headBox.iHeight + 50, 25, 25)
	iconFrame2:setIconName(neutrino.NEUTRINO_ICON_STAR_ON)
	iconFrame2:disablePaintFrame()

	iconFrame3= neutrino.CFrame(neutrino.FRAME_ICON_NOTSELECTABLE)
	iconFrame3:setPosition(textbox.iX + 25 + 25, box.iY + headBox.iHeight + 50, 25, 25)
	iconFrame3:setIconName(neutrino.NEUTRINO_ICON_STAR_ON)
	iconFrame3:disablePaintFrame()

	iconFrame4= neutrino.CFrame(neutrino.FRAME_ICON_NOTSELECTABLE)
	iconFrame4:setPosition(textbox.iX + 25 +25 + 25, box.iY + headBox.iHeight + 50, 25, 25)
	iconFrame4:setIconName(neutrino.NEUTRINO_ICON_STAR_OFF)
	iconFrame4:disablePaintFrame()

	-- Text
	frame4 = neutrino.CFrame(neutrino.FRAME_TEXT_NOTSELECTABLE)
	frame4:setPosition(textbox)
	--frame4:setBackgroundColor(0xFFAAAA)
	frame4:setTitle(movieInfo.epgInfo1 .. "\n" .. movieInfo.epgInfo2)
	frame4:setActionKey(null, "frame4")
	frame4:disablePaintFrame()

	-- plugin
	frame5 = neutrino.CFrame(neutrino.FRAME_PLUGIN)
	frame5:setPosition(pluginBox)
	frame5:setTitle("nfilm")
	frame5:setPlugin("nfilm")
	--frame5:showPluginName()
	--frame5:disableShadow()

	-- vframe
	vframe = neutrino.CFrame(neutrino.FRAME_LINE_VERTICAL)
	vframe:setPosition(box.iX + listbox.iWidth + 10, box.iY + 50, 5, box.iHeight- 100)

	testFrame = neutrino.CFrameBox()
	testFrame:setMode(neutrino.FRAMEBOX_MODE_RANDOM)
	testFrame:addFrame(titleFrame)
	testFrame:addFrame(iconFrame1)
	testFrame:addFrame(iconFrame2)
	testFrame:addFrame(iconFrame3)
	testFrame:addFrame(iconFrame4)
	testFrame:addFrame(frame4)
	testFrame:addFrame(frame5)
	testFrame:addFrame(frame3)
	testFrame:addFrame(frame2)
	testFrame:addFrame(frame1)
	testFrame:addFrame(vframe)

	--listbox
	listBox = neutrino.ClistBox(listbox)
	listBox:enablePaintHead()
	listBox:setTitle("listBox", neutrino.NEUTRINO_ICON_MOVIE)
	listBox:setHeadGradient(neutrino.nogradient)
	listBox:setOutFocus()

	listBox:addItem(neutrino.CMenuForwarder(neutrino.LOCALE_MENU_BACK))
	listBox:addItem(neutrino.CMenuSeparator(neutrino.LINE))
	listBox:addItem(neutrino.ClistBoxItem("item1", true, "", neutrino.CAudioPlayerSettings(), "jumpTarget"))
	listBox:addItem(neutrino.ClistBoxItem("item2"))
	listBox:addItem(neutrino.ClistBoxItem("item3"))
	listBox:addItem(neutrino.CMenuSeparator(neutrino.LINE))
	listBox:addItem(neutrino.ClistBoxItem("item4"))
	listBox:addItem(neutrino.ClistBoxItem("item5"))
	listBox:addItem(neutrino.CMenuSeparator(neutrino.LINE))
	listBox:addItem(neutrino.CMenuSeparator())
	listBox:addItem(neutrino.CMenuSeparator())
	listBox:addItem(neutrino.CMenuSeparator())
	listBox:addItem(neutrino.CMenuSeparator())
	listBox:addItem(neutrino.CMenuSeparator())
	listBox:addItem(neutrino.CMenuSeparator(neutrino.LINE))
	listBox:addItem(neutrino.ClistBoxItem("Exit", true, "", null, "exit"))

	local m = neutrino.CWidget(box)
	m:enableCenterPos()
	--m:enablePaintMainFrame()

	if selected < 0 then
		selected = 0
	end
	
	m:setSelected(selected)

	m:addItem(window)
	m:addItem(head)
	m:addItem(listBox)
	m:addItem(testFrame)
	m:addItem(foot)

	m:addKey(neutrino.RC_red, null, "audioPlayer")
	m:addKey(neutrino.RC_green, null, "pictureViewer")
	--m:addKey(neutrino.RC_yellow, null, "moviePlayer")
	m:addKey(neutrino.RC_blue, null, "infoBox")
	m:addKey(neutrino.RC_info, null, "infoBox")
	m:addKey(neutrino.RC_audio, null, "infoBox")

	ret = m:exec(null, "")

	local actionKey = m:getActionKey()

	if actionKey == "moviePlayer" then
		print("lua sample: testCWindow(): actionKey: moviePlayer")

		audioPlayer()
	elseif actionKey == "pictureViewer" then
		print("lua sample: testCWindow(): actionKey: pictureViewer")

		pictureViewer()
	elseif actionKey == "audioPlayer" then
		print("lua sample: testCWindow(): actionKey: audioPlayer")

		window:hide()
		audioPlayer()
	elseif actionKey == "frame1" then
		print("lua sample: testCWindow(): actionKey: frame1")

		moviePlayer()
	elseif actionKey == "exit" then
		print("lua sample: testCWindow(): actionKey: exit")

		return neutrino.RETURN_REPAINT
	elseif actionKey == "frame3" then
		print("lua sample: testCWindow(): actionKey: frame3")

		--funArt()
		movieWidget = neutrino.CMovieInfoWidget()
		movieWidget:setMovie(movieInfo)

		movieWidget:exec(null, "")

	elseif actionKey == "nfilm" then
		print("lua sample: testCWindow(): actionKey: nfilm")
		neutrino.g_PluginList:startPlugin("nfilm")
	elseif actionKey == "infoBox" then
		print("lua sample: testCWindow(): actionKey: infoBox")
		
		infoBox()
	elseif actionKey == "frame4" then
		print("lua sample: testCWindow(): actionKey: frame4")

		neutrino.InfoBox(title, "lua window|widget")
	end

	if m:getExitPressed() ~= true then
		testCWindow()
	end

	return ret
end

-- CFrameBox
function testCFrameBox()
	local ret = neutrino.RETURN_REPAINT

	local box = neutrino.CBox()
	local fb = neutrino.CSwigHelpers()

	box.iX = fb:getScreenX() + 40
	box.iY = fb:getScreenY() + 40
	box.iWidth = 350
	box.iHeight = fb:getScreenHeight() - 80

	local window = neutrino.CWindow(box.iX + 40, box.iY + 40, fb:getScreenWidth() - 80, fb:getScreenHeight() - 80)
	window:enableCenterPos()

	local frameBox = neutrino.CFrameBox(box)
	frameBox:setMode(neutrino.FRAMEBOX_MODE_VERTICAL)

	frame1 = neutrino.CFrame()
	frame1:setTitle("MP3")
	frameBox:addFrame(frame1)

	frame2 = neutrino.CFrame()
	frame2:setTitle("PicViewer")
	frameBox:addFrame(frame2)

	frame3 = neutrino.CFrame()
	frame3:setTitle("MoviePlayer")
	frame3:setIconName(neutrino.NEUTRINO_ICON_MOVIE)
	frame3:setOption("spielt Movie Dateien")
	frame3:setActionKey(null, "moviePlayer")
	frameBox:addFrame(frame3)

	frame10 = neutrino.CFrame()
	frame10:setTitle("Beenden")
	frame10:setActionKey(null, "exit")
	frameBox:addFrame(frame10)

	local m = neutrino.CWidget()

	--m:enablePaintMainFrame()

	--m:addItem(window)
	m:addItem(frameBox)

	ret = m:exec(null, "")

	local actionKey = m:getActionKey()

	if actionKey == "moviePlayer" then
		print("testCFrameBox: actionKey: moviePlayer")
		moviePlayer()
	elseif actionKey == "exit" then
		print("testCFrameBox: actionKey: exit")
		return ret
	end

	if m:getExitPressed() ~= true then
		testCFrameBox()
	end

	return ret
end

-- CFrameBox
function testCFrameBoxRandom()
	local ret = neutrino.RETURN_REPAINT

	local box = neutrino.CBox()
	local fb = neutrino.CSwigHelpers()

	box.iX = fb:getScreenX() + 40
	box.iY = fb:getScreenY() + 40
	box.iWidth = fb:getScreenWidth() - 80
	box.iHeight = fb:getScreenHeight() - 80

	local window = neutrino.CWindow(box.iX + 40, box.iY + 40, fb:getScreenWidth() - 80, fb:getScreenHeight() - 80)
	window:enableCenterPos()

	local frameBox = neutrino.CFrameBox(box)
	frameBox:setMode(neutrino.FRAMEBOX_MODE_RANDOM)

	frame1 = neutrino.CFrame()
	frame1:setPosition(box.iX, box.iY + 2, 350, 60)
	frame1:setTitle("MP3")
	frame1:setActionKey(null, "audioPlayer")
	frameBox:addFrame(frame1)

	frame2 = neutrino.CFrame()
	frame2:setPosition(box.iX, box.iY + 2 + 60, 350, 60)
	frame2:setTitle("PicViewer")
	frame2:setActionKey(null, "pictureViewer")
	frameBox:addFrame(frame2)

	frame3 = neutrino.CFrame()
	frame3:setPosition(box.iX, box.iY + 2 + 60 + 2 + 60, 350, 60)
	frame3:setTitle("MoviePlayer")
	frame3:setIconName(neutrino.NEUTRINO_ICON_MOVIE)
	frame3:setOption("spielt Movie Dateien")
	frame3:setActionKey(null, "moviePlayer")
	frameBox:addFrame(frame3)

	frame10 = neutrino.CFrame()
	frame10:setPosition(box.iX, fb:getScreenHeight() - 80 - 60, 350, 60)
	frame10:setTitle("Beenden")
	frame10:setActionKey(null, "exit")
	frameBox:addFrame(frame10)

	local m = neutrino.CWidget(box)

	m:enablePaintMainFrame()

	m:addItem(window)
	m:addItem(frameBox)

	ret = m:exec(null, "")

	local actionKey = m:getActionKey()

	if actionKey == "moviePlayer" then
		print("testCFrameBox: actionKey: moviePlayer")
		moviePlayer()
	elseif actionKey == "audioPlayer" then
		audioPlayer()
	elseif actionKey == "pictureViewer" then
		pictureViewer()
	elseif actionKey == "exit" then
		print("testCFrameBox: actionKey: exit")
		return ret
	end

	if m:getExitPressed() ~= true then
		testCFrameBoxRandom()
	end

	return ret
end

function movieBrowser()
	local ret = neutrino.RETURN_REPAINT

	local menu = neutrino.ClistBoxWidget("Movie Browser", neutrino.NEUTRINO_ICON_MOVIE)
	menu:setWidgetType(neutrino.WIDGET_TYPE_FRAME)
	menu:setItemsPerPage(6, 2)
	menu:enablePaintDate()

	-- head
	info = neutrino.button_label_struct()

	info.button = neutrino.NEUTRINO_ICON_BUTTON_HELP
	info.locale = neutrino.NONEXISTANT_LOCALE
	info.localename = ""
	menu:setHeaderButtons(info)

	btn = neutrino.button_label_struct()

	btn.button = neutrino.NEUTRINO_ICON_BUTTON_MUTE_SMALL
	btn.locale = neutrino.NONEXISTANT_LOCALE
	btn.localename = ""
	menu:setHeaderButtons(btn)

	-- foot
	btnRed = neutrino.button_label_struct()

	btnRed.button = neutrino.NEUTRINO_ICON_BUTTON_RED
	btnRed.locale = neutrino.NONEXISTANT_LOCALE
	btnRed.localename = "delete all"
	menu:setFooterButtons(btnRed)

	btnGreen = neutrino.button_label_struct()

	btnGreen.button = neutrino.NEUTRINO_ICON_BUTTON_GREEN
	btnGreen.locale = neutrino.NONEXISTANT_LOCALE
	btnGreen.localename = "Add"
	menu:setFooterButtons(btnGreen)

	if selected < 0 then
		selected = 0
	end

	menu:setSelected(selected)
	
	local item = nil

	local ret = neutrino.RETURN_REPAINT
	local selected = 0

	-- load movies
	local fileBrowser = neutrino.CFileBrowser()
	local fh = neutrino.CFileHelpers()
	local fileFilter = neutrino.CFileFilter()

	config = neutrino.CConfigFile('\t')

	config:loadConfig(neutrino.CONFIGDIR .. "/neutrino.conf")

	local PATH = config:getString("network_nfs_recordingdir") .. "/"

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

	-- fill items
	fileBrowser.Filter = fileFilter

	local filelist = {}
	filelist = fh:readDir(PATH, fileFilter)

	--print(filelist.Name)

	menu:exec(null, "")

	if menu:getExitPressed() ~= true then
		movieBrowser()
	end

	return ret
end

-- main
function main()
	local ret = neutrino.RETURN_REPAINT
	local m = neutrino.ClistBoxWidget("lua sample")

	m:setMode(neutrino.MODE_MENU)
	m:enableShrinkMenu()
	m:enablePaintFootInfo()
	m:setFootInfoMode(neutrino.FOOT_HINT_MODE)

	item1 = neutrino.CMenuForwarder("testCWidget", true, "", null, "listWidget")
	item1:setInfo1("lua: testing CWidget")

	item2 = neutrino.CMenuForwarder("testClistBoxWidget")
	item2:setActionKey(null, "listBoxWidget")
	item2:setInfo1("lua: testing ClistBoxWidget")

	item3 = neutrino.CMenuForwarder("testClistBox")
	item3:setInfo1("lua: testing ClistBox")

	item4 = neutrino.CMenuForwarder("testCWindow")
	item4:setInfo1("lua: testing CWindow")

	item5 = neutrino.CMenuForwarder("testCFrameBox")
	item5:setInfo1("lua: testing CFrameBox")
	item5:setActionKey(null, "frameBox")

	item6 = neutrino.CMenuForwarder("testActionKey/jumpTarget")
	item6:setActionKey(neutrino.CAudioPlayerSettings(), "jumpTarget")
	item6:setInfo1("lua: testing testActionKey/jumpTarget")
	
	item7 = neutrino.CMenuForwarder("movieBrowser", true, "", self, "movieBrowser")

	item8 = neutrino.CMenuForwarder("testCFrameBoxRandom")
	item8:setInfo1("lua: testing CFrameBoxRandom")
	item8:setActionKey(null, "frameBoxRandom")

	m:addItem(item1)
	m:addItem(item2)
	m:addItem(item3)
	m:addItem(item4)
	m:addItem(item5)
	m:addItem(item6)
	m:addItem(item7)
	m:addItem(item8)

	if selected < 0 then
		selected = 0
	end

	m:setSelected(selected)

	ret = m:exec(None, "")

	selected = m:getSelected() 
	actionKey = m:getActionKey()

	if actionKey == "listWidget" then
		ret = testCWidget()
	elseif actionKey == "frameBox" then
		ret = testCFrameBox()
	elseif actionKey == "jumpTarget" then

	elseif actionKey == "movieBrowser" then
		ret = movieBrowser()
	elseif actionKey == "frameBoxRandom" then
		ret = testCFrameBoxRandom()
	end

	if selected >= 0 then
		if selected == 1 then
			ret = testClistBoxWidget()
		elseif selected == 2 then
			ret = testClistBox()
		elseif selected == 3 then
			ret = testCWindow()
		end
	end
	
	if m:getExitPressed() ~= true and ret == neutrino.RETURN_REPAINT then
		main()
	end

	return ret
end

main()





