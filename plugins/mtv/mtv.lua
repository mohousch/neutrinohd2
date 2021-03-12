--[[
	mtv.ch
	Copyright (C) 2015,2019,2020  Jacek Jendrzej 'satbaby'
	With Help from: Thomas(2015,2016,2017,2018,2019,2020),Dosik7(2017),BPanther(2018)

	License: GPL

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public
	License as published by the Free Software Foundation; either
	version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	General Public License for more details.

	You should have received a copy of the GNU General Public
	License along with this program; if not, write to the
	Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
	Boston, MA  02110-1301, USA.
]]

local glob = {}
local mtv_version = "mtv.de Version 0.37" -- Lua API Version: " .. APIVERSION.MAJOR .. "." .. APIVERSION.MINOR
--local n = neutrino()
local conf = {}
--local on="ein"
--local off="aus"

function get_confFile()
	return neutrino.PLUGINDIR .. "/mtv/mtv.conf"
end
function get_conf_mtvfavFile()
	return neutrino.PLUGINDIR .. "/mtv/mtvfav.conf"
end

--[[
function hideMenu(menu)
	if menu ~= nil then 
		menu:hide() 
	end
end
]]

--[[
function setvar(k, v)
	if v and #v > 0 then
		conf[k]=v
		conf.changed = true
	end
end
]]

--[[
function file_exists(file)
	--local fh = filehelpers.new()
	--if fh then return fh:exist(file, "f") else return false end
	--local fh = neutrino.CFileHelpers()
	if neutrino.file_exists(file) then
		return true
	else
		return false
	end
end
]]

function saveConfig()
--[[
	if conf.changed then
		local config	= configfile.new()
		config:setString("path", conf.path)
		config:setString("path_m3u", conf.path_m3u)
		config:setBool  ("dlflag",conf.dlflag)
		config:setBool  ("flvflag",conf.flvflag)
		config:setBool  ("hlsflag",conf.hlsflag)
		config:setBool  ("playflvflag",conf.playflvflag)
		config:setBool  ("shuffleflag",conf.shuffleflag)
		config:setString("maxRes", conf.maxRes)
		config:setString("search", conf.search)
		config:saveConfig(get_confFile())
		conf.changed = false
	end
	if glob.fav_changed == true then
		local file_mtvconf=io.open(get_conf_mtvfavFile(),"w")
		for k, v in ipairs(glob.mtv) do
			if v.fav == true then
				file_mtvconf:write('name="'.. v.name ..'",url="'.. v.url ..'"\n')
			end
		end
		file_mtvconf:close()
	end
]]
	--if conf.changed then
		local config = neutrino.CConfigFile('\t')

		config:setString("path", conf.path)
		config:setString("path_m3u", conf.path_m3u)
		config:setBool  ("dlflag",conf.dlflag)
		config:setBool  ("flvflag",conf.flvflag)
		config:setBool  ("hlsflag",conf.hlsflag)
		config:setBool  ("playflvflag",conf.playflvflag)
		config:setBool  ("shuffleflag",conf.shuffleflag)
		config:setString("maxRes", conf.maxRes)
		config:setString("search", conf.search)
		config:saveConfig(get_confFile())
		conf.changed = false
	--end

	--if glob.fav_changed == true then
		local file_mtvconf = io.open(get_conf_mtvfavFile(),"w")
		for k, v in ipairs(glob.mtv) do
			if v.fav == true then
				file_mtvconf:write('name="'.. v.name ..'",url="'.. v.url ..'"\n')
			end
		end
		file_mtvconf:close()
	--end
end

function loadConfig()
--[[
	local config	= configfile.new()
	config:loadConfig(get_confFile())
	conf.path = config:getString("path", "/media/sda1/movies/")
	conf.path_m3u = config:getString("path_m3u", "/media/sda1/movies/")
	conf.dlflag = config:getBool("dlflag", false)
	conf.flvflag = config:getBool("flvflag", false)
	conf.hlsflag = config:getBool("hlsflag", true) --hls as default, rtmp server is broken???
	conf.playflvflag = config:getBool("playflvflag", false)
	conf.shuffleflag = config:getBool("shuffleflag", false)
	conf.maxRes = config:getString("maxRes", "1280x720")
	conf.search = config:getString("search", "Justin Bieber")
	conf.changed = false
]]
	local config = neutrino.CConfigFile('\t')

	config:loadConfig(get_confFile())
	conf.path = config:getString("path", "/media/sda1/movies/")
	conf.path_m3u = config:getString("path_m3u", "/media/sda1/movies/")
	conf.dlflag = config:getBool("dlflag", false)
	conf.flvflag = config:getBool("flvflag", false)
	conf.hlsflag = config:getBool("hlsflag", true) --hls as default, rtmp server is broken???
	conf.playflvflag = config:getBool("playflvflag", false)
	conf.shuffleflag = config:getBool("shuffleflag", false)
	conf.maxRes = config:getString("maxRes", "1280x720")
	conf.search = config:getString("search", "Justin Bieber")
	conf.changed = false
end

function which(bin_name)
	local path = os.getenv("PATH") or "/bin"
	for v in path:gmatch("([^:]+):?") do
		local file = v .. "/" .. bin_name
		if neutrino.file_exists(file) then
			return true
		end
	end
	return false
end

function read_file(filename)
	if filename == nil then
		print("Error: FileName  is empty")
		return nil
	end

	local fp = io.open(filename, "r")

	if fp == nil then 
		print("Error opening file '" .. filename .. "'.") 
		return nil 
	end

	local data = fp:read("*a")
	fp:close()

	return data
end

function get_json_data(url)
	print("get_json_data: ")

	local data = getdata(url)

	if data == nil then 
		return nil 
	end

	local videosection = string.match(data,"triforceManifestFeed = (.-});")

	if glob.mtv_live_url == nil then
		glob.mtv_live_url = data:match("(http[%w%./:]+mtv%-germany%-live)")
	end

	data = nil
	collectgarbage()

	return videosection
end

