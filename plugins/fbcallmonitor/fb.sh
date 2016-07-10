#!/bin/sh

set -x

. $PLUGINDIR/fbcallmonitor/fb.conf

# do not change anything below here ----
Version=v0.5			#Zur Versionsanzeige im telnet
FBBOOK=$PLUGINDIR/fbcallmonitor/fb.csv	#internal phonebook uses comma sep. values
TMPFILE=/tmp/fritzbox.tmp	#stores page from wget command

#
#definition of the reverse search online (GOYELLOW.DE)
#
SEARCHPRV="GoYellow"
SEARCHURL="http://www.goyellow.de/inverssuche/?TEL="
SEARCHPDETAIL="Detailinformationen"
#

inverssuche () {    
    NUMMER=`echo $1 | sed -e "s@\ @@g" -e "s@+49@0@"`
    echo "$NUMMER"
    #try to get caller-id of $NUMMER via internet
    echo "Searching for $NUMMER via $SEARCHPRV"
    wget "$SEARCHURL$NUMMER"  -O $TMPFILE
    #extract caller-id from online result
    NAME=`grep $SEARCHPDETAIL $TMPFILE | head -1 | sed -e "s/^.*zu\(.*\)in.*$/\1/"`
    STREET=`grep "street-address" $TMPFILE| head -1 | sed 's/<[^>]*>/ /g'`
    CITY=`grep "postal-code" $TMPFILE| head -1 | sed 's/<[^>]*>/ /g'`
    ADDRESSE=`echo $STREET~n$CITY | sed -e 's/ //g'`
    if [ "$NAME" = "" ]; then
     NAME="Unbekannt"
     ADDRESSE=""
    fi
     export NAME
     export ADDRESSE    
};

internesuche ()  {
    NUMMER=`echo $1 | sed -e "s@\ @@g" -e "s@+49@0@"`
    echo "$NUMMER"
    if [ -f $FBBOOK ]; then
    LINE=`grep $NUMMER $FBBOOK`
    echo "$LINE"
    NAME=`echo $LINE | awk -F"," {'print $2'}`
    ADDRESSE=`echo $LINE | awk -F"," {'print $3 "~n " $4'}`
    fi
    export NAME
    export ADDRESSE   
};

#additional paths to files we use (will be created automatically if not present)
NULL=/dev/null                              #'NULL'
SCRIPT=$0                                   #full path to this script
export SCRIPT
#
#
#check if FritzBox! is up and callmonitor port is reachable. if not exit.
#
cat /dev/null | nc $FRITZBOXIP $TELDPORT
if [ $? != 0 ]; then
 echo "Fritzbox (" $FRITZBOXIP ") ist nicht auf Port" $TELDPORT "erreichbar!"
 echo "Skript wird beendet!"
 FRITZUP=down
 wget -q -O /dev/null "http://$ip/control/message?popup=Fritzbox%20nicht%20erreichbar!"
 exit 1
else
 FRITZUP=up
fi

#
#if script is called without any parameters show help text
#
if [ "$1" = "" ]; then
 echo
 echo "Usage: $SCRIPT START STOP"
 echo
 echo "START               Start Script and deamonize"
 echo "STOP                kill all processes we created"
 echo
fi

