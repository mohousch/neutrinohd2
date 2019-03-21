-- 
--

local selected = -1

-- CMessageBox
function messageBox()
	title = "luaTest"
	msg = "TEST"
	mBox = neutrino.CMessageBox(title, msg)
	mBox:exec(-1)
end

-- CMoviePlayerGui
function moviePlayer()
	settings = neutrino.SNeutrinoSettings()
	PATH = settings.network_nfs_moviedir

	fileBrowser = neutrino.CFileBrowser()
	fileFilter = neutrino.CFileFilter()
	fileFilter:addFilter("ts")

	fileBrowser.Multi_Select = false
	fileBrowser.Dirs_Selectable = false
	fileBrowser.Filter = fileFilter

	fileBrowser:exec(PATH)
		
	cFile = fileBrowser:getSelectedFile()

	player = neutrino.CMoviePlayerGui()
	
	player:addToPlaylist(cFile)
	player:exec(None, "")
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

-- CHeaders
function headers()
	head = neutrino.CHeaders()
	head:paintHead(150,10,550,35,neutrino.NEUTRINO_ICON_MP3,"test")
	head:paintFoot(150, 570,550,35)

	msg = -1
	loop = true
--[[
	while loop == true do

		msg = neutrino.CSwigHelpers():getRCCode()

		if msg == RC_home then
			loop = false
			break
		end
	end
]]--

	neutrino.CRCInput():messageLoop()

	neutrino.CSwigHelpers():paintBackground()
	neutrino.CSwigHelpers():blit()
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

-- CAudioPlayerGui
function audioPlayer()
	settings = neutrino.SNeutrinoSettings()
	PATH = settings.network_nfs_audioplayerdir

	fileBrowser = neutrino.CFileBrowser()
	fileFilter = neutrino.CFileFilter()
	fileFilter:addFilter("mp3")

	fileBrowser.Multi_Select = false
	fileBrowser.Dirs_Selectable = false
	fileBrowser.Filter = fileFilter

	fileBrowser:exec(PATH)
	
	--cFile = neutrino.CFile()	
	cFile = fileBrowser:getSelectedFile()

	player = neutrino.CAudioPlayerGui()
	
	player:addToPlaylist(cFile)
	player:exec(None, "")
end

-- CPictureViewerGui
function picPlayer()
	settings = neutrino.SNeutrinoSettings()
	PATH = settings.network_nfs_picturedir

	fileBrowser = neutrino.CFileBrowser()
	fileFilter = neutrino.CFileFilter()
	fileFilter:addFilter("jpeg")
	fileFilter:addFilter("jpg")

	fileBrowser.Multi_Select = false
	fileBrowser.Dirs_Selectable = false
	fileBrowser.Filter = fileFilter

	fileBrowser:exec(PATH)
	
	--cFile = neutrino.CFile()	
	cFile = fileBrowser:getSelectedFile()

	player = neutrino.CPictureViewerGui()
	
	player:addToPlaylist(cFile)
	player:exec(None, "")
end

listWidget = neutrino.ClistBoxWidget("pythonTest:ClistBoxWidget", neutrino.NEUTRINO_ICON_MOVIE)

function showMenu()
	print("showMenu")

	--listWidget:setSelected(selected)
	listWidget:setWidgetType(neutrino.WIDGET_TYPE_CLASSIC)
	listWidget:setMode(neutrino.MODE_MENU)
	listWidget:enableShrinkMenu()
	--listWidget:enableWidgetChange()
	listWidget:enablePaintFootInfo()

	-- CMessageBox
	item1 = neutrino.CMenuForwarder("CMessageBox", true, "", null, "red action")
	item1:setItemIcon(neutrino.DATADIR .. "/neutrino/icons/plugin.png")
	item1:setHelpText("testing CMessageBox")

	-- CFileBrowser|CMoviePlayerGui
	item2 = neutrino.CMenuForwarder("CFileBrowser|CMoviePlayerGui")
	item2:setItemIcon(neutrino.DATADIR .. "/neutrino/icons/plugin.png")
	item2:setHelpText("testing CFileBrowser|CMoviePlayerGui")

	-- CHelpBox
	item3 = neutrino.CMenuForwarder("CHelpBox")
	item3:setItemIcon(neutrino.DATADIR .. "/neutrino/icons/plugin.png")
	item3:setHelpText("testing CHelpBox")

	-- CHeaders
	item4 = neutrino.CMenuForwarder("CHeaders")
	item4:setItemIcon(neutrino.DATADIR .. "/neutrino/icons/plugin.png")
	item4:setHelpText("testing CHeaders")

	-- CHintBox
	item5 = neutrino.CMenuForwarder("CHintBox")
	item5:setItemIcon(neutrino.DATADIR .. "/neutrino/icons/plugin.png")
	item5:setHelpText("testing CHintBox")

	-- CInfoBox
	item6 = neutrino.CMenuForwarder("CInfoBox")
	item6:setItemIcon(neutrino.DATADIR .. "/neutrino/icons/plugin.png")
	item6:setHelpText("testing CInfoBox")

	-- CAudioPlayerGui
	item7 = neutrino.CMenuForwarder("CAudioPlayerGui")
	item7:setItemIcon(neutrino.DATADIR .. "/neutrino/icons/plugin.png")
	item7:setHelpText("testing CAudioPlayerGui")

	-- CPictureViewerGui
	item8 = neutrino.CMenuForwarder("CPictureViewerGui")
	item8:setItemIcon(neutrino.DATADIR .. "/neutrino/icons/plugin.png")
	item8:setHelpText("testing CPictureViewerGui")

	listWidget:addItem(item1)
	listWidget:addItem(item2)
	listWidget:addItem(item3)
	listWidget:addItem(item4)
	listWidget:addItem(item5)
	listWidget:addItem(item6)
	listWidget:addItem(item7)
	listWidget:addItem(item8)

	listWidget:addKey(neutrino.RC_info)

	listWidget:exec(null, "")

	selected = listWidget:getSelected()
	key = listWidget:getKey()

	if key == neutrino.RC_info then
		messageBox()
	end

	if selected == 0 then
		messageBox()
	end
	if selected == 1 then
		moviePlayer()
	end
	if selected == 2 then
		helpBox()
	end
	if selected == 3 then
		headers()
	end
	if selected == 4 then
		hintBox()
	end
	if selected == 5 then
		infoBox()
	end
	if selected == 6 then
		audioPlayer()
	end
	if selected == 7 then
		picPlayer()
	end

	if listWidget:getExitPressed() == false then
		listWidget:clearItems()
		showMenu()
	end
end

-- main
function main()
	showMenu()
end

main()