function init()
	print("init:")

	collectgarbage()

	if vodeoPlay == nil then
		--vodeoPlay = video.new()
		vodeoPlay = neutrino.CMoviePlayerGui()
	end

	neutrino.CFileHelpers():createDir("/tmp/mtv")

	glob.fav_changed = false
	glob.mtv_artist={}
	glob.mtv={
			--{name = "Playlists",url="http://www.mtv.de/playlists",fav=false},
			{name = "MTV MUSIK",url="http://www.mtv.de/musik",fav=false},
			--{name = "MTV Unplugged",url="http://www.mtv.de/shows/w2nofb/mtv-unplugged",fav=false},
			{name = "Hitlist Germany - Top100",url="http://www.mtv.de/shows/fdc2nt/mtv-top-100",fav=false},
			{name = "MTV Shows",url="http://www.mtv.de/shows",fav=false},
			{name = "MTV Buzz",url="http://www.mtv.de/buzz",fav=false},
			--{name = "MTV PUSH",url="http://www.mtv.de/shows/z4pet5/mtv-push",fav=false},
			--{name = "MTV PLAYLIST",url="http://www.mtv.de/playlists",fav=false},
			{name = "ALLE MUSIKVIDEOS",url="http://www.mtv.de/musik",fav=false},
			{name = "Singel Top 100",url="http://www.mtv.de/charts/c6mc86/single-top-100",fav=false},
			--{name = "Album Top 100",url="http://www.mtv.de/charts/70tgrf/album-top-100",fav=false},
			{name = "Midweek Singel Top100",url="http://www.mtv.de/charts/n91ory/midweek-single-top-100",fav=false},
			--{name = "Midweek Album Top100",url="http://www.mtv.de/charts/ew735d/midweek-album-top-100",fav=false},
			{name = "Singel Trending",url="http://www.mtv.de/charts/9gtiy5/single-trending",fav=false},
			{name = "Top100 Musik Streaming",url="http://www.mtv.de/charts/h4oi23/top100-music-streaming",fav=false},
			{name = "Download Charts Singel",url="http://www.mtv.de/charts/pcbqpc/downloads-charts-single",fav=false},
			--{name = "Download Charts Album",url="http://www.mtv.de/charts/ha7dbg/downloads-charts-album",fav=false},
			{name = "Top15 deutsche Singel",url="http://www.mtv.de/charts/jlyhaa/top-15-deutschsprachige-single-charts",fav=false},
			--{name = "Top15 deutsche Alben",url="http://www.mtv.de/charts/wp4sh5/top-15-deutschsprachige-alben-charts",fav=false},
			--{name = "Hip Hop",url="http://www.mtv.de/charts/vh9pco/hip-hop-charts",fav=false},
			{name = "Dance Charts",url="http://www.mtv.de/charts/2ny5w9/dance-charts",fav=false},
	}

	local url = "http://www.mtv.de/charts"
	glob.mtv_live_url = nil
	videosection = get_json_data(url)
	if videosection == nil then 
		return nil 
	end

	local json = require "json"
	local jnTab = json.decode(videosection)
	if jnTab == nil  then 
		return nil 
	end

	local videosection_url = jnTab.manifest.zones.t5_lc_promo1.feed
	videosection = getdata(videosection_url)
	if videosection == nil then 
		return nil 
	end

	jnTab = json.decode(videosection)
	local pages = 1
	videosection_url = videosection_url:match("(.*)%d+$")

	local add = true
	local add2016 = false
	if jnTab.result and jnTab.result.chartTypeList then
		for k, v in ipairs(jnTab.result.chartTypeList) do
			if v.label and v.link then
				if v.label:find("Offizielle") == nil and v.label ~= "Deine Lieblingsvideos bei MTV" and v.label:find("Album") == nil and v.label:find("Alben") == nil and v.label:find("Vinyl") == nil then
					table.insert(glob.mtv,{name=v.label, url=v.link,})
					if add2016 == false and v.label == "Top 100 Jahrescharts 2015" then
						add2016 = true
						table.insert(glob.mtv,{name="Top 100 Jahrescharts 2016", url="http://www.mtv.de/charts/yrk67s/top-100-jahrescharts-2016",})
					end
				end
			end
		end
	end

	local mtvconf = get_conf_mtvfavFile()
	local havefile = neutrino.file_exists(mtvconf)
	if havefile == true then
		local favdata = read_file(mtvconf)
		if havefile ~= nil then
			for _name ,_url in favdata:gmatch('name%s-=%s-"(.-)"%s-,%s-url%s-=%s-"(.-)"') do
				table.insert(glob.mtv,{name=_name, url=_url,fav=true})
			end
		end
	end
end

function info(captxt, infotxt, sleep)
	if captxt == mtv_version and infotxt == nil then
		infotxt = captxt
		captxt = "Information"
	end
	--local msg, data = 0,0
	--local h = hintbox.new{caption=captxt, text=infotxt}
	local h = neutrino.CHintBox(captxt, infotxt)
	h:exec(10)
	--h:paint()
--[[
	if sleep then
		for i=1,sleep*5,1 do
			msg, data = n:GetInput(500)
			if msg == RC.ok or msg == RC.home then
				break
			end
		end
	else
		repeat
			msg, data = n:GetInput(500)
		until msg == RC.ok or msg == RC.home
	end
	h:hide()
]]
end

function getdata(Url, outputfile)
	print("getdata:")

	if Url == nil then 
		return nil 
	end
--[[
	if Curl == nil then
		Curl = curl.new()
	end
	local ret, data = Curl:download{url=Url,A="Mozilla/5.0;",maxRedirs=5,followRedir=true,o=outputfile }
	if ret == CURL.OK then
		if outputfile then
			return 1
		end
		return data
	else
		return nil
	end
]]
	local data = nil
	outputfile = "/tmp/mtv.txt"

	--local ret, data = Curl:download{url=Url,A="Mozilla/5.0;",maxRedirs=5,followRedir=true,o=outputfile }
	local ret = neutrino.downloadUrl(Url, outputfile, 'Mozilla/5.0;', 90)

	if ret == true then
		local fp = io.open(outputfile, "r")
		if fp == nil then
			error("Error opening file '" .. fname .. "'.")
		else
			data = fp:read("*all")
			fp:close()
		end
	end

	return data
end

function exist_url(tab,_url)
	for i, v in ipairs(tab) do
		if v.url == _url then
			return true
		end
	end

	return false
end