#
case $1 in
  START|start)
  echo
  echo "------------------------------------"
  echo "Fritzbox Callmonitor" $Version "gestartet"
  echo "---Folgende Optionen sind gesetzt---"
  echo "Debugmodus: " $debug
  echo "Ueberwachte Rufnummern: " $Ziel_1 " und " $Ziel_2 "und " $Ziel_3
  echo "Alle Rufnummern ueberwachen: " $Alle
  echo "Eingehende Anrufe ueberwachen: " $monRing
  echo "IP Adresse der Kathrein: " $ip
  echo "------------------------------------"
  
  if [ $debug = 0 ]; then
     wget -q -O /dev/null "http://$ip/control/message?popup=FritzBox!%20Callmonitor%20gestartet"
  fi
  #very tricky line to get data from FritzBox! using netcat and divide it using
  #awk. Only make changes to this line if you completely understand it!
  #Make sure the next line end with a trailing '&' or your box will hang!
    tail -f < /dev/null|nc $FRITZBOXIP $TELDPORT|awk -F";" '{if ($4==""){$4="Unbekannt"} system (ENVIRON ["SCRIPT"] " "$2" "$1" "$3" "$4" "$5" "$6" "$7 )}' &
     ;;
  RING)
    echo $2" "$3" Ankommend: von "$5" auf "$6
    internesuche $5
    b=" Anruf von "$5"~n~n "$NAME"~n "$ADDRESSE"~n f�r "$6    
    if [ "$NAME" = "" ]; then      
      if [ $invers = 1 ]; then
      inverssuche $5
      b=" Anruf von "$5"~n~n "$NAME"~n "$ADDRESSE"~n f�r "$6
      else
      b=" Anruf von "$5"~n f�r "$6
      fi
    fi
    #prepare text
    b1=`echo $b               | sed -e '{s/'$Ziel_1'/'$Ziel_1_name'/g;s/'$Ziel_2'/'$Ziel_2_name'/g;s/'$Ziel_3'/'$Ziel_3_name'/g;}'` 
    b2=`echo $b1              | sed -e '{s/ /%20/g;s/�/%C3%84/g;s/�/%C3%96/g;s/�/%C3%9C/g;s/�/%C3%A4/g;s/�/%C3%B6/g;s/�/%C3%BC/g;s/�/ss/g;s/&/%26/g;s/~n/%0a/g;s/#/%20/g;}'`
    #Eingehende Anrufe anzeigen?
    if [ $monRing = 1 ]; then
      #Welche Rufnummern sollen �berwacht werden?
      if [ $6 = $Ziel_1 -o $6 = $Ziel_2 -o $6 = $Ziel_3 -o $Alle = 1 ] ; then
            if [ $debug = 0 ]; then 
              if [ $muteRing = 1 ]; then #wenn Option gesetzt ist dann wird der Ton abgeschaltet 
                wget -q -O /dev/null "http://$loginname:$passwort@127.0.0.1/control/volume?mute" #Befehl f�r Tonsperre
              fi
              if [ $popup = 1 ]; then
              wget -q -O /dev/null "http://$ip/control/message?popup=$b2"  >> $NULL
              else
              wget -q -O /dev/null "http://$ip/control/message?nmsg=$b2"  >> $NULL
              fi
            else
              echo $b
              echo $b1
              echo $b2
            fi  
      fi
    fi   
    
    ;;
  STOP|stop)
     #kill all processes we spawned
     #hopefully only a single tail task is running so we kill it :)
     pid=`ps | grep "tail -f"| grep -v grep | awk '{print $1}'`
     if [ "X${pid}" != "X" ] ; then
        kill -9 ${pid}
     fi
     if [ $debug = 0 ]; then
        wget -q -O /dev/null "http://$ip/control/message?popup=FritzBox!%20Callmonitor%20gestopt"
     fi
     pid=`ps |grep "Unbekannt" | grep -v grep | awk '{print $1}'`
     if [ "X${pid}" != "X" ] ; then
        kill -9 ${pid}
     fi
     pid=`ps |grep ".*nc.*$TELDPORT" | grep -v grep | awk '{print $1}'`
     if [ "X${pid}" != "X" ] ; then
        kill -9 ${pid}
     fi
     pid=`ps |grep $SCRIPT | grep -v grep | awk '{print $1}'`
     if [ "X${pid}" != "X" ] ; then
        kill -9 ${pid}
     fi
     ;;
  DISCONNECT)
    if [ $debug = 0 ]; then
      if [ $muteRing = 1 ]; then #wenn Option gesetzt ist dann wird der Ton abgeschaltet 
          wget -q -O /dev/null "http://$loginname:$passwort@$ip/control/volume?unmute"  >> $NULL #Befehl f�r Tonsperre
      fi
      if [ $monDisconnect = 1 ]; then
        h=$(($5/3600))                        #get hours from   $5
        m=$(($5%3600/60))                     #get minutes from $5
        s=$((($5%3600)%60))                   #get seconds from $5
        if [ $h -lt 10 ]; then h="0"$h; fi    #add leading 0 if needed
        if [ $m -lt 10 ]; then m="0"$m; fi    #add leading 0 if needed
        if [ $s -lt 10 ]; then s="0"$s; fi    #add leading 0 if needed
        b="~n Aufgelegt nach "$5" Sekunden Dauer ~n (hh:mm:ss): "$h":"$m":"$s"~n"
        b1=`echo $b | sed -e '{s/ /%20/g;s/�/%C3%84/g;s/�/%C3%96/g;s/�/%C3%9C/g;s/�/%C3%A4/g;s/�/%C3%B6/g;s/�/%C3%BC/g;s/�/ss/g;s/&/%26/g;s/~n/%0a/g;s/#/%20/g;}'`
        wget -q -O /dev/null "http://$ip/control/message?popup=$b1"
      fi
      if [ -f $TMPFILE ];     then  rm $TMPFILE; fi
    fi
    
    ;;
esac          