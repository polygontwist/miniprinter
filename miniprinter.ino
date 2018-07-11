/*
 ESP8266, DOUT, 115200, 80MHz
 ESP12-E: 4M (3M SPIFFS)

Zeiten abhängig von Motorspannung (hier ca.3V3)
 01 Motorpuls(MP); ~0.64ms hi|1.6ms low
 02 /
 03 Nadel 4
 04 Nadel 3
 05 Nadel 2
 06 Nadel 1
 07 Nadel GND
 08 Motor +
 09 Motor -
 10 Reedrelay, pro Umdrehung; ca.0.254s hi|78.5ms low (MP:97|29)
 11 /

Nadelabstand: ~12mm, (126 MP ->0.0952mm)
Verfahrung:   ~35.5mm

wenn linke Kante: Papier+1 ~0.5mm

*/

//MAC:  a0:20:a6:1:59:78


const char* progversion  = "WLan-Printer V0.1";//ota fs ntp ti getpin 
#define ARDUINO_HOSTNAME  "wlanprinter"

#define pin_P1 0    //Motorpuls
#define pin_P2 0    //Reedrelaypuls
#define pin_N1 0
#define pin_N2 0
#define pin_N3 0
#define pin_N4 0
#define pin_Motor 0

#define pin_Button 0          //Button Flash  
#define pin_buttoninvert true  

#define pin_led 2             //blue    
#define pin_ledinvert true              



#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <time.h>

#include <JeVe_EasyOTA.h>  // https://github.com/jeroenvermeulen/JeVe_EasyOTA/blob/master/JeVe_EasyOTA.h
#include "FS.h"

#include "myNTP.h"
myNTP oNtp;

#include "data.h" //index.htm

//#define WIFI_SSID         ""
//#define WIFI_PASSWORD     ""
#include "wifisetup.h"    //fest eingebunden

#define actionheader "HTTP/1.1 303 OK\r\nLocation:/index.htm\r\nCache-Control: no-cache\r\n\r\n"

EasyOTA OTA;
ESP8266WebServer server(80);
File fsUploadFile;                      //Haelt den aktuellen Upload

bool isAPmode=false;
int anzahlVerbindungsversuche=0;
#define check_wlanasclient 15000      //alle 15 Sekunden* gucken ob noch verbunden, wenn nicht neuer Versuch
                                      //zwischen 15 und 30 Sekunden
unsigned long check_wlanasclient_previousMillis=0;
#define anzahlVersuche 10             //nach 10 Versuchen im AP-Modus bleiben
#define keinAPModus true              //true=immer wieder versuchen ins WLAN zu kommen

unsigned long butt_zeitchecker= 120;//ms min presstime
unsigned long butt_previousMillis=0;
unsigned long buttpresstime=0;

uint8_t MAC_array[6];
char MAC_char[18];
String macadresse="";

String printbefehl="";
bool isgettingprint=false;

//---------------printer------------------------
//int rasterlength=0;
//int rasterheight=0;
#define spalten (97+29) //motorpulse=15*8+6   max.15 Zeichen breit
#define zeilen 8            //8xreedrelaypulse oder 4x wegen&hinher ?
#define rasterength (spalten*zeilen) //of byte ->bit0=N1 bit1=N2 bit2=N3 bit3=N4 
              // ->1008byte (126byte pro pixelzeile)
byte rasterarray[rasterength];       //fester Puffer

#define zeichenprozeile 15  //



//---------------helper--------------------------
//format bytes
String formatBytes(size_t bytes) {
  if (bytes < 1024) {
    return String(bytes) + "B";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + "KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + "MB";
  } else {
    return String(bytes / 1024.0 / 1024.0 / 1024.0) + "GB";
  }
}

//----------------basics------------------------
void setLED(bool an){
  if(pin_ledinvert)an=!an;
  digitalWrite(pin_led, an);
}
void toogleLED(){
  digitalWrite(pin_led, !digitalRead(pin_led));
}