function getliste(url)
	--local h = hintbox.new{caption="Info", text="Liste wird erstellt\n"}
	--h:paint()
	local h = neutrino.CHintBox("Info", "Liste wird erstellt\n")
	h:paint()

	local videosection = get_json_data(url)

	if videosection == nil then 
		h:hide() 
		return nil 
	end

	local liste = {}
	local json = require "json"
	local urlTab = json.decode(videosection)
	local tc = {"t4_lc_promo1"}
	if url == "http://www.mtv.de/musik" then tc = {"t4_lc_promo1","t5_lc_promo1","t6_lc_promo1","t7_lc_promo1","t8_lc_promo1","t9_lc_promo1","t10_lc_promo1","t11_lc_promo1"}  		end

	for t in pairs(tc) do
		if urlTab.manifest.zones[tc[t]] and urlTab.manifest.zones[tc[t]].feed then
			local videosection_url = urlTab.manifest.zones[tc[t]].feed
			for p=1,6,1 do
				if videosection_url then
					local  pc = "?"
					if videosection_url:find("?") then 
						pc = "&" 
					end
					videosection = getdata(videosection_url .. pc .. "pageNumber=" .. p)
					if videosection == nil then
						if #liste > 0 then 
							h:hide() 
							return liste 
						else 
							return nil 
						end
					end
				else
					h:hide()
					return nil
				end

				local jnTab = json.decode(videosection)
				if jnTab == nil or jnTab.result == nil or jnTab.result.data == nil then 					h:hide()  
					if #liste > 0 then 
						return liste 
					else 
						return nil 
					end 
				end

				for k, v in ipairs(jnTab.result.data.items) do
					if v.videoUrl or v.canonicalURL then
						local video_url = v.videoUrl or v.canonicalURL
						if exist_url(liste,video_url) == false then
							local artist = v.shortTitle or v.artist or ""
							if #artist == 0 and v.artists then 
								artist = v.artists[1].name 
							end
							local _logo = nil
							if v.images and v.images.url then 
								_logo = v.images.url 
								--print("logo: ".. _logo .. "")
							end
							--_logo = _logo or ""
							
							tfile = "/tmp/mtv/" .. v.title ..".jpg"
							if _logo ~= nil and neutrino.file_exists(tfile) ~= true then
								neutrino.downloadUrl(_logo, tfile)
								_logo = tfile
							else
								_logo = neutrino.DATADIR .. "/neutrino/icons/nopreview.jpg"
							end
							
							local chpos = nil
							if v.chartPosition and v.chartPosition.current then 									chpos = v.chartPosition.current 
							end
							table.insert(liste,{name=artist .. ": " .. v.title, url=video_url,
								logo =_logo,enabled=conf.dlflag, vid=id,chartpos=chpos
								})
						end
					end
				end
			end
		end
	end
	h:hide()

--[[
	if liste ~= nil then
		print(liste)
	else
		print("NULL")
	end
]]

	return liste
end

