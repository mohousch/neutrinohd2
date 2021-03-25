--[[
	Netzkino-Plugin

	The MIT License (MIT)

	Copyright (c) 2014 Marc Szymkowiak 'Ezak91' marc.szymkowiak91@googlemail.com
	for release-version

	Copyright (c) 2014 micha_bbg, svenhoefer, bazi98 an many other db2w-user
	with hints and codesniplets for db2w-Edition

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:
	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
]]

caption = "Netzkino HD"
local json = require "json"

ret = nil -- global return value
local selected_category = 0
local selected_movie = 0

-- ####################################################################
-- function from http://lua-users.org/wiki/BaseSixtyFour

-- character table string
local b='ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/'

-- decode
function dec(data)
	data = string.gsub(data, '[^'..b..'=]', '')
	return (data:gsub('.', function(x)
	if (x == '=') then return '' end
	local r,f='',(b:find(x)-1)
	for i=6,1,-1 do r=r..(f%2^i-f%2^(i-1)>0 and '1' or '0') end
	return r;
	end):gsub('%d%d%d?%d?%d?%d?%d?%d?', function(x)
	if (#x ~= 8) then return '' end
	local c=0
	for i=1,8 do c=c+(x:sub(i,i)=='1' and 2^(8-i) or 0) end
	return string.char(c)
	end))
end
-- ####################################################################

function decodeImage(b64Image)
	local imgTyp = b64Image:match("data:image/(.-);base64,")
	local repData = "data:image/" .. imgTyp .. ";base64,"
	local b64Data = string.gsub(b64Image, repData, "");

	local tmpImg = os.tmpname()

	local retImg = tmpImg .. "." .. imgTyp

	local f = io.open(retImg, "w+")
	f:write(dec(b64Data))
	f:close()
	os.remove(tmpImg)

	return retImg
end

function init()
	-- set collectgarbage() interval from 200 (default) to 50
	collectgarbage('setpause', 50)

	categories = {}
	movies = {};
	page = 1;
	max_page = 1;

	neutrino.CFileHelpers():createDir("/tmp/netzkino")

	-- use netzkino icon placed in same dir as the plugin ...
	--netzkino_png = script_path() .. "netzkino.png"
	-- ... or use icon placed in one of neutrino's icon dirs
	--netzkino_png = "netzkino"
	-- ... or use a base64 encoded icon
	netzkino_png = decodeImage("data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABgAAAAYCAIAAABvFaqvAAAAA3NCSVQICAjb4U/gAAAAYnpUWHRSYXcgcHJvZmlsZSB0eXBlIEFQUDEAAHicVcixDYAwDADB3lN4hHccHDIOQgFFQoCyf0EBDVee7O1so696j2vrRxNVVVXPkmuuaQFmXgZuGAkoXy38TEQNDyseBiAPSLYUyXpQ8HMAAAL2SURBVDiNrZU7b1xVEMf/c869d++192FlN944rGPHdiIFCSmKeBQgREFBRUNDxQegoEWiT0FHg0QBVdpIfACqNJESKWmiBCSkOGZtL3i9jvZ57z2PGYrdldeLnUTCo1Odx+/M/GfmHPr67t84D1PnQnkzEJGA/geIFKmAvTfDbiypsPdQryAGp7oAqLyzI91dMQNL8Yc310jcwOqWKXZUVQGAvA5EypvUNh+BlLpwNazUAyZZIq8K3H9ZyffCQesg3kKYQPjs0Ii8Se2L+yitxJsfRUsrBIiwkMo98qCcV98ehlXae+iyAYjOBoFs8xGW1pNL18Q769lT8MnlET3/rRG97OcyzKwtrqbFNbf7WE4GNwMilXd2QCq5dI2dKxbUZzdKn9Y7P3331e1vvxl0WsZjZCS3NlnehC6Yw23Q8fFjjYiUdHdVdVOYveDWevmLd+Lx0vWNK14wsmK9EEHYqeoGH/xO9S3xPOcROZOKGYXlOoSJ0O7l3//4yw8/36ldrA37XevYeJnIIhJVlsVbmx4rNQUR2IwQLRJBmJXw0/1h8/Lnyx98uVK/sNfuOu8hLDwZYEZUFDM6Pf3eO2cysB/fO8zS0UCTCABh50yqxGNck6TZu1m5pyCBChPNJogWAB6XG4noKJl4rqMwLtL4DgIp7ThXYfzfrEkQL5AumN4/AAECyFyCITKeB2B6bZAKkzKmm47zJ8xUafDRNimNVxqpwHe2qdKQmeKeqSPhqLYOl2XtF6QmIROh1/5zrNGUorNOE6Yf1a7OdskJsYkQvHXL/fUgI4praxruj/3B7V93xJsOl+WQlQ6zo6YcPAtW3yOlzwRBJExKuPK+232c9lu6utFVF+/tJ84gSgI/bJujbeS9YPXdcLEifKJp57tfhIOkpDc/NofPfeuJ32dEJe+t8TkAqjSixk1Seo5yCmgMI0JheYvq123aE5sqEQqTcKFCwiI894CcDZrgWISDeBHx4mSG3fxr9kagqXev2TC1c/tF/gU5kpOQwApURgAAAABJRU5ErkJggg==");
end

--Kategorien anzeigen
function get_categories()
	print("getCategories:")

	local fname = "/tmp/netzkino/netzkino_categories.txt";

	local h = neutrino.CHintBox(caption, "Kategorien werden geladen ...", neutrino.HINTBOX_WIDTH, netzkino_png)
	h:paint();

	neutrino.downloadUrl('https://www.netzkino.de/capi/get_category_index', fname, 'Mozilla/5.0;', 90)

	local fp = io.open(fname, "r")
	if fp == nil then
		h:hide();
		error("Error opening file '" .. fname .. "'.")
	else
		print("open file '" .. fname .. "'.")

		local s = fp:read("*a")
		fp:close()

		local j_table = json.decode(s)

		if j_table == nil  then 
			return nil 
		end

		j_categories = j_table.categories
		local j = 1;
		for i = 1, #j_categories do
			local cat = j_categories[i];
			if cat ~= nil then
				-- Kategorie 9  -> keine Streams
				-- todo remove Kategorie 6481 & 6491(altes Glueckskino & Glueckskino) -> keine Streams
				if cat.id ~= 9 then
					categories[j] =
					{
						id           = j;
						category_id  = cat.id;
						title        = cat.title;
						post_count   = cat.post_count;
					};
					j = j + 1;
				end
			end
		end

		h:hide();

		page = 1;
		if j > 1 then
			get_categories_menu();
		else
			local mBox = neutrino.CMessageBox("Fehler", "Keinen Kategorien gefunden!")
			mBox:exec()
		end
	end
end

-- Erstellen des Kategorien-Menü
function get_categories_menu()
	print("getCategories_menu:")
	
	local m_categories = nil
	local item = nil

	m_categories = neutrino.ClistBoxWidget("" .. caption.." Kategorien", netzkino_png)
	m_categories:enableShrinkMenu()

	for index, category_detail in pairs(categories) do
		local count = "(" .. category_detail.post_count .. ")"
		item = neutrino.ClistBoxItem(category_detail.title)
		item:setOptionInfo(count)
		item:setIconName(netzkino_png)

		m_categories:addItem(item)
	end

	if selected_category < 0 then
		selected_category = 0
	end

	m_categories:setSelected(selected_category)

	m_categories:exec(null, "")

	selected_category = m_categories:getSelected()
		
	if selected_category >= 0 then
		get_movies(selected_category + 1);
	end
		
	if m_categories:getExitPressed() ~= true then
		get_categories_menu()
	end
end

-- Filme zur Kategorie laden (variabel Pro Seite)
function get_movies(_id)
	print("get_movies:")

	local fname = "/tmp/netzkino/netzkino_movies.txt";
	local page_nr = page
	movies = {};

	local h = neutrino.CHintBox(caption, "Movies wird geladen ...", neutrino.HINTBOX_WIDTH, netzkino_png)
	h:paint();
	
	local url = "https://www.netzkino.de/capi/get_category_posts&id=" .. categories[_id].category_id .. "&count=50d&page=" .. page_nr .."&custom_fields=Streaming"

	neutrino.downloadUrl(url, fname)

	local fp = io.open(fname, "r")
	if fp == nil then
		h:hide();
		error("Error opening file '" .. fname .. "'.")
	else
		local s = fp:read("*a")
		fp:close()

		local j_table = json.decode(s)
		max_page = tonumber(j_table.pages);
		local posts = j_table.posts

		j = 1;
		for i = 1, #posts do
			local j_streaming = nil;
			local custom_fields = posts[i].custom_fields
			if custom_fields ~= nil then
				local stream = custom_fields.Streaming
				if stream ~= nil then
					j_streaming = stream[1]
				end
			end

			if j_streaming ~= nil then
				j_title = posts[i].title
				j_content = posts[i].content

				local j_cover = "";
				local tfile = neutrino.DATADIR .. "/neutrino/icons/nopreview.jpg"
				local attachments = posts[i].attachments[1]
				if attachments ~= nil then
					local images = attachments.images;
					if images ~= nil then
						local full = images.full
						if full ~= nil then
							j_cover = full.url

							tfile = "/tmp/netzkino/" .. j_title .. ".jpg"
							if j_cover ~= nil and neutrino.file_exists(tfile) ~= true then
								neutrino.downloadUrl(j_cover, tfile)
							end
						end
					end
				end

				movies[j] =
				{
					id      = j;
					title   = j_title;
					content = j_content;
					--cover   = j_cover;
					cover     = tfile;
					stream  = j_streaming;
				};

				j = j + 1;
			end
		end
		h:hide();

		if j > 1 then
			get_movies_menu(_id);
		else
			local mBox = neutrino.CMessageBox("Fehler", "Keinen Stream gefunden!")
			mBox:exec()

			get_categories();
		end
	end
end

--Auswahlmenü der Filme anzeigen
function get_movies_menu(_id)
	local ret = neutrino.RETURN_REPAINT
	local menu_title = caption .. ": " .. categories[_id].title;

	local red = neutrino.button_label_struct()
	red.button = neutrino.NEUTRINO_ICON_BUTTON_RED
	red.locale = neutrino.NONEXISTANT_LOCALE
	red.localename = "Action"

	local green = neutrino.button_label_struct()
	green.button = neutrino.NEUTRINO_ICON_BUTTON_GREEN
	green.locale = neutrino.NONEXISTANT_LOCALE
	green.localename = ""

	local yellow = neutrino.button_label_struct()
	yellow.button = neutrino.NEUTRINO_ICON_BUTTON_YELLOW
	yellow.locale = neutrino.NONEXISTANT_LOCALE
	yellow.localename = "Neu bei NetzKino"

	local blue = neutrino.button_label_struct()
	blue.button = neutrino.NEUTRINO_ICON_BUTTON_BLUE
	blue.locale = neutrino.NONEXISTANT_LOCALE
	blue.localename = "Highlight"

	local info = neutrino.button_label_struct()
	info.button = neutrino.NEUTRINO_ICON_BUTTON_HELP
	
	local rec = neutrino.button_label_struct()
	rec.button = neutrino.NEUTRINO_ICON_REC
	
	m_movies = neutrino.ClistBoxWidget(menu_title, netzkino_png)

	m_movies:setWidgetType(neutrino.WIDGET_TYPE_FRAME)
	m_movies:setItemsPerPage(6, 2)
	m_movies:enableCenterPos()
	m_movies:enableCenterPos()
	m_movies:setFooterButtons(red)
	m_movies:setFooterButtons(green)
	m_movies:setFooterButtons(yellow)
	m_movies:setFooterButtons(blue)
	m_movies:setHeaderButtons(info)
	m_movies:setHeaderButtons(rec)
	
	local item = nil
	for _id, movie_detail in pairs(movies) do
		item = neutrino.ClistBoxItem(conv_utf8(movie_detail.title))
		item:setItemIcon(movie_detail.cover)
		--item:setHelpText(conv_utf8(movie_detail.content))
		item:setActionKey(null, "play")

		m_movies:addItem(item)
	end

	if selected_movie < 0 then
		selected_movie = 0
	end

	m_movies:setSelected(selected_movie)

	m_movies:addKey(neutrino.RC_info, null, "info")
	m_movies:addKey(neutrino.RC_record, null, "record")
	m_movies:addKey(neutrino.RC_red, null, "action")
	m_movies:addKey(neutrino.RC_green, null, "green")
	m_movies:addKey(neutrino.RC_yellow, null, "new")
	m_movies:addKey(neutrino.RC_blue, null, "highlight")

	m_movies:exec(null, "")
	
	selected_movie = m_movies:getSelected()
	local key = m_movies:getKey()
	local actionKey = m_movies:getActionKey()

	if actionKey == "play" then
		--playMovie(selected_movie + 1)
		funArt(selected_movie + 1)
	elseif actionKey == "info" then
		showMovieInfo(selected_movie + 1)
	elseif actionKey == "record" then
		download_stream(selected_movie + 1)
	elseif actionKey == "action" then
		 get_movies(7551)
	elseif actionKey == "green" then
	elseif actionKey == "new" then
		 get_movies(81)
	elseif actionKey == "highlight" then
		 get_movies(9692)
	end
	
	if m_movies:getExitPressed() ~= true then
		get_movies_menu(_id)
	end

end

function playMovie(_id)
	video = neutrino.CMoviePlayerGui()
	local file = nil

	stream_name = conv_utf8(movies[_id].stream)
	title = conv_utf8(movies[_id].title)
	info1 = conv_utf8(movies[_id].content)
	cover = movies[_id].cover
	file = "https://pmd.netzkino-seite.netzkino.de/" .. stream_name ..".mp4"
		
	if file ~= nil then
		video:addToPlaylist(file, title, info1, "", cover)
		video:exec(null, "")
	end
end

local f_selected = 0

function funArt(_id)
	stream_name = conv_utf8(movies[_id].stream)
	title = conv_utf8(movies[_id].title)
	info1 = conv_utf8(movies[_id].content)
	cover = movies[_id].cover
	file = "https://pmd.netzkino-seite.netzkino.de/" .. stream_name ..".mp4"

	local fb = neutrino.CSwigHelpers()

	local box = neutrino.CBox()
	box.iWidth = fb:getScreenWidth() - 80
	box.iHeight = fb:getScreenHeight() - 80
	box.iX = fb:getScreenX() + 40
	box.iY = fb:getScreenY() + 40

	local frame = neutrino.CBox()
	frame.iWidth = 250
	frame.iHeight = 60
	frame.iX = box.iX + 10
	frame.iY = box.iY + box.iHeight - 10 - 40 - 60

	local textbox = neutrino.CBox()
	textbox.iWidth = 350
	textbox.iHeight = 350
	textbox.iX = box.iX + 10
	textbox.iY = box.iY + 40 + 10

	local window = neutrino.CWindow(fb:getScreenX(), fb:getScreenY(), fb:getScreenWidth(), fb:getScreenHeight())
	window:enableCenterPos()

	artFrame = neutrino.CFrame(neutrino.FRAME_PICTURE_NOTSELECTABLE)
	artFrame:setPosition(fb:getScreenX() + fb:getScreenWidth()/2, fb:getScreenY(), fb:getScreenWidth()/2, fb:getScreenHeight())
	artFrame:setIconName(cover)

	textFrame = neutrino.CFrame(neutrino.FRAME_TEXT_NOTSELECTABLE)
	textFrame:setPosition(textbox)
	textFrame:setTitle(conv_utf8(movies[_id].title) .. "\n" .. conv_utf8(movies[_id].content))
	textFrame:setActionKey(null, "info")
	textFrame:disableShadow()
	--textFrame:disablePaintFrame()

	playFrame = neutrino.CFrame()
	playFrame:setPosition(frame)
	playFrame:setTitle("abspielen")
	playFrame:setIconName(neutrino.NEUTRINO_ICON_PLAY)
	playFrame:setActionKey(null, "playMovie")
	--playFrame:disableShadow()

	infoFrame = neutrino.CFrame()
	infoFrame:setPosition(frame.iX + 250 + 10, frame.iY, 250, 60)
	infoFrame:setTitle("Info:")
	infoFrame:setIconName(neutrino.NEUTRINO_ICON_INFO)
	infoFrame:setActionKey(null, "info")
	--infoFrame:disableShadow()

	testFrame = neutrino.CFrameBox(fb:getScreenX(), fb:getScreenY(), fb:getScreenWidth(), fb:getScreenHeight())
	testFrame:setMode(neutrino.FRAMEBOX_MODE_RANDOM)
	testFrame:addFrame(artFrame)
	testFrame:addFrame(textFrame)
	testFrame:addFrame(playFrame)
	testFrame:addFrame(infoFrame)

	if f_selected < 0 then
		f_selected = 0
	end

	widget = neutrino.CWidget()
	
	widget:addItem(window)
	widget:addItem(testFrame)

	widget:exec(null, "")
	local action = widget:getActionKey()
	f_selected = testFrame:getSelected()

	local player = neutrino.CMoviePlayerGui()

	if action == "playMovie" then
		player:addToPlaylist(file, title, info1, "", cover)
		player:exec(null, "")
	elseif action == "info" then
		neutrino.InfoBox(info1, title, neutrino.NEUTRINO_ICON_INFO, cover, 160, 320)
	end

	if widget:getExitPressed() ~= true then
		funArt(_id)
	end
end

-- Filminfos anzeigen
function showMovieInfo(_id)
	print("showMovieInfo:")

	neutrino.InfoBox(conv_utf8(movies[_id].content), conv_utf8(movies[_id].title), netzkino_png, movies[_id].cover, 160, 320)
end

--Stream downloaden
function download_stream(_id)
	local stream_name = conv_utf8(movies[_id].stream);

	config = neutrino.CConfigFile('\t')

	config:loadConfig(neutrino.CONFIGDIR .. "/neutrino.conf")

	d_path = config:getString("network_nfs_recordingdir")

	if (d_path == nil) then
		d_path ="/media/sda1/movies/";
	end

	local movie_file = "" .. d_path .. "/" .. conv_utf8(movies[_id].title) .. ".mp4" ;

	local h = neutrino.CHintBox(caption, "Download: "..conv_utf8(movies[_id].title).."", neutrino.HINTBOX_WIDTH, netzkino_png)
	h:paint()

	neutrino.downloadUrl("https://pmd.netzkino-seite.netzkino.de/" .. stream_name .. ".mp4", movie_file, "", 0)

	h:hide()
end

-- UTF8 in Umlaute wandeln
function conv_utf8(_string)
	if _string ~= nil then
		_string = string.gsub(_string,"\\u0026","&");
		_string = string.gsub(_string,"\\u00a0"," ");
		_string = string.gsub(_string,"\\u00b4","´");
		_string = string.gsub(_string,"\\u00c4","Ä");
		_string = string.gsub(_string,"\\u00d6","Ö");
		_string = string.gsub(_string,"\\u00dc","Ü");
		_string = string.gsub(_string,"\\u00df","ß");
		_string = string.gsub(_string,"\\u00e1","á");
		_string = string.gsub(_string,"\\u00e4","ä");
		_string = string.gsub(_string,"\\u00e8","è");
		_string = string.gsub(_string,"\\u00e9","é");
		_string = string.gsub(_string,"\\u00f4","ô");
		_string = string.gsub(_string,"\\u00f6","ö");
		_string = string.gsub(_string,"\\u00fb","û");
		_string = string.gsub(_string,"\\u00fc","ü");
		_string = string.gsub(_string,"\\u2013","–");
		_string = string.gsub(_string,"\\u201c","“");
		_string = string.gsub(_string,"\\u201e","„");
		_string = string.gsub(_string,"\\u2026","…");
		_string = string.gsub(_string,"&#038;","&");
		_string = string.gsub(_string,"&#8211;","–");
		_string = string.gsub(_string,"&#8212;","—");
		_string = string.gsub(_string,"&#8216;","‘");
		_string = string.gsub(_string,"&#8217;","’");
		_string = string.gsub(_string,"&#8230;","…");
		_string = string.gsub(_string,"&#8243;","″");
		_string = string.gsub(_string,"<[^>]*>","");
		_string = string.gsub(_string,"\\/","/");
		_string = string.gsub(_string,"\\n","");
	end

	return _string
end

function main()
	init();
	get_categories();
	neutrino.CFileHelpers():removeDir("/tmp/netzkino")
	os.remove("/tmp/lua*")
	collectgarbage();
end

main()