bool getLED(){
  if(pin_ledinvert){
     return digitalRead(pin_led)==LOW;
    }
    else{
     return digitalRead(pin_led)==HIGH;
    }
}

bool getButton(){
  if(pin_buttoninvert){
     return digitalRead(pin_Button)==LOW;
    }
    else{
     return digitalRead(pin_Button)==HIGH;
    }
}

void setNadeln(bool bN1,bool bN2,bool bN3,bool bN4){
  digitalWrite(pin_N1, bN1);
  digitalWrite(pin_N2, bN2);
  digitalWrite(pin_N3, bN3);
  digitalWrite(pin_N4, bN4);
}

void setMotor(bool an){
  digitalWrite(pin_Motor, an);
}

bool getP1(){
  return digitalRead(pin_P1)==HIGH;
}
bool getP2(){
  return digitalRead(pin_P2)==HIGH;
}

//---------------------------------------------
void connectWLAN(){
   setLED(true);
  
   anzahlVerbindungsversuche++;
   OTA.setup(WIFI_SSID, WIFI_PASSWORD, ARDUINO_HOSTNAME);//connect to WLAN
   isAPmode=!(WiFi.status() == WL_CONNECTED);


   Serial.print("mode: ");
   if(isAPmode)
      Serial.println("AP");
      else
      Serial.println("client");

  macadresse="";
  WiFi.macAddress(MAC_array);
  for (int i = 0; i < sizeof(MAC_array); ++i) {
    if(i>0) macadresse+=":";
    macadresse+= String(MAC_array[i], HEX);
  }
  Serial.print("MAC: ");
  Serial.println(macadresse);

  Serial.print("Connected to ");
  Serial.println(WIFI_SSID);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP()); 

  if(isAPmode){
      setLED(true);
      delay(500);
      setLED(false);
      delay(250);
      setLED(true);
      delay(500);
      setLED(false);
      delay(250);
      setLED(true);
      delay(500);
      setLED(false);
      delay(250);
    }
    else{
      anzahlVerbindungsversuche=0;//erfolgreich verbunden, Zaehler auf 0 setzen
      setLED(false);
   }
}
//----------------------------------------------

void setup() {
  //serial
  Serial.begin(115200);
  Serial.println("");
  Serial.println("");
  Serial.println(progversion);
  
  //SPIFFS
  SPIFFS.begin();

  //Pins initialisieren 
  pinMode(pin_led, OUTPUT);
  pinMode(pin_Button, INPUT); 
  
  //pinMode(pin_P1, INPUT);
  //pinMode(pin_P2, INPUT);
  //pinMode(pin_N1, OUTPUT);
  //pinMode(pin_N2, OUTPUT);
  //pinMode(pin_N3, OUTPUT);
  //pinMode(pin_N4, OUTPUT);
  //pinMode(pin_Motor, OUTPUT);

 //Power ON:
  setLED(true);  //LED on
  setMotor(false);
  setNadeln(false,false,false,false);

 //rasterinit
//  rasterarray = (int*)malloc((rasterength)*(sizeof(byte)));
  for (int i=0;i<rasterength;i++) {
    rasterarray[i] = 0;
  }


  //OTA
  OTA.onMessage([](char *message, int line) {
    toogleLED();
    //digitalWrite(pin_led, !digitalRead(pin_led));//Staus LED blinken
    Serial.println(message);
  });

  connectWLAN();
  
  server.on("/action", handleAction);//daten&befehle
  
  server.on("/",handleIndex);
  server.on("/index.htm", handleIndex);
  server.on("/index.html", handleIndex);
  
  server.on("/data.json", handleData);//aktueller Status+Dateiliste
  
  server.on("/upload", HTTP_POST, []() {
    server.send(200, "text/plain", "");
  }, handleFileUpload);             //Dateiupload
  
  server.onNotFound(handleNotFound);//Datei oder 404

  server.begin();
  Serial.print("HTTP server started HOSTNAME:");
  Serial.println(ARDUINO_HOSTNAME);
  
  Serial.println("ready.");

  //NTP start
  oNtp.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  server.handleClient();
  OTA.loop();
  oNtp.update();
  isAPmode=!(WiFi.status() == WL_CONNECTED);

  unsigned long currentMillis = millis();//milliseconds since the Arduino board began running
  
  //Button
  if( currentMillis - butt_previousMillis > butt_zeitchecker){//alle 120ms
     butt_previousMillis=currentMillis;
    
     if (!getButton()) {//up
        if( buttpresstime>0){
           setLED(false);
           //digitalWrite(pin_led, true);//LED off
           Serial.println(buttpresstime);
           if(buttpresstime>6000){//ca. 6 sec
              Serial.println("Restart ESP");
              //restart
              setLED(true);
              delay(500);
              setLED(false);
              delay(500);
              setLED(true);
              delay(500);
              setLED(false);
              delay(500);
              setLED(true);
              delay(500);
              setLED(false);
              delay(500);
              ESP.restart();
           }
           else
           {
            //toogle Relais
            //toogleRelais();
            Serial.println("toogle nothing");
           }
           buttpresstime=0;
        }
      }
      else
      {//down
        buttpresstime+=butt_zeitchecker;//Zeit merken +=120ms
        if(buttpresstime>6000)
              toogleLED();
            else
              setLED(true);//LED an
       }
  }

  

  //WLAN-ceck
 unsigned long cwl=random(check_wlanasclient, check_wlanasclient+check_wlanasclient);//x..x+15sec sonst zu viele Anfragen am AP
 if(currentMillis - check_wlanasclient_previousMillis > cwl){
      //zeit abgelaufen
      check_wlanasclient_previousMillis = currentMillis;
       if(isAPmode){//apmode
        //neuer Verbindengsaufbauversuch
         if(anzahlVerbindungsversuche<anzahlVersuche || keinAPModus){//nur x-mal, dann im AP-Mode bleiben
               connectWLAN();
         }
      }
  }

  //printer
  if(!isgettingprint){
    //check
    if(printbefehl!=""){drucken();}
  }
}