function get_m3u_url(m3u8_url)
	if m3u8_url == nil then 
		return nil 
	end

	local videoUrl = nil
	local res = 0
	local data = getdata(m3u8_url)

	if data then
		local host = m3u8_url:match('([%a]+[:]?//[_%w%-%.]+)/')
		if m3u8_url:find('/master.m3u8') then
			local lastpos = (m3u8_url:reverse()):find("/")
			local hosttmp = m3u8_url:sub(1,#m3u8_url-lastpos)
			if hosttmp then
				host = hosttmp .."/"
			end
		end
		local maxRes = 1280
		if conf.maxRes then
			local maxResStr = conf.maxRes:match("(%d+)x")
			maxRes = tonumber(maxResStr)
		end
		for band, res1, res2, url in data:gmatch('BANDWIDTH=(%d+).-RESOLUTION=(%d+)x(%d+).-\n(.-)\n') do
			if url and res1 then
				local nr = tonumber(res1)
				if nr <= maxRes and nr > res then
					res=nr
					if host and url:sub(1,4) ~= "http" then
						url = host .. url
					end
					videoUrl = url
				end
			end
		end
	end

	return videoUrl
end

function getvideourl(url, vidname, hls)
	print("getvideourl:")

	local	json = require "json"
	local data = getdata(url)
	local id = data:match('itemId":"(.-)"')
	local service_url = "http://media.mtvnservices.com/pmt/e1/access/index.html?uri=mgid:arc:episode:mtv.de:" .. id .. "&configtype=edge&ref=" .. url
	data = getdata(service_url)
	local jnTab = json.decode(data)
	if jnTab.feed.items[1].group.content then
		local jsUrl = jnTab.feed.items[1].group.content
		if hls then
			jsUrl = jsUrl:gsub("(&device=.-)&","")
			jsUrl = jsUrl .. "&acceptMethods=hls"
		end
		data = getdata(jsUrl .. "&format=json")
		if data then
			jnTab = json.decode(data)
		end
	end
	data = nil

	local max_w = 0
	local video_url = nil
	if jnTab and jnTab.package and jnTab.package.video and jnTab.package.video.item and jnTab.package.video.item[1].rendition then
		for k,v in pairs(jnTab.package.video.item[1].rendition) do
			if (hls or v.width or v.rdminwidth) and v.src then
				local w = tonumber(v.width or v.rdminwidth)
				if hls or w > max_w then
					video_url = v.src
					max_w = w
				end
			end
		end
	end
	local x = nil
	if video_url then
		if video_url:find("m3u8") then
			video_url = get_m3u_url(video_url)
		end
		x = video_url:find("rtmp") or video_url:find("m3u8")
	end
	if not x then
	  print("########## Error ##########")
	  print(url,video_url,clip_page)
	  print("###########################")
	end
	if video_url and video_url:find("copyright_error") then
		if h then
			h:hide()
		end
		info("Video Not Available", "Copyright Error\n" .. vidname,2)
	end

	return video_url
end

--[[
function godirectkey(d)
	if d  == nil then return d end
	local  _dkey = ""
	if d == 1 then
		_dkey = RC["red"]
	elseif d == 2 then
		_dkey = RC["green"]
	elseif d == 3 then
		_dkey = RC["yellow"]
	elseif d == 4 then
		_dkey = RC["blue"]
	elseif d < 14 then
		_dkey = RC[""..d - 4 ..""]
	elseif d == 14 then
		_dkey = RC["0"]
	else
		-- rest
		_dkey = ""
	end
	return _dkey
end
]]

function action_exec(id)
	if id  then
		--local i = tonumber(id)
		local i = id
		if glob.MTVliste[i].name == nil then
			glob.MTVliste[i].name = "NoName_" .. i
		end
		local url = getvideourl(glob.MTVliste[i].url, glob.MTVliste[i].name, conf.hlsflag)
		if url then
			--hideMenu(glob.menu_liste)
			--vodeoPlay:setSinglePlay()
			--vodeoPlay:PlayFile(glob.MTVliste[i].name, url);
			vodeoPlay:addToPlaylist(url, glob.MTVliste[i].name)
			vodeoPlay:exec(null, "")
		end
	end
end

--[[
function gen_m3u_list(filename)
	local m3ufilename= conf.path_m3u .. "/" .. filename .. ".m3u"

	local m3ufile=io.open(m3ufilename,"w")
	if m3ufile then
		m3ufile:write("#EXTM3U name=" .. filename .. "\n")
		local infotext = "Dateien werden für M3U Liste vorbereitet.  "
		--local pw = cprogresswindow.new{title=infotext}
		local pw = neutrino.CProgressWindow()
		pw:setTitle(infotext)
		pw:paint()
		pw:showStatusMessageUTF("Start")
		--pw:paint()
		--pw:showStatus{statusText="Start"}
        	for k, v in ipairs(glob.MTVliste) do
			if v.name == nil then
				v.name = "NoName"
			end
			local url = getvideourl(v.url,v.name,conf.hlsflag)
			if url then
				local extinf = ", "
				if v.logo and #v.logo > 1 then --TODO Add Logo parse to CMoviePlayerGui::parsePlaylist
					extinf = " logo=" .. v.logo ..".jpg ,"
				end
				extinf = extinf .. v.name
				m3ufile:write("#EXTINF:-1".. extinf .."\n")
				m3ufile:write(url .."\n")
				--pw:showStatus{prog=k,max=#glob.MTVliste,statusText=tostring(k) .. "/" .. tostring(#glob.MTVliste) .. "  " .. v.name}
				pw:showGlobalStatus(k)
			end
		end
		m3ufile:close()
		pw:hide()
		info("Info", filename.." - Playlist wurde erstellt\n" .. m3ufilename,2)
	else
		info("Info", filename.." - Fehler beim erstellen:\n " .. m3ufilename .. " ",3)
	end
	--return MENU_RETURN.EXIT_REPAINT
end
]]

function make_shuffle_list(tab)
	local randTable={}
	for k=1 , #tab do
		randTable[k]=k
	end
	local shuffleTable = {}
	for k=1,#randTable do
		math.randomseed(os.time() *100000000000)
		local r=table.remove(randTable,math.random(#randTable))
		shuffleTable[k]=tab[r]
	end
	return shuffleTable
end

function playlist(filename)
	--hideMenu(glob.menu_liste)

	local tab = {}
	if conf.shuffleflag == true then
		tab = make_shuffle_list(glob.MTVliste)
	else
		tab = glob.MTVliste
	end

	local i = 1
	local KeyPressed = 0
	--vodeoPlay:setSinglePlay(false)
	repeat
		if tab[i].name == nil then
			tab[i].name = "NoName"
		end
		local url = getvideourl(tab[i].url,tab[i].name,conf.hlsflag)
		if url then
			local videoformat = url:sub(-4)
			if videoformat ~= ".flv" or conf.playflvflag then
				local prevn , nextn = "",""
				if i-1 ~= 0 then
					prevn = "Vorherige Titel: " .. tab[i-1].name
				end
				if i < #tab then
					nextn = "Nächste Titel: ".. tab[i+1].name
				end
				--KeyPressed = vodeoPlay:PlayFile( "(" .. i.. "/" .. #tab .. ") " .. tab[i].name,url, nextn, prevn)
				vodeoPlay:addToPlaylist(url,"(" .. i.. "/" .. #tab .. ") " .. tab[i].name)
				--vodeoPlay:exec(null ,"")
			end
		end

		i = i + 1
		--[[
		if KeyPressed == PLAYSTATE.NORMAL then --play continue
			i=i+1
		elseif KeyPressed == PLAYSTATE.STOP then
			i=0
			break
		elseif KeyPressed == PLAYSTATE.NEXT then
			i=i+1
		elseif KeyPressed == PLAYSTATE.PREV then
			i=i-1
		else
			print("Error")
			i=0
			break
		end
		]]

	until i==0 or i == #tab+1

	vodeoPlay:exec(null ,"")

	--return MENU_RETURN.EXIT_REPAINT
end

function dlstart(name)
	local infotext = "Dateien werden für Download vorbereitet.  "
	name = name:gsub([[%s+]], "_")
	name = name:gsub("[:'&()/]", "_")
	local dlname = "/tmp/" .. name ..".dl"
	local havefile = neutrino.file_exists("/tmp/.rtmpdl")
	if havefile == true then
		info("Info", "Ein anderer Download ist bereits aktiv.", 4)
		return
	end
	local dl=io.open(dlname,"w")
	local script_start = false

	--local pw = cprogresswindow.new{title=infotext}
	local pw = neutrino.CProgressWindow()

	pw:setTitle(infotext)
	pw:paint()
	--pw:paint()
	--pw:showStatus{statusText="Start"}
	pw:showStatusMessageUTF("Start")
	for i, v in ipairs(glob.MTVliste) do
		if v.enabled == true then
			if glob.MTVliste[i].name == nil then
				glob.MTVliste[i].name = "NoName_" .. i
			end
			local url = getvideourl(glob.MTVliste[i].url,glob.MTVliste[i].name,conf.hlsflag)
			if url then
				local fname = v.name:gsub([[%s+]], "_")
				fname = fname:gsub("[:'()]", "_")
				fname = fname:gsub("/", "-")
				pw:showStatus{prog=i,max=#glob.MTVliste,statusText=tostring(i) .. "/" .. tostring(#glob.MTVliste) .. "  " .. fname}
				local videoformat = url:sub(-4)
				if videoformat == nil then
					videoformat = ".mp4"
				end
				if conf.hlsflag then
					dl:write("ffmpeg -y -nostdin -loglevel 30 -i " .. url .. " -c copy  " .. conf.path .. "/" .. fname   .. ".ts\n")
					script_start = true
				elseif videoformat ~= ".flv" or conf.flvflag then
					dl:write("rtmpdump -e -r " .. url .. " -o " .. conf.path .. "/" .. fname  .. videoformat .."\n")
					script_start = true
				end
			end
		end
	end
	pw:hide()
	if script_start == true then
	dl:close()
	local scriptname  = "/tmp/" .. name ..".sh"
	local script=io.open(scriptname,"w")
	script:write(
	[[#!/bin/sh
	while read -r i
	do
		$i
	done < ]]
	)
	script:write("'" .. dlname .. "'\n")
	script:write([[
	wget -q 'http://127.0.0.1/control/message?popup=Video Liste ]])
		script:write(name .. " wurde heruntergeladen.' -O /dev/null\n")
		script:write("rm '" .. dlname .. "'\n")
		script:write("rm '" .. scriptname .. "'\n")
		script:write("rm /tmp/.rtmpdl\n")

		script:close()
		os.execute("echo >/tmp/.rtmpdl")
		os.execute("sleep 2")
		os.execute("chmod 755 '" .. scriptname .. "'")
		os.execute("sh '"..scriptname.."' &")
	else
		local er = hintbox.new{caption="Info", text=name .." - \nDownload ist fehlerhaft \noder Video in FLV-Format"}
		er:paint()
		os.remove(dlname)
		print("ERROR")
		os.execute("sleep 2")
		er:hide()
	end
end

function exist(_url)
	for i, v in ipairs(glob.mtv) do
		if v.fav == true and v.url == _url then
			return true
		end
	end
	return false
end

function addfav(id)
	local addinfo = false
	for i, v in ipairs(glob.mtv_artist) do
		if v.enabled and exist(v.url) == false then
			table.insert(glob.mtv,{name=v.name, url=v.url,fav=true})
			glob.mtv_artist[i].disabled=true
			glob.fav_changed = true
			addinfo = true
		end
	end
	if addinfo == true then
		info("Info","Zu Favoriten hinzugefügt",2)
	end
end

function favdel(id)
	local delinfo = false
	for i, v in ipairs(glob.mtv) do
		if v.fav and v.enabled then
			table.remove(glob.mtv,i)
			glob.fav_changed = true
			delinfo = true
		end
	end
	if delinfo == true then
		info("Info","Ausgewählten Favoriten gelöscht",2)
	end

end

function chooser_menu(id)
--[[
	if id:sub(1,29) =="Erstelle Download Liste für " then
		local forwarder_action = "dlstart"
		local forwarder_name = "Download starten"
		local chooser_action = "set_bool_in_liste"
		local hintname = "Speichert die ausgewählten Videos unter: " .. conf.path
		local _id = id:sub(30,#id)
		local name = id
		local value=conf.dlflag
		gen_chooser_menu(glob.MTVliste, name, _id, chooser_action, forwarder_action, forwarder_name, hintname, value, glob.menu_liste)
	elseif id:sub(1,15) =="Neue Favoriten" then
		local forwarder_action = "addfav"
		local forwarder_name = "Zu Favoriten hinzufügen"
		local chooser_action = "set_bool_in_searchliste"
		local hintname = "Speichert die ausgewählten Videos unter: " .. conf.path
		local name = id .. " hinzufügen"
		local value=false
		gen_chooser_menu(glob.mtv_artist, name , id, chooser_action, forwarder_action, forwarder_name, hintname, value, glob.search_artists_menu)
	elseif id =="favdel" then
		local forwarder_action = "favdel"
		local forwarder_name = "Favoriten löschen"
		local chooser_action = "set_bool_in_mtv"
		local hintname = "Lösche die ausgewählten Videos."
		local name = id .. " hinzufügen"
		local value=false
		gen_chooser_menu(glob.mtv, name , id, chooser_action, forwarder_action, forwarder_name, hintname, value, glob.settings_menu)

	end
]]
end

function set_bool_in_liste(k, v)
	local i = tonumber(k)
	if v == on then
		glob.MTVliste[i].enabled=true
	else
		glob.MTVliste[i].enabled=false
	end
end

function set_bool_in_searchliste(k, v)
	local i = tonumber(k)
	if v == on then
		glob.mtv_artist[i].enabled=true
	else
		glob.mtv_artist[i].enabled=false
	end
end

function set_bool_in_mtv(k, v)
	local i = tonumber(k)
	if v == on then
		glob.mtv[i].enabled=true
	else
		glob.mtv[i].enabled=false
	end
end

function gen_chooser_menu(table, name , _id, _chooser_action, _forwarder_action, _forwarder, _hintname, _value, hidemenu)
--[[
	hideMenu(hidemenu)
	local menu =  menu.new{name=name, icon="icon_red"}
	menu:addItem{type="back"}
	menu:addItem{type="separatorline"}
	local d = 1 -- directkey

	menu:addItem{type="forwarder", name=_forwarder, action=_forwarder_action, enabled=true,
	id=_id, directkey=godirectkey(d),hint=_hintname}
	menu:addItem{type="separatorline"}
	for i, v in ipairs(table) do
		if (("favdel" == _forwarder_action and v.fav==true) or "addfav" ==  _forwarder_action or "dlstart" ==  _forwarder_action) then
			d = d + 1
			local dkey = godirectkey(d)
			menu:addItem{type="chooser", action=_chooser_action, options={ on, off }, id=i, value=bool2onoff(_value), 
			name= i .. ": " ..v.name, hint_icon="hint_service",hint=v.name .. " speichern ? Ein/Aus"}
		end
	end
	menu:exec()
	menu:hide()
	return MENU_RETURN.EXIT_REPAINT
]]
end

function __menu(_menu, menu_name, table, _action)
	print("__menu:")

	if table == nil or #table == 0 then
		info("Info", "Liste ist leer.", 1)
		return
	end

	--hideMenu(glob.mtv_listen_menu)

	--_menu:addItem{type="back"}
	--_menu:addItem{type="separatorline"}
	local d = 1 -- directkey
	local playhint = "Playlist: "
	if conf.shuffleflag then
		playhint = "Playlist in zufällig Reihenfolge: "
	end
	--[[_menu:addItem{type="forwarder", name="Playlist", action="playlist", enabled=true,
	id="Playlist " .. menu_name, directkey=godirectkey(d),hint=playhint .. menu_name}]]
	_menu:addItem(neutrino.CMenuForwarder("Playlist"))
        --d=d+1
	--[[_menu:addItem{type="forwarder", name="Erstelle M3U Playlist", action="gen_m3u_list", enabled=true,
	id=menu_name, directkey=godirectkey(d),hint="Erstelle eine M3U Playlist im Verzeichnis: " .. conf.path_m3u .. "/" .. menu_name .. ".m3u"}]]
	--d = d + 1
	--[[_menu:addItem{type="forwarder", name="Erstelle Download Liste", action="chooser_menu", enabled=glob.have_rtmpdump,
			id="Erstelle Download Liste für "..menu_name, directkey=godirectkey(d),hint="Welche Videos sollen heruntergeladen werden ?"}
	_menu:addItem{type="separatorline"}]]
	d = d + 1 --skip blue

	for i, v in ipairs(table) do
		--d = d + 1
		--local dkey = godirectkey(d)
		local cont = i .. ": "
		if v.chartpos then
			cont = " (" .. v.chartpos .. ") "
		end
		--_menu:addItem{type="forwarder", name=cont .. v.name, action=_action,enabled=true,id=i,directkey=dkey,hint=""}
		_menu:addItem(neutrino.CMenuForwarder(cont .. v.name))
	end
	_menu:exec(null, "")
	--_menu:hide()
	--return MENU_RETURN.EXIT_REPAINT
end

function mtv_liste(id)
	print("mtv_liste:")

	--
	neutrino.CFileHelpers():removeDir("/tmp/mtv")
	neutrino.CFileHelpers():createDir("/tmp/mtv")
	--

	--local i = tonumber(id)
	local i = id
	glob.MTVliste = nil;
	local url = glob.mtv[i].url
	glob.MTVliste = getliste(url)
	--glob.menu_liste  = menu.new{name=glob.mtv[i].name, icon="icon_blue"}
	glob.menu_liste = neutrino.ClistBoxWidget(glob.mtv[i].name, neutrino.PLUGINDIR .. "/mtv/mtv_hint.png")
	glob.menu_liste:setWidgetType(neutrino.WIDGET_TYPE_FRAME)
	glob.menu_liste:setItemsPerPage(3, 2)
	glob.menu_liste:addWidget(neutrino.WIDGET_TYPE_CLASSIC)
	glob.menu_liste:enableWidgetChange()

	local item = nil
	if glob.MTVliste ~= nil then
		--glob.menu_liste = neutrino.ClistBoxWidget(glob.mtv[i].name, neutrino.NEUTRINO_ICON_MOVIE)
		--__menu(glob.menu_liste, glob.mtv[i].name, glob.MTVliste, "action_exec")
		collectgarbage()
		--return MENU_RETURN.EXIT_REPAINT

		--glob.menu_liste:addItem(neutrino.CMenuForwarder("Playlist"))

		for i, v in ipairs(glob.MTVliste) do
			--d = d + 1
			--local dkey = godirectkey(d)
			local cont = i .. ": "
			if v.chartpos then
				cont = " (" .. v.chartpos .. ") "
			end

			item = neutrino.ClistBoxItem(v.name, true, "", null, "play")

			item:setItemIcon(v.logo)

			glob.menu_liste:addItem(item)
		end
		--_menu:addItem{type="forwarder", name=cont .. v.name, action=_action,enabled=true,id=i,directkey=dkey,hint=""}
		--_menu:addItem(neutrino.CMenuForwarder(cont .. v.name))
	end

	repeat
		glob.menu_liste:exec(null, "")

		selected = glob.menu_liste:getSelected()
		actionKey = glob.menu_liste:getActionKey()

		--if selected >= 0 then
		if actionKey == "play" then
			--if selected == 0 then
			--	playlist(selected + 1)
			--else
			action_exec(selected + 1)
			--end
		end
	until glob.menu_liste:getExitPressed() == true
end

function set_path(id,value)
	if conf[id] ~= value then
		conf[id]=value
		conf.changed = true
	end
end

function set_option(k, v)
	if k == "maxRes" then
		conf[k]=v
	end

	if v == on then
		conf[k]=true
	elseif v == off then
		conf[k]=false
	end
	if k == "hlsflag" then
		glob.have_rtmpdump=nil
		if conf[k] then
			glob.have_rtmpdump=which("ffmpeg")
		else
			glob.have_rtmpdump=which("rtmpdump")
		end
	end
	conf.changed = true
end

function bool2onoff(a)
	if a then return on end
	return off
end

local m_selected = -1
function setings()
	--hideMenu(glob.main_menu)
--[[
	local d =  1
	local menu =  menu.new{name="Einstellungen", icon="icon_blue"}
	glob.settings_menu = menu
	menu:addItem{type="back"}
	menu:addItem{type="separatorline"}
	menu:addItem{ type="filebrowser", dir_mode="1", id="path", name="Verzeichnis: ", action="set_path",
		   enabled=true,value=conf.path,directkey=godirectkey(d),
		   hint_icon="hint_service",hint="In welchem Verzeichnis soll das Video gespeichert werden ?"
		 }
	d=d+1
	menu:addItem{ type="filebrowser", dir_mode="1", id="path_m3u", name="Verzeichnis M3U: ", action="set_path",
		   enabled=true,value=conf.path_m3u,directkey=godirectkey(d),
		   hint_icon="hint_service",hint="In welchem Verzeichnis soll das M3U Playlist gespeichert werden ?"
		 }
	d=d+1
	menu:addItem{type="chooser", action="set_option", options={ on, off }, id="hlsflag", value=bool2onoff(conf.hlsflag), directkey=godirectkey(d), name="Videos in HLS-Format",hint_icon="hint_service",hint="HLS als bevorzugte Video Format 'ein' oder 'aus'"}
	d=d+1
	local res_opt={ '3840x2160','2560x1440','1920x1080','1280x720','854x480','640x360' }
	menu:addItem{type="chooser", action="set_option", options=res_opt, id="maxRes", value=conf.maxRes, name="Max. Auflösung" ,directkey=godirectkey(d),hint_icon="hint_service",hint="Max. Video Auflösung"}
	d=d+1
	menu:addItem{type="chooser", action="set_option", options={ on, off }, id="dlflag", value=bool2onoff(conf.dlflag), directkey=godirectkey(d), name="Auswahl vorbelegen mit",hint_icon="hint_service",hint="Erstelle Auswahlliste mit 'ein' oder 'aus'"}
	d=d+1
	menu:addItem{type="chooser", action="set_option", options={ on, off }, id="flvflag", value=bool2onoff(conf.flvflag), directkey=godirectkey(d), name="Videos in FLV-Format herunterladen ? ",hint_icon="hint_service",hint="Videos in FLV-Format sind meisten mit nicht kompatiblen Video-Codec (zB: vp6f)."}
	d=d+1
	menu:addItem{type="chooser", action="set_option", options={ on, off }, id="playflvflag", value=bool2onoff(conf.playflvflag),
	directkey=godirectkey(d), name="Videos in FLV-Format abspielen ? ",hint_icon="hint_service",hint="Videos in FLV-Format sind meisten mit nicht kompatiblen Video-Codec (zB: vp6f)."}
	d=d+1
	menu:addItem{type="chooser", action="set_option", options={ on, off }, id="shuffleflag", value=bool2onoff(conf.shuffleflag),
	directkey=godirectkey(d), name="Zufällig abspielen ? ",hint_icon="hint_service",hint="Video wiedergabelisten zufällig abspielen. "}

	menu:addItem{type="separatorline"}
	d=d+1
	menu:addItem{type="forwarder", name="Ausgewählte Favoriten löschen.", action="chooser_menu", enabled=true,
	id="favdel", directkey=godirectkey(d),hint="Favoriten bearbeiten"}
	menu:exec()
	menu:hide()
	return MENU_RETURN.EXIT_REPAINT
]]
	loadConfig()

	local menu = neutrino.ClistBoxWidget("Einstellungen", neutrino.NEUTRINO_ICON_SETTINGS)
	local item = nil
	menu:setMode(neutrino.MODE_SETUP)
	menu:enableShrinkMenu()

	menu:addItem(neutrino.CMenuForwarder(neutrino.LOCALE_MENU_BACK))
	menu:addItem(neutrino.CMenuSeparator(neutrino.LINE))
	item = neutrino.CMenuForwarder("Einstellungen speichern")
	item:setIconName(neutrino.NEUTRINO_ICON_BUTTON_RED)
	item:setDirectKey(neutrino.RC_red)
	menu:addItem(item)
	menu:addItem(neutrino.CMenuSeparator(neutrino.LINE))
	menu:addItem(neutrino.CMenuForwarder("Verzeichniss:", true, conf.path))
	menu:addItem(neutrino.CMenuForwarder("Verzeichniss M3U:", true, conf.path_m3u))
	
	hls = ""
	if conf.hlsflag == true then
		hls = "on"
	else
		hls = "off"
	end
	menu:addItem(neutrino.CMenuForwarder("Videos in HLS-Format", true, hls))
	
	--local res_opt={ '3840x2160','2560x1440','1920x1080','1280x720','854x480','640x360' }

	menu:addItem(neutrino.CMenuForwarder("Max. Auflösung", true, conf.maxRes))

	dl = ""
	if conf.dlflag == true then
		dl = "on"
	else
		dl = "off"
	end
	menu:addItem(neutrino.CMenuForwarder("Auswahl vorbelegen mit", true, dl))

	if m_selected < 0 then
		m_selected = 0
	end

	menu:setSelected(m_selected)

	menu:exec(null, "")

	m_selected = menu:getSelected()

	if m_selected >= 0 then
		if m_selected == 0 then
			return
		elseif m_selected == 2 then
			neutrino.HintBox("MTV", "Einstellungen werden gespeichert bitte warten...")
			saveConfig()
		elseif m_selected == 4 then
			local fileBrowser = neutrino.CFileBrowser()
			fileBrowser.Dir_Mode = true

			if fileBrowser:exec(conf.path) == true then
				conf.path = fileBrowser:getSelectedFile().Name

				saveConfig()
			end 
		elseif m_selected == 5 then
			local fileBrowser = neutrino.CFileBrowser()
			fileBrowser.Dir_Mode = true

			if fileBrowser:exec(conf.path_m3u) == true then
				conf.path_m3u = fileBrowser:getSelectedFile().Name

				saveConfig()
			end
		elseif m_selected == 6 then 
			if conf.hlsflag == true then
				conf.hlsflag = false
			else
				conf.hlsflag = true
			end

			saveConfig()
		elseif m_selected == 7 then
			if conf.maxRes == '3840x2160' then
				conf.maxRes = '2560x1440'
			elseif conf.maxRes == '2560x1440' then
				conf.maxRes = '1920x1080'
			elseif conf.maxRes == '1920x1080' then
				conf.maxRes = '1280x720'
			elseif conf.maxRes == '1280x720' then
				conf.maxRes = '854x480'
			elseif conf.maxRes == '854x480' then
				conf.maxRes = '640x360'
			elseif conf.maxRes == '640x360' then
				conf.maxRes = '3840x2160'
			end

			saveConfig()
		elseif m_selected == 8 then 
			if conf.dlflag == true then
				conf.dlflag = false
			else
				conf.dlflag = true
			end

			saveConfig()
		end
	end

	if menu:getExitPressed() ~= true then
		setings()
	end
end

function gen_search_list(search)
	local url = "http://www.mtv.de/kuenstler/" .. search:sub(1,1) .. "/1"
	glob.mtv_artist = {}
	local videosection = get_json_data(url)
	if videosection == nil then return nil end

	data = nil
	collectgarbage()

	if videosection == nil then return nil end

	local json = require "json"
	local jnTab = json.decode(videosection)
	if jnTab == nil  then return nil end
	local videosection_url = jnTab.manifest.zones.t5_lc_promo1.feed
	videosection = getdata(videosection_url)
	if videosection == nil then return nil end

	jnTab = json.decode(videosection)
	local pages = 1
	videosection_url = videosection_url:match("(.*)%d+$")
	if jnTab.result.pages then pages = tonumber(jnTab.result.pages) end

	for p=1,pages,1 do
		if p > 1 then
			if videosection_url then
				videosection = getdata(videosection_url .. p)
				if videosection == nil then
					return MENU_RETURN.EXIT_REPAINT
				end
			else
				return MENU_RETURN.EXIT_REPAINT
			end
			jnTab = json.decode(videosection)
		end

		if jnTab == nil or jnTab.result == nil or jnTab.result.artists == nil then return MENU_RETURN.EXIT_REPAINT end

		local add = true
		local use_seek = #search > 1
		for k, v in ipairs(jnTab.result.artists) do
			if v.name and v.canonicalURL then
				if use_seek then
					local name = v.name:lower()
					local a,b = name:find(search:lower())
					if a == 1 and b then add = true else add = false end
				end
				if add then
					table.insert(glob.mtv_artist,{name=v.name, url=v.canonicalURL, enabled=false,disabled=false})
				end
			end
		end
	end
end

function searchliste(id)
	--hideMenu(glob.artists_menu)
	--local i = tonumber(id)
	local i = id
	glob.MTVliste = nil;
	if glob.mtv_artist[i].disabled == true then 
		return 
	end

	local url = glob.mtv_artist[i].url
	glob.MTVliste = getliste(url)
	--glob.menu_liste  = menu.new{name=glob.mtv_artist[i].name, icon="icon_blue"}
	glob.menu_liste = neutrino.ClistBoxWidget(glob.mtv_artist[i].name, neutrino.PLUGINDIR .. "/mtv/mtv_hint.png")

	__menu(glob.menu_liste ,glob.mtv_artist[i].name, glob.MTVliste,"action_exec")
	collectgarbage()
	--return MENU_RETURN.EXIT_REPAINT
end

function search_artists()
	if conf.search == nil then 
		return 
	end

	--hideMenu(glob.main_menu)
	--local h = hintbox.new{caption="Info", text="Suche: " .. conf.search}
	local h = neutrino.CHintBox("Info", "Suche: " .. conf.search)
	h:paint()

	if #conf.search > 0 then
		gen_search_list(conf.search)
	end
	h:hide()
	if glob.mtv_artist == nil or #glob.mtv_artist == 0 then
		info("Info", "Liste ist leer.", 1)
		return
	end

	local d = 1
	--local menu =  menu.new{name=conf.search, icon="icon_yellow"}
	local menu = neutrino.ClistBoxWidget(conf.search, neutrino.PLUGINDIR .. "/mtv/mtv_hint.png")
	glob.search_artists_menu = menu
	glob.artists_menu = menu
	--menu:addItem{type="back"}
	--menu:addItem{type="separatorline"}
	--[[menu:addItem{type="forwarder", name="Neue Favoriten hinzufügen", action="chooser_menu", enabled=true,
	id="Neue Favoriten", directkey=godirectkey(d),hint="Listen für Favoriten auswählen und hinzufügen."}]]
	menu:addItem(neutrino.CMenuForwarder("Neue Favoriten hinzufügen"))
	--menu:addItem{type="separatorline"}

	if glob.mtv_artist then
		for i, v in ipairs(glob.mtv_artist) do
			--d = d + 1
			--local dkey = godirectkey(d)
			--menu:addItem{type="forwarder", name=i .. ": " .. v.name, action="searchliste",enabled=true,id=i,directkey=dkey,hint="Suchwort-Liste für " .. conf.search}
			menu:addItem(neutrino.CMenuForwarder(i .. ": " .. v.name))
		end
	end

	menu:exec(null, "")

	local selected = menu:getSelected()

	if selected >= 0 then
		if selected == 0 then
			chooser_menu()
		elseif selected > 0 then
			searchliste()
		end
	end

	if menu:getExitPressed() ~= true then
		search_artists()
	end
	--menu:hide()
end

function play_live()
	local video_url = getvideourl(glob.mtv_live_url, "live", true)
	if video_url then
		--hideMenu(glob.main_menu)
		--vodeoPlay:setSinglePlay()
		--vodeoPlay:PlayFile("MTV Live Stream", video_url);
		vodeoPlay:addToPlaylist(video_url, "MTV Live Stream")
		vodeoPlay:exec(null, "")
	end
	--return MENU_RETURN.EXIT_REPAINT
end

local selected_mtl = 0
function mtv_listen_menu()
	print("mtv_listen_menu:")

	if glob.mtv == nil then
		return
	end

	--hideMenu(glob.main_menu)
	--glob.mtv_listen_menu  = menu.new{name="MTV Listen", icon="icon_red"}
	menu = neutrino.ClistBoxWidget("MTV Listen", neutrino.PLUGINDIR .. "/mtv/mtv_hint.png")
	glob.mtv_listen_menu = menu

	if selected_mtl < 0 then
		selected_mtl = 0
	end

	menu:setSelected(selected_mtl)

	--menu:addItem{type="back"}
	--menu:addItem{type="separatorline"}
	--local d = 0 -- directkey
	local item = nil
	for i, v in ipairs(glob.mtv) do
		--d = d + 1
		--local dkey = godirectkey(d)
		--menu:addItem{type="forwarder", name=v.name, action="mtv_liste",enabled=true,id=i,directkey=dkey,hint=v.url}
		item = neutrino.ClistBoxItem(v.name)
		item:setHelpText(v.url)
		item:setActionKey(null, "mtv_liste")

		menu:addItem(item)
	end

	menu:exec(null, "")

	selected_mtl = menu:getSelected()
	actionKey = menu:getActionKey()

	if actionKey == "mtv_liste" then
		mtv_liste(selected_mtl + 1)
	end

--[[
	selected_mtl = menu:getSelected()

	if selected_mtl >= 0 then
		mtv_liste(selected_mtl + 1)
	end
]]
	
	if menu:getExitPressed() ~= true then
		mtv_listen_menu()
	end
	
	--menu:hide()
	--return MENU_RETURN.EXIT_REPAINT
end

local selected_mm = 0
function main_menu()
	print("mainMenu:")

	--glob.main_menu  = menu.new{name="MTV", icon="icon_red"}
	local menu = neutrino.ClistBoxWidget("MTV", neutrino.PLUGINDIR .. "/mtv/mtv_hint.png")
	menu:setMode(neutrino.MODE_MENU)
	menu:enableShrinkMenu()
	glob.main_menu = menu

	if selected_mm < 0 then
		selected_mm = 0
	end

	menu:setSelected(selected_mm)

	--local d = 1 -- directkey

	--menu:addKey{directkey=RC["info"], id=mtv_version, action="info"}
	--menu:addItem{type="back"}
	--menu:addItem{type="separatorline"}
	--[[menu:addItem{type="forwarder", name="MTV Listen", action="mtv_listen_menu", enabled=true,
	id="dummy"..d, directkey=godirectkey(d),hint="MTV Listen"}]]
	menu:addItem(neutrino.CMenuForwarder("MTV Listen", true, "", null, "list"))
        --d=d+1
	--menu:addItem{type="separatorline"}
	--[[menu:addItem{type="forwarder", name="Suche nach Künstler", action="search_artists", enabled=true,
	id="find", directkey=godirectkey(d),hint="Suche nach Künstler"}]]
	menu:addItem(neutrino.CMenuForwarder("Suche nach künstler", true, conf.search, null, "search"))
	--d=d+1
	--menu:addItem{type="keyboardinput", action="setvar", id="search", name="Künstler Name:", value=conf.search,directkey=godirectkey(d),hint_icon="hint_service",hint="Nach welchem Künstler soll gesucht werden ?"}
	--menu:addItem{type="separatorline"}
	--d=d+1
	--[[menu:addItem{type="forwarder", name="MTV Live", action="play_live", enabled=(glob.mtv_live_url~=nil),
	id="dummy", directkey=godirectkey(d),hint="MTV Live Stream"}]]
	menu:addItem(neutrino.CMenuForwarder("MTV Live", glob.mtv_live_url~=nil, "", null, "live"))

	--menu:addItem{type="separatorline"}
	--d=d+1
	--[[menu:addItem{type="forwarder", name="Einstellungen", action="setings", enabled=true,
	id="dummy"..d, directkey=godirectkey(d),hint="Einstellungen"}]]
	menu:addItem(neutrino.CMenuForwarder("Einstellugen", true, "", null, "settings"))

	menu:exec(null, "")

	actionKey = menu:getActionKey()

	if actionKey == "list" then
		mtv_listen_menu()
	elseif actionKey == "search" then
		search_artists()
	elseif actionKey == "live" then
		play_live()
	elseif actionKey == "settings" then
		setings()
	end

	if menu:getExitPressed() ~= true then
		main_menu()
	end
end

function main()
	init()
	loadConfig()
	glob.have_rtmpdump=nil
	if conf.hlsflag then
		glob.have_rtmpdump=which("ffmpeg")
	else
		glob.have_rtmpdump=which("rtmpdump")
	end
	main_menu()
	saveConfig()
	neutrino.CFileHelpers():removeDir("/tmp/mtv")
	collectgarbage()
	os.execute("rm /tmp/lua*");
end

main()


