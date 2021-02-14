--[[
	Wunderschön-App
	Vers.: 0.4
	Copyright (C) 2019-2021, Udog

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

        Copyright (C) for the linked videos and for the Logo by the the respective owners!
        Copyright (C) for the Base64 encoder/decoder function by Alex Kloss <alexthkloss@web.de>, licensed under the terms of the LGPL
]]

local json = require "json"

--Objekte
--[[
function script_path()
	local str = debug.getinfo(2, "S").source:sub(2)
	return str:match("(.*/)")
end
]]

function init()
	--n = neutrino();
	p = {}
	func = {}
	pmid = 0
	stream = 1
        tmpPath = "/tmp"
	zdf = decodeImage("data:image/png;base64,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")
end

function add_stream(t,u,f)
  p[#p+1]={title=t,url=u,from=f,access=stream}
end

--[[
function getdata(Url,outputfile)
	if Url == nil then return nil end
	--if Curl == nil then
	--	Curl = curl.new()
	--end
	--local ret, data = Curl:download{url=Url,A="AppleSyndication/56.1",followRedir=true,o=outputfile }
	--downloadUrl(std::string url, std::string file, const std::string userAgent = " ", unsigned int timeout = 60);
	local ret = downloadUrl(Url, outputfile, "AppleSyndication/56.1")

	--if ret == true then
	--	return data
	--else
	--	return nil
	--end
end
]]

-- Base64 encoder/decoder function

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

local tmpImg
function decodeImage(b64Image)
	local imgTyp = b64Image:match("data:image/(.-);base64,")
	local repData = "data:image/" .. imgTyp .. ";base64,"
	local b64Data = string.gsub(b64Image, repData, "");

	tmpImg = os.tmpname()
	local retImg = tmpImg .. "." .. imgTyp

	local f = io.open(retImg, "w+")
	f:write(dec(b64Data))
	f:close()
--	os.remove(tmpImg) -- enable only for testing

	return retImg
end

function fill_playlist() --- > begin playlist
   add_stream("Sehnsuchtsziel Norditalien","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/209/2090586/2090586_25784663.mp4","Folge 1,")
   add_stream("Süditalien Spezial","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/209/2095697/2095697_25881513.mp4","Folge 2")
   add_stream("Ostsee-Reisen - von Rügen bis Fehmarn","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/170/1702724/1702724_19788356.mp4","Folge 3")
   add_stream("Durch das wilde Mecklenburg","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/189/1890654/1890654_22303881.mp4","Folge 4")
   add_stream("Ostsee-Highlights II - zwischen Darß und Kurischer Nehrung","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/197/1975998/1975998_23838540.mp4","Folge 5")
   add_stream("Romantisches Neckartal - Von Heilbronn bis Heidelberg","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/211/2115883/2115883_26223176.mp4","Folge 6")
   add_stream("Das neue Neanderland","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/211/2115642/2115642_26221165.mp4","Folge 7")
   add_stream("Rheinreise - Unterwegs mit Tamina Kallert und Stefan Pinnow","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/211/2115717/2115717_26222796.mp4","Folge 8")
   add_stream("Mit Schiff und Rad durch Holland","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/175/1757049/1757049_20516295.mp4","Folge 9")
   add_stream("Romantisches Holland - Zwischen Tulpenpracht und Königskrone","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/212/2122466/2122466_26331359.mp4","Folge 10")
   add_stream("Durch das wilde Mecklenburg","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/213/2130886/2130886_26467666.mp4","Folge 11")
   add_stream("Peloponnes im Frühling","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/213/2134947/2134947_26526094.mp4","Folge 12")
   add_stream("Kreta: Zeus, Raki und Sirtaki","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/213/2131012/2131012_26468146.mp4","Folge 13")
   add_stream("Ferien an der Nordsee - Von der Elbe bis nach Husum","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/193/1932338/1932338_23071918.mp4","Folge 14")
   add_stream("Familieninsel Föhr","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/213/2139484/2139484_26611578.mp4","Folge 15")
   add_stream("Die Sommer-Radtour durch den Westen","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/214/2147448/2147448_26723550.mp4","Folge 16")
   add_stream("Die Azoren - Naturerlebnis im Atlantik","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/214/2144459/2144459_26674676.mp4","Folge 17")
   add_stream("Ferien an der Mosel","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/215/2152417/2152417_26802024.mp4","Folge 18")
   add_stream("Berchtesgadener Land - Hochgefühle in den Alpen","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/215/2155977/2155977_26858706.mp4","Folge 19")
   add_stream("Auf dem Tauernradweg","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/215/2156080/2156080_26860747.mp4","Folge 20") 
   add_stream("Irlands grüner Süden","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/216/2160703/2160703_26934397.mp4","Folge 21")
   add_stream("Schottlands Nordwesten - von den Highlands zu den Äußeren Hebriden","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/216/2161558/2161558_27022576.mp4","Folge 22")
   add_stream("Die Sieg - Von der Quelle bis zur Mündung","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/216/2165370/2165370_27011414.mp4","Folge 23")
   add_stream("Der Niederrhein - Weiter Himmel, grünes Land","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/216/2165365/2165365_27011238.mp4","Folge 24")
   add_stream("Entlang der Lahn","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/215/2152403/2152403_26801514.mp4","Folge 25")
   add_stream("Pellworm - Grüne Insel im Wattenmeer","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/202/2024944/2024944_24663321.mp4","Folge 26")
   add_stream("Der Darß - Deutschlands schöne Halbinsel","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/218/2180100/2180100_27266533.mp4","Folge 27")
   add_stream("Sylt für alle","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/216/2169475/2169475_27079568.mp4","Folge 28")
   add_stream("Kleinwalsertal: Öko-Urlaub in Österreich","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/215/2155951/2155951_26858339.mp4","Folge 29")
   add_stream("Urlaub in Nordholland","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/202/2024935/2024935_24659671.mp4","Folge 30")
   add_stream("Wunderschoene Nordseeinseln","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/207/2075829/2075829_25542693.mp4","Folge 31")
   add_stream("Die südliche Toskana","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/219/2193393/2193393_27513983.mp4","Folge 32")
   add_stream("Das Allgäu - dem Himmel so nah","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/219/2193384/2193384_27513294.mp4","Folge 33")
   add_stream("Mit dem E-Bike durch die Alpen - von Salzburg bis zur Adria","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/219/2193505/2193505_27517215.mp4","Folge 34")
   add_stream("Limfjord, Løgstor, Aalborg, Sæby","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/219/2198105/2198105_27612735.mp4","Folge 35")
   add_stream("Auf nach Ameland!","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/220/2203402/2203402_27730605.mp4","Folge 36")
   add_stream("Die Sommer-Radtour durch den Westen","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/220/2207983/2207983_27836124.mp4","Folge 37")
   add_stream("Auf der Friedensroute zwischen Münster und Osnabrück","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/220/2207941/2207941_27827226.mp4","Folge 38")
   add_stream("Von Aachen nach Trier - mit dem Rad durch drei Länder","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/221/2213752/2213752_27955146.mp4","Folge 39")
   add_stream("Durch die wilde Nordeifel","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/221/2213746/2213746_27957621.mp4","Folge 40")
   add_stream("Mit Schiff und Rad durch Holland","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/221/2219594/2219594_28066119.mp4","Folge 41")
   add_stream("Flevoland - Wasserwelt an Markermeer und IJsselmeer","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/221/2219001/2219001_28055599.mp4","Folge 42")
   add_stream("Wangerooge - Insel ohne Eile","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/222/2221940/2221940_28127723.mp4","Folge 43")
   add_stream("Slowenien - Alpen mit Meerblick","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/222/2228770/2228770_28313021.mp4","Folge 44")
   add_stream("Die Ruhe weg - Mit dem Bulli durchs Paderborner Land","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/223/2235064/2235064_28470152.mp4","Folge 45")
   add_stream("Urlaub rund um Münster","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/223/2234235/2234235_28450047.mp4","Folge 46")
   add_stream("Mit dem Trethausboot über die Ruhr - von Mülheim nach Essen","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/223/2239023/2239023_28574470.mp4","Folge 47")
   add_stream("Die WDR-Radtour - Vom Drachenfels zum Baldeneysee","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/223/2239028/2239028_28574552.mp4","Folge 48")
   add_stream("Urlaub rund um Köln","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/223/2238980/2238980_28573360.mp4","Folge 49")
   add_stream("Ruhr-Radeln von Winterberg bis Hagen","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/224/2249559/2249559_28849241.mp4","Folge 50")
   add_stream("Ruhr-Radeln von Hagen nach Duisburg","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/224/2249712/2249712_28853401.mp4","Folge 51")
   add_stream("Mit dem Fahrrad vom Brocken nach Bielefeld","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/225/2254193/2254193_28982738.mp4","Folge 52")
   add_stream("Teutoburger Wald - In acht Etappen auf dem Hermannsweg","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/225/2254230/2254230_28984008.mp4","Folge 53")
   add_stream("Vom Halterner Silbersee in die Hohe Mark","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/225/2250549/2250549_28874071.mp4","Folge 54")
   add_stream("Ibiza - ein Lebensgefühl","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/203/2034137/2034137_24815819.mp4","Folge 55")
   add_stream("Ruhr-Radeln von Winterberg bis Hagen","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/226/2260323/2260323_29139908.mp4","Folge 56")
   add_stream("Ostfriesland - vom Winde verwirrt?","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/226/2265612/2265612_29288189.mp4","Folge 57")
   add_stream("Rund um die Flensburger Förde","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/226/2265608/2265608_29288127.mp4","Folge 58")
   add_stream("Auf der Schlei bis an die Ostsee","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/226/2266349/2266349_29324268.mp4","Folge 60")
   add_stream("Gran Canaria und seine verborgenen Schätze","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/227/2271584/2271584_29455551.mp4","Folge 61")
   add_stream("Budapest, Berlin & Krakau","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/227/2270991/2270991_29442741.mp4","Folge 62")
   add_stream("Geheimnisvolles Siebenbürgen - Reise in eine andere Zeit","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/227/2271663/2271663_29461833.mp4","Folge 63")
   add_stream("Die dänische Ostsee - von Møn bis Kopenhagen","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/227/2275986/2275986_29570721.mp4","Folge 64")
   add_stream("Eine Reise durch die Uckermark und den Landkreis Barnim","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/227/2276002/2276002_29571581.mp4","Folge 65") 
   add_stream("Geheimnisvolles Siebenbürgen - Reise in eine andere Zeit","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/227/2278309/2278309_29645105.mp4","Folge 66")
   add_stream("Wunderschön! Der Darß - Deutschlands schöne Halbinsel","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/227/2279163/2279163_29662765.mp4","Folge 67")
   add_stream("Fehmarn - Inselglück für alle","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/226/2265666/2265666_29289809.mp4","Folge 68")
   add_stream("Durch das wilde Mecklenburg - Unterwegs mit Marco Schreyl","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/228/2280244/2280244_29690214.mp4","Folge 69")
   add_stream("Mit dem Hausboot zur Müritz","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/228/2281094/2281094_29714771.mp4","Folge 70") 
   add_stream("Der Spreewald - eine Reise durch verwunschene Wasserwelten","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/228/2281198/2281198_29717386.mp4","Folge 71")
   add_stream("2 für 300: Tamina auf Deutschlandtour","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/228/2281159/2281159_29716128.mp4","Folge 72")
   add_stream("Auf dem Segelboot in Kroatien","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/228/2281865/2281865_29735941.mp4","Folge 73")
   add_stream("Zypern auf eigene Faust","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/228/2285325/2285325_29837100.mp4","Folge 74")
   add_stream("Auf dem Canal du Midi zum Mittelmeer","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/228/2281927/2281927_29738002.mp4","Folge 75")
   add_stream("Gotland erleben - Schwedens Sonneninsel","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/228/2285393/2285393_29838412.mp4","Folge 76") 
   add_stream("Auf dem Highway 1 zwischen San Francisco und Los Angeles","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/228/2285955/2285955_29853438.mp4","Folge 77")
   add_stream("Taminas schoenste Winterreisen","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/229/2290467/2290467_29983612.mp4","Folge 78")
   add_stream("Südtirol im Herbst","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/229/2296312/2296312_30149523.mp4","Folge 79")
   add_stream("Genusswandern in der Schweiz","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/229/2296397/2296397_30151749.mp4","Folge 80")
   add_stream("Traumziel Seychellen","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/230/2301171/2301171_30293664.mp4","Folge 81") 
   add_stream("Sri Lanka - Perle im Indischen Ozean","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/230/2306339/2306339_30444954.mp4","Folge 82")
   add_stream("Winter in Nord und West","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/230/2306333/2306333_30479795.mp4","Folge 83")
   add_stream("Wintersonne in NRW","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/231/2311488/2311488_30599804.mp4","Folge 84")
   add_stream("Weihnachten in Ostwestfalen und in der Eifel","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/231/2313076/2313076_30669025.mp4","Folge 85")
   add_stream("Mit Eseln durch die Steiermark - Trekking mit Gefühl","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/231/2316514/2316514_30768678.mp4","Folge 86") 
   add_stream("Durch die wilde Nordeifel","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/231/2316531/2316531_30768058.mp4","Folge 87")
   add_stream("Kleinwalsertal - Öko-Urlaub in Österreich","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/231/2316544/2316544_30767742.mp4","Folge 88")
   add_stream("Weihnachten im Sauerland","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/232/2326634/2326634_31113542.mp4","Folge 89")
   add_stream("Das südliche Florida","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/229/2291300/2291300_30007851.mp4","Folge 90") 
   add_stream("Wunderschoene Urlaubsinseln","http://wdrmedien-a.akamaihd.net/medp/ondemand/weltweit/fsk0/180/1807784/1807784_21186604.mp4","Folge 91")
   add_stream("Wunderschoen! Norderney im Winter","https://mediandr-a.akamaihd.net/progressive/2021/0117/TV-20210117-0943-5700.hd.mp4","Folge 92")
end --- > end of playlist
 
--[[
function set_pmid(id)
  pmid=tonumber(id);
  return MENU_RETURN["EXIT_ALL"];
end
]]

function select_playitem()
--  local m=menu.new{name="", icon=kunst} 
  --local m=menu.new{name="Wunderschön", icon=""}
    local m = neutrino.ClistBoxWidget("Wunderschön", neutrino.PLUGINDIR .. "/Wunderschön/Wunderschön_hint.png", 2*neutrino.MENU_WIDTH) 
    local item
    m:setWidgetType(neutrino.WIDGET_TYPE_CLASSIC) -- if you want to change the design
    m:setMode(neutrino.MODE_LISTBOX)
    m:enableShrinkMenu()
    m:addWidget(neutrino.WIDGET_TYPE_STANDARD)
    m:addWidget(neutrino.WIDGET_TYPE_EXTENDED)
    m:addWidget(neutrino.WIDGET_TYPE_FRAME)
    m:enableWidgetChange()
    --m:enablePaintItemInfo() --sucks
    m:enablePaintDate()

    if pmid < 0 then
	pmid = 0
    end

    m:setSelected(pmid)

    for i,r in  ipairs(p) do
      item = neutrino.ClistBoxItem(r.title)
      item:setOption(r.from)
      item:setHelpText(r.from)
      item:setItemIcon(zdf)
      item:set2lines()
    --m:addItem{type="forwarder", action="set_pmid", id=i, icon="streaming", name=r.title, hint=r.from, hint_icon="hint_reload"}
	m:addItem(item)
  end 

  m:exec(null, "")
  pmid = 0
  pmid = m:getSelected() + 1

  local vPlay = neutrino.CMoviePlayerGui()

--[[
  movie = neutrino.MI_MOVIE_INFO()
  movie.epgTitle = func[p[pmid].access](p[pmid].title)
  movie.epgInfo1 = func[p[pmid].access](p[pmid].from)
  movie.file.Name = func[p[pmid].access](p[pmid].url)
]]

  local movie = func[p[pmid].access](p[pmid].url)

  if movie ~= nil then
    vPlay:addToPlaylist(movie)
    vPlay:exec(None, "")
  end

  if m:getExitPressed() ~= true then
     select_playitem()
  end
--[[ 
  repeat
    pmid=0
    m:exec()
    if pmid==0 then
      return
    end  

    local vPlay = nil 
    local url=func[p[pmid].access](p[pmid].url)
    if url~=nil then
      if  vPlay  ==  nil  then
	vPlay  =  video.new()
      end
 	vPlay:PlayFile("Wunderschön",url,p[pmid].title, p[pmid].from ); 
   end
  until false]]
end

--Main
function main()
	init()
	func={
	  [stream]=function (x) return x end,
	}
	fill_playlist()
	select_playitem()
	os.execute("rm /tmp/lua*");
end

main()