//------------Data IO--------------------

void handleData(){// data.json
  String message = "{\r\n";
  String aktionen = "";

  //uebergabeparameter?
  for (uint8_t i = 0; i < server.args(); i++) {
    if (server.argName(i) == "settimekorr") {
       oNtp.setTimeDiff(server.arg(i).toInt());
       aktionen +="set_timekorr ";
    }
    if (server.argName(i) == "led") {
       if (server.arg(i) == "on" ){
            handleAktion(3,1);
             aktionen +="LED_ON ";
        }
       if (server.arg(i) == "off"){
            handleAktion(4,1);
            aktionen +="LED_OFF ";
       }
    }
   /* if (server.argName(i) == "relais") {
       if (server.arg(i) == "on" ){
             handleAktion(1,1);
             aktionen +="Relais_ON ";
        }
       if (server.arg(i) == "off"){
             handleAktion(2,1);
             aktionen +="Relais_OFF ";
       }
    }*/
  }
  
  message +="\"hostname\":\""+String(ARDUINO_HOSTNAME)+"\",\r\n";
  message +="\"aktionen\":\""+aktionen+"\",\r\n";
  message +="\"progversion\":\""+String(progversion)+"\",\r\n";
  message +="\"cpu_freq\":\""+String(ESP.getCpuFreqMHz())+"\",\r\n";
  message +="\"chip_id\":\""+String(ESP.getChipId())+"\",\r\n";

  message +="\"isAPmode\":\"";
  if(isAPmode)
     message +="true";
    else
     message +="false";
  message +="\",\r\n";

  
  byte ntp_stunde   =oNtp.getstunde();
  byte ntp_minute   =oNtp.getminute();
  byte ntp_secunde  =oNtp.getsecunde();
 

//  ntp_stunde
  message +="\"lokalzeit\":\"";
  if(ntp_stunde<10)message+="0";
  message+=String(ntp_stunde)+":";
  if(ntp_minute<10)message+="0";
  message+= String(ntp_minute)+":";
  if(ntp_secunde<10)message+="0";
  message+=String(ntp_secunde);
  message +="\",\r\n";
  
  message +="\"datum\":{\r\n";
  message +=" \"tag\":"+String(oNtp.getwochentag())+",\r\n";
  message +=" \"year\":"+String(oNtp.getyear())+",\r\n";
  message +=" \"month\":"+String(oNtp.getmonth())+",\r\n";
  message +=" \"day\":"+String(oNtp.getday())+",\r\n";
  message +=" \"timekorr\":"+String(oNtp.getUTCtimediff())+",\r\n";
  if(oNtp.isSummertime())
    message +=" \"summertime\":true\r\n";
  else
    message +=" \"summertime\":false\r\n";
  message +="},\r\n";

//led/relais-status
  
  message +="\"portstatus\":{\r\n";
  
 /* message +="\"relais\":";
  if(getRelay())
         message +="true";
         else
         message +="false";
  message +=",\r\n";*/
  
  message +="\"button\":";
  if(getButton())
         message +="true";
         else
         message +="false";
  message +=",\r\n";
  
  message +="\"led\":";
 
  if(getLED())
         message +="true";
         else
         message +="false";
  message +="\r\n";
   
  message +="},\r\n";
  
 
  message +="\"macadresse\":\""+macadresse+"\",\r\n";

  FSInfo fs_info;
  if (SPIFFS.info(fs_info)) {
      message +="\"fstotalBytes\":"+String(fs_info.totalBytes)+",\r\n";
      message +="\"fsusedBytes\":"+String(fs_info.usedBytes)+",\r\n";

      message +="\"fsused\":\"";
      message +=float(int(100.0/fs_info.totalBytes*fs_info.usedBytes*100.0)/100.0);
      message +="%\",\r\n";
   }
  //files
  message +="\"files\":[\r\n";
  String fileName;
  Dir dir = SPIFFS.openDir("/");
  uint8_t counter=0;
  while (dir.next()) {
      fileName = dir.fileName(); 
      if(counter>0)  message +=",\r\n";
      message +=" {";
      message +="\"fileName\":\""+fileName+"\", ";
      message +="\"fileSize\":"+String(dir.fileSize());
      message +="}";
     counter++;
  };
  message +="\r\n]\r\n";
//--
 
  message +="\r\n}";
  
  server.send(200, "text/html", message );
  Serial.println("send data.json");
}


