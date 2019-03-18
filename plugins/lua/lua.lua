-- 
--

function messageBox()
	title = "luaTest"
	msg = "TEST"
	mBox = neutrino.CMessageBox(title, msg)
	mBox:exec(-1)
end

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

function showMenu(parent)
	print("showMenu")

	selected = 0

	parent:setSelected(selected)
	parent:setMode(neutrino.MODE_MENU)
	parent:enableShrinkMenu()
	parent:enableWidgetChange()
	parent:enablePaintFootInfo()

	-- CMessageBox
	item1 = neutrino.CMenuForwarder("CMessageBox", true, "", null, "red action")
	item1:setItemIcon(neutrino.DATADIR .. "/neutrino/icons/plugin.png")
	item1:setHelpText("testing CMessageBox")

	-- CFileBrowser|CMoviePlayerGui
	item2 = neutrino.ClistBoxItem("CFileBrowser|CMoviePlayerGui")
	item2:setItemIcon(neutrino.DATADIR .. "/neutrino/icons/plugin.png")
	item2:setHelpText("testing CFileBrowser|CMoviePlayerGui")

	-- CHelpBox
	item3 = neutrino.ClistBoxItem("CHelpBox")
	item3:setItemIcon(neutrino.DATADIR .. "/neutrino/icons/plugin.png")
	item3:setHelpText("testing CHelpBox")

	-- CHeaders
	item4 = neutrino.ClistBoxItem("CHeaders")
	item4:setItemIcon(neutrino.DATADIR .. "/neutrino/icons/plugin.png")
	item4:setHelpText("testing CHeaders")

	-- CHintBox
	item5 = neutrino.ClistBoxItem("CHintBox")
	item5:setItemIcon(neutrino.DATADIR .. "/neutrino/icons/plugin.png")
	item5:setHelpText("testing CHintBox")

	-- CInfoBox
	item6 = neutrino.ClistBoxItem("CInfoBox")
	item6:setItemIcon(neutrino.DATADIR .. "/neutrino/icons/plugin.png")
	item6:setHelpText("testing CInfoBox")

	parent:addItem(item1)
	parent:addItem(item2)
	parent:addItem(item3)
	parent:addItem(item4)
	parent:addItem(item5)
	parent:addItem(item6)

	parent:exec(null, "")
end

function exec(parent, actionKey)
	print("_exec")

	selected = parent:getSelected()

	print(selected)

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
end

function hide(parent)
	parent:hide()
end

-- main
function main()
	-- variables
	listWidget = neutrino.ClistBoxWidget("pythonTest:ClistBoxWidget", neutrino.NEUTRINO_ICON_MOVIE)

	-- paint
	showMenu(listWidget)

	-- exec
	exec(listWidget, actionKey)

	-- hide
	hide(listWidget)
end

main()





