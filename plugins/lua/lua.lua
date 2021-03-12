--[[
neutrinoHD2 lua sample plugin
]]

local selected = -1
local RETURN_NONE	= 0
local RETURN_REPAINT 	= 1
local RETURN_EXIT 	= 2
local RETURN_EXIT_ALL   = 4

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
	local ret = RETURN_REPAINT

	local testWidget = neutrino.CWidget()
	local listBox = neutrino.ClistBox()

	testWidget:enableCenterPos()

	listBox:setTitle("testCWidget(ClistBox)")
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
	item5 = neutrino.CMenuForwarder("CStringInput", false)
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

	if testWidget:getExitPressed() ~= true and ret == RETURN_REPAINT then
		testCWidget()
	end

	return ret
end

-- ClistBoxWidget
function testClistBoxWidget()
	local ret = RETURN_REPAINT

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
		
	if listBoxWidget:getExitPressed() ~= true and ret == RETURN_REPAINT then
		testClistBoxWidget()
	end

	return ret
end

-- ClistBox
function testClistBox()
	local ret = RETURN_REPAINT

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
	local ret = RETURN_REPAINT

	local box = neutrino.CBox()
	local fb = neutrino.CSwigHelpers()
	local button = neutrino.CButtons()

	box.iX = fb:getScreenX() + 40
	box.iY = fb:getScreenY() + 40
	box.iWidth = fb:getScreenWidth() - 80
	box.iHeight = fb:getScreenHeight() - 80

	--
	btn = neutrino.button_label_struct()

	btn.button = neutrino.NEUTRINO_ICON_AUDIO
	btn.locale = neutrino.NONEXISTANT_LOCALE
	btn.localename = "green"

	local btnRed = neutrino.NEUTRINO_ICON_BUTTON_RED
	local btnGreen = neutrino.NEUTRINO_ICON_BUTTON_GREEN
	local btnYellow = neutrino.NEUTRINO_ICON_BUTTON_YELLOW
	local btnBlue = neutrino.NEUTRINO_ICON_BUTTON_BLUE

	window = neutrino.CWindow(box)
	head = neutrino.CHeaders(box.iX, box.iY, box.iWidth, 40, "CHeaders", neutrino.NEUTRINO_ICON_MOVIE)
	foot = neutrino.CFooters(box.iX, box.iY + box.iHeight - 40, box.iWidth, 40)

	window:enableCenterPos()
	--head:enablePaintDate()
	head:setButtons(btn, 1)

	local m = neutrino.CWidget(box)
	--m:enablePaintMainFrame()
	m:enableCenterPos()

	--m:addKey(neutrino.RC_ok)
	m:addKey(neutrino.RC_down)
	m:addKey(neutrino.RC_up)
	m:addKey(neutrino.RC_info)
	m:addKey(neutrino.RC_red)
	m:addKey(neutrino.RC_green)
	m:addKey(neutrino.RC_yellow, null, "moviePlayer")
	m:addKey(neutrino.RC_blue)

	window:paint()
	head:paint()
	foot:paint()
	neutrino.CButtons():paintButton(btnRed, "AudioPlayer", box.iX + 5, box.iY + box.iHeight - 40, (box.iWidth)/4, 40)
	neutrino.CButtons():paintButton(btnGreen, "PicturePlayer", box.iX + (box.iWidth)/4 + 5, box.iY + box.iHeight - 40, (box.iWidth)/4, 40)
	neutrino.CButtons():paintButton(btnYellow, "MoviePlayer", box.iX + 2*(box.iWidth)/4 + 5, box.iY + box.iHeight - 40, (box.iWidth)/4, 40)
	neutrino.CButtons():paintButton(btnBlue, "InfoBox", box.iX + 3*(box.iWidth)/4 + 5, box.iY + box.iHeight - 40, (box.iWidth)/4, 40)

	ret = m:exec(null, "")


	local key = m:getKey()
	local actionKey = m:getActionKey()

	if actionKey == "moviePlayer" then
		print("lua sample: testCWindow(): actionKey: moviePlayer")

		window:hide()
		moviePlayer()
	end
		
	if key == neutrino.RC_red then
		window:hide()
		audioPlayer()
	elseif key == neutrino.RC_green then
		window:hide()
		pictureViewer()
	--elseif key == neutrino.RC_yellow then
	--	window:hide()
	--	moviePlayer()
	elseif key == neutrino.RC_blue then
		window:hide()
		infoBox()
	end

	if m:getExitPressed() ~= true then
		testCWindow()
	end

	window:hide()