void handleIndex() {//Rueckgabe HTML
  //$h1gtag $info
  int pos1 = 0;
  int pos2 = 0;
  String s;
  String tmp;

  String message = "";

  while (indexHTM.indexOf("\r\n", pos2) > 0) {
    pos1 = pos2;
    pos2 = indexHTM.indexOf("\r\n", pos2) + 2;
    s = indexHTM.substring(pos1, pos2);

    //Tags gegen Daten ersetzen
    if (s.indexOf("$h1gtag") != -1) {
      s.replace("$h1gtag", progversion);//Ueberscherschrift=Prog-Version
    }

    //Liste der Dateien
    if(s.indexOf("$filelist") != -1){        
        tmp="<table class=\"files\">\n";
        String fileName;
        Dir dir = SPIFFS.openDir("/");
        while (dir.next()) {
            fileName = dir.fileName(); 
            Serial.print("getfilelist: ");
            Serial.println(fileName);
            tmp+="<tr>\n";
            tmp+="\t<td><a target=\"_blank\" href =\"" + fileName + "\"" ;
            tmp+= " >" + fileName.substring(1) + "</a></td>\n\t<td class=\"size\">" + formatBytes(dir.fileSize())+"</td>\n\t<td class=\"action\">";
            tmp+="<a href =\"" + fileName + "?delete=" + fileName + "\" class=\"fl_del\"> l&ouml;schen </a>\n";
            tmp+="\t</td>\n</tr>\n";
        };

        FSInfo fs_info;
        tmp += "<tr><td colspan=\"3\">";
        if (SPIFFS.info(fs_info)) {
          tmp += formatBytes(fs_info.usedBytes).c_str(); //502
          tmp += " von ";
          tmp += formatBytes(fs_info.totalBytes).c_str(); //2949250 (2.8MB)   formatBytes(fileSize).c_str()
          tmp += " (";
          tmp += float(int(100.0/fs_info.totalBytes*fs_info.usedBytes*100.0)/100.0);
          tmp += "%)";
          /*tmp += "<br>\nblockSize:";
          tmp += fs_info.blockSize; //8192
          tmp += "<br>\npageSize:";
          tmp += fs_info.pageSize; //256
          tmp += "<br>\nmaxOpenFiles:";
          tmp += fs_info.maxOpenFiles; //5
          tmp += "<br>\nmaxPathLength:";
          tmp += fs_info.maxPathLength; //32*/
        }
        tmp+="</td></tr></table>\n";
        s.replace("$filelist", tmp);
    }

    
    message += s;
  }

  server.send(200, "text/html", message );
}


