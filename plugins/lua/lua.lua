-- 
--

function messageBox()
	mBox = neutrino.CMessageBox("CMessageBox: lua","first test\ntesting CMessageBox\ndas ist alles ;-)")
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

function showMenu(parent)
	print("showMenu")

	selected = 0

	parent:setSelected(selected)
	parent:setMode(neutrino.MODE_MENU)
	parent:enableShrinkMenu()
	parent:enableWidgetChange()
	parent:enablePaintFootInfo()

	item1 = neutrino.CMenuForwarder("messageBox", true, "", null, "red action")
	item1:setItemIcon(neutrino.DATADIR .. "/neutrino/icons/plugin.png")
	item1:setHelpText("testing CMessageBox")

	item2 = neutrino.ClistBoxItem("item2", true, "", null, "green action")
	item2:setItemIcon(neutrino.DATADIR .. "/neutrino/icons/plugin.png")

	item3 = neutrino.ClistBoxItem("moviePlayer")
	item3:setItemIcon(neutrino.DATADIR .. "/neutrino/icons/plugin.png")
	item3:setHelpText("testing CMoviePlayerGui")

	parent:addItem(item1)
	parent:addItem(item2)
	parent:addItem(item3)

	parent:exec(null, "")
end

function exec(parent, actionKey)
	print("_exec")

	selected = parent:getSelected()

	print(selected)

	if selected == 0 then
		messageBox()
		--return neutrino.menu_return():RETURN_REPAINT
	end

	if selected == 1 then
		--return neutrino.menu_return():RETURN_REPAINT
	end

	if selected == 2 then
		moviePlayer()
		--return neutrino.menu_return():RETURN_REPAINT
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