end

-- CFrameBox
function testCFrameBox()
	local ret = neutrino.menu_return().RETURN_REPAINT

	local box = neutrino.CBox()
	local fb = neutrino.CSwigHelpers()

	box.iX = fb:getScreenX() + 40
	box.iY = fb:getScreenY() + 40
	box.iWidth = 350
	box.iHeight = fb:getScreenHeight() - 80

	local frameBox = neutrino.CFrameBox(box)

	frameBox:setMode(neutrino.FRAME_MODE_VERTICAL)
	--frameBox:setBackgroundColor(neutrino.COL_BACKGROUND0)

	frame1 = neutrino.CFrame("MP3")
	frame2 = neutrino.CFrame("PicViewer")
	frame3 = neutrino.CFrame("MoviePlayer")
	frame3:setIconName(neutrino.NEUTRINO_ICON_MOVIE)
	frame3:setOption("spielt Movie Dateien")
	frame3:setActionKey(null, "moviePlayer")

	frameBox:addFrame(frame1)
	frameBox:addFrame(frame2)
	frameBox:addFrame(frame3)
	frameBox:addFrame(neutrino.CFrameSeparator())
	frameBox:addFrame(neutrino.CFrameSeparator())
	frameBox:addFrame(neutrino.CFrameSeparator())
	frameBox:addFrame(neutrino.CFrameSeparator())
	frameBox:addFrame(neutrino.CFrameSeparator())
	frameBox:addFrame(neutrino.CFrameSeparator())

	frame4 = neutrino.CFrame("Beenden")
	frame4:setActionKey(null, "exit")
	frameBox:addFrame(frame4)

	local m = neutrino.CWidget()

	m:addKey(neutrino.RC_ok)
	m:addKey(neutrino.RC_right)
	m:addKey(neutrino.RC_left)
	m:addKey(neutrino.RC_down)
	m:addKey(neutrino.RC_up)
	m:addKey(neutrino.RC_info)

	repeat
		frameBox:paint()

		ret = m:exec(null, "")

		local selected = frameBox:getSelected()
		local actionKey = frameBox:getActionKey()

		local key = m:getKey()

		if key == neutrino.RC_left then
			frameBox:swipLeft()
		elseif key == neutrino.RC_right then
			frameBox:swipRight()
		elseif key == neutrino.RC_down then
			frameBox:scrollLineDown()
		elseif key == neutrino.RC_up then
			frameBox:scrollLineUp()
		elseif key == neutrino.RC_info then
			infoBox()
		elseif key == neutrino.RC_ok then
			frameBox:hide()

			if actionKey == "moviePlayer" then
				print("testCFrameBox: actionKey: moviePlayer")
				moviePlayer()
			elseif actionKey == "exit" then
				print("testCFrameBox: actionKey: exit")
				break;
			end

			if selected >=0 then
				if selected == 0 then
					audioPlayer()
				elseif selected == 1 then
					pictureViewer()
				--elseif selected == 2 then
				--	moviePlayer()
				--elseif selected == 9 then
					--break
				end
			end
		end
	until m:getExitPressed() == true

	frameBox:hide()

	return ret
end

-- main
function main()
	local ret = RETURN_REPAINT
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

	m:addItem(item1)
	m:addItem(item2)
	m:addItem(item3)
	m:addItem(item4)
	m:addItem(item5)
	m:addItem(item6)

	if selected < 0 then
		selected = 0
	end

	m:setSelected(selected)

	ret = m:exec(None, "")

	selected = m:getSelected() 
	actionKey = m:getActionKey()
	Key = m:getKey()

	if actionKey == "listWidget" then
		print("lua:sample: main(): actionKey: listWidget")
		testCWidget()
	elseif actionKey == "frameBox" then
		print("lua:sample: main(): actionKey: frameBox")
		ret = testCFrameBox()
	elseif actionKey == "jumpTarget" then
		print("lua:sample: main(): actionKey: jumpTarget")
	end

	if selected >= 0 then
		--[[if selected == 0 then
			testCWidget()
		else]]if selected == 1 then
			testClistBoxWidget()
		elseif selected == 2 then
			testClistBox()
		elseif selected == 3 then
			testCWindow()
		--elseif selected == 4 then
		--	testCFrameBox()
		end
	end
	
	if m:getExitPressed() ~= true and ret == RETURN_REPAINT then
		main()
	end

	return ret
end

main()