void handleAction() {//Rueckgabe JSON
  /*
      /action?setport=LEDON    LED einschalten
      /action?setport=LEDOFF   LED ausschalten
      /action?getpin=0        aktuellen Status von Pin IO0
  */
  String message = "{\n";
  message += "\"Arguments\":[\n";

  uint8_t AktionBefehl = 0;
  uint8_t keyOK = 0;
  uint8_t aktionresult = 0;

  for (uint8_t i = 0; i < server.args(); i++) {
    if (i > 0) message += ",\n";
    message += "  {\"" + server.argName(i) + "\" : \"" + server.arg(i) + "\"";

    if (server.argName(i) == "setport") {
      //if (server.arg(i) == "ON")   AktionBefehl = 1;
      //if (server.arg(i) == "OFF")  AktionBefehl = 2;
      if (server.arg(i) == "LEDON")  AktionBefehl = 3;
      if (server.arg(i) == "LEDOFF")  AktionBefehl = 4;
    }

    if (server.argName(i) == "print") {
        message += " ,\"printermsg\": \"";
        if(printbefehl==""){
            isgettingprint=true;
            printbefehl=server.arg(i);
            isgettingprint=false;
            message += "ok";
          }
          else
          {
             message += "notfree";
          }
          message += "\"";
    }

  
    if (server.argName(i) == "getpin"){
       message += " ,\"val\": \"";
       if(digitalRead( server.arg(i).toInt())==HIGH)
              message += "true";
              else
              message += "false";
       message += "\"";
    }

    //TODO:
    //setSSID
    //setPassword

    
    message += "}";
  }
  message += "\n]";

  if(AktionBefehl>0){
      aktionresult = handleAktion(AktionBefehl, 1);
      message += ",\n\"befehl\":\"";
      if (aktionresult > 0)
        message += "OK";
      else
        message += "ERR";
      message += "\"";
  }
  
  message += "\n}";
  server.send(200, "text/plain", message );

}


String getContentType(String filename) {              // ContentType fuer den Browser
  if (filename.endsWith(".htm")) return "text/html";
  //else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  /*else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/x-pdf";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz")) return "application/x-gzip";*/
  return "text/plain";
}

void handleFileUpload() {          // Dateien ins SPIFFS schreiben
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    //Serial.print("handleFileUpload Name: ");
    if (filename.length() > 30) {
      int x = filename.length() - 30;
      filename = filename.substring(x, 30 + x);
    }
    filename = server.urlDecode(filename);
    filename = "/" + filename;
    
    fsUploadFile = SPIFFS.open(filename, "w");
    //if(!fsUploadFile) Serial.println("!! file open failed !!");
    
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (fsUploadFile){
        //Serial.print("handleFileUpload Data: "); Serial.println(upload.currentSize);
        fsUploadFile.write(upload.buf, upload.currentSize);
      }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile){
      fsUploadFile.close();
      //Serial.println("close");
    }
    yield();
    //Serial.print("handleFileUpload Size: "); Serial.println(upload.totalSize);
    server.sendContent(actionheader);//Seite neu laden
  }
}

bool handleFileRead(String path) {//Datei loeschen oder uebertragen
  if(server.hasArg("delete")) {
    //Serial.print("delete: ");
    //Serial.print(server.arg("delete"));
    SPIFFS.remove(server.arg("delete"));  //hier wir geloescht
       /*Serial.println(" OK");
        else
        Serial.println(" ERR");*/
    server.sendContent(actionheader);//Seite neu laden
    return true;
  }
  path = server.urlDecode(path);
  //Serial.print("GET ");
  //Serial.print(path);
  if (SPIFFS.exists(path)) {
    //Serial.println(" send");
    //   Serial.println("handleFileRead: " + path);
    File file = SPIFFS.open(path, "r");
    size_t sent = server.streamFile(file, getContentType(path));
    file.close();
     return true;
  }
  //Serial.println(" 404");
  return false;
}


void handleNotFound() {
 //--check Dateien im SPIFFS--
 if(!handleFileRead(server.uri())){ 
    //--404 als JSON--
    String message = "{\n \"error\":\"File Not Found\", \n\n";
    message += " \"URI\": \"";
    message += server.uri();
    message += "\",\n \"Method\":\"";
    message += (server.method() == HTTP_GET) ? "GET" : "POST";
    message += "\",\n";
    message += " \"Arguments\":[\n";
    for (uint8_t i = 0; i < server.args(); i++) {
      if (i > 0) message += ",\n";
      message += "  {\"" + server.argName(i) + "\" : \"" + server.arg(i) + "\"}";
    }
    message += "\n ]\n}";  
    server.send(404, "text/plain", message);  
  }
}

//----------------------IO-----------------------------
uint8_t handleAktion(uint8_t befehl, uint8_t key) {
  uint8_t re = 0;
  Serial.print("handleAktion:");
  Serial.print(befehl);
  Serial.print(",");
  Serial.println(key);/**/
  if (key == 1) {
    if (befehl == 1) {//ON
      //setRelais(true);
      re = 1;
    }
    if (befehl == 2) {//OFF
     // setRelais(false);
      re = 2;
    }
    if (befehl == 3) {//LEDON
      setLED(true);
      //digitalWrite(pin_led, false);
      re = 3;
    }
    if (befehl == 4) {//LEDOFF
      setLED(false);
      //digitalWrite(pin_led,true );
      re = 4;
    }
  }
  return re;
}

//------------Drucken--------------------
int printpos=0;
int printstatus=0;

 

void drucken(){
   int i;
   char c;//char byte
   
   //printbefehl
   if(printstatus==0){
      //printbefehl -> rastern

      //rasterinit, alles leeren
      for(i=0;i<rasterength;i++) {
        rasterarray[i] = 0;
      }

      //text-zeichen rastern
      int anzahlzeichen=printbefehl.length();
      Serial.print("Anzahl:");
      Serial.println(anzahlzeichen);
     // Serial.println(printbefehl.getBytes());
      
      for(i=0;i<anzahlzeichen;i++) {//=Xpos
        c=printbefehl[i];//umlaute 2 byte!, Ä:[195],132 oder 3 byte €:[226],130,172
        Serial.print(c,DEC);
        if(c==195 || c==194){
          //zeichentab16 
          i++;
          c=printbefehl[i];
          Serial.print(",");
          Serial.print(c,DEC);
        }
        else
        if(c==226){
          //zeichentab24
          Serial.print(",");
          i++;
          c=printbefehl[i];
          Serial.print(c,DEC);
          Serial.print(",");
           i++;
          c=printbefehl[i];
          Serial.print(c,DEC);
         
        }
        else{
          //zeichentab8
          Serial.print(" ");
          Serial.print(char(c));
         }
        
         Serial.println("");
      }
 Serial.println(printbefehl);

       Serial.println("*");
      printstatus=1;
      return;
   } 
 
   if(printstatus==1){
   //raster abfahren (rzeile{loop|rzeile|...)
      
      printstatus=2;
      return;
   }

   if(printstatus==2){
    //wenn raster-printing zuende, rintbefehl freimachen
     printbefehl="";
     printstatus=0;
   }
}



