#if defined(CONFIG_IDF_TARGET_ESP32S3)
#include <FS.h>
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "esp_task_wdt.h"
#include <ESPmDNS.h>
#include <DNSServer.h>
#include "SD_MMC.h"
#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "USB.h"
#include "USBMSC.h"


#define USETFT false  // use dongle lcd screen

#if USETFT
#include <TFT_eSPI.h>  // https://github.com/stooged/TFT_eSPI
TFT_eSPI tft = TFT_eSPI();  
long tftCnt = 0;
bool tftOn = true;
#endif


//-------------------DEFAULT SETTINGS------------------//

                       // use config.ini [ true / false ]
#define USECONFIG true // this will allow you to change these settings below via the webpage or the config.ini file.
                       // if you want to permanently use the values below then set this to false.

// create access point
boolean startAP = true;
String AP_SSID = "USB_WEB_AP";
String AP_PASS = "password";
IPAddress Server_IP(10, 0, 0, 1);
IPAddress Subnet_Mask(255, 255, 255, 0);

// connect to wifi
boolean connectWifi = false;
String WIFI_SSID = "Home_WIFI";
String WIFI_PASS = "password";
String WIFI_HOSTNAME = "usb.local";

//-----------------------------------------------------//

#include "Pages.h"
AsyncWebServer server(80);
DNSServer dnsServer;
USBMSC dev;
File upFile;
//USBCDC USBSerial;
#define MOUNT_POINT "/sdcard"
#define PDESC "USB-Dongle"
#define MDESC "T-D-S3"
sdmmc_card_t *card;
bool remountSD = false;
long unCnt = 0;


String split(String str, String from, String to)
{
  String tmpstr = str;
  tmpstr.toLowerCase();
  from.toLowerCase();
  to.toLowerCase();
  int pos1 = tmpstr.indexOf(from);
  int pos2 = tmpstr.indexOf(to, pos1 + from.length());
  String retval = str.substring(pos1 + from.length(), pos2);
  return retval;
}

bool instr(String str, String search)
{
  int result = str.indexOf(search);
  if (result == -1)
  {
    return false;
  }
  return true;
}

String formatBytes(size_t bytes)
{
  if (bytes < 1024)
  {
    return String(bytes) + " B";
  }
  else if (bytes < (1024 * 1024))
  {
    return String(bytes / 1024.0) + " KB";
  }
  else if (bytes < (1024 * 1024 * 1024))
  {
    return String(bytes / 1024.0 / 1024.0) + " MB";
  }
  else
  {
    return String(bytes / 1024.0 / 1024.0 / 1024.0) + " GB";
  }
}

String urlencode(String str)
{
  String encodedString = "";
  char c;
  char code0;
  char code1;
  char code2;
  for (int i = 0; i < str.length(); i++)
  {
    c = str.charAt(i);
    if (c == ' ')
    {
      encodedString += '+';
    }
    else if (isalnum(c))
    {
      encodedString += c;
    }
    else
    {
      code1 = (c & 0xf) + '0';
      if ((c & 0xf) > 9)
      {
        code1 = (c & 0xf) - 10 + 'A';
      }
      c = (c >> 4) & 0xf;
      code0 = c + '0';
      if (c > 9)
      {
        code0 = c - 10 + 'A';
      }
      code2 = '\0';
      encodedString += '%';
      encodedString += code0;
      encodedString += code1;
    }
    yield();
  }
  encodedString.replace("%2E", ".");
  return encodedString;
}


#if USECONFIG
void handleConfig(AsyncWebServerRequest *request)
{
  if (request->hasParam("ap_ssid", true) && request->hasParam("ap_pass", true) && request->hasParam("web_ip", true) && request->hasParam("subnet", true) && request->hasParam("wifi_ssid", true) && request->hasParam("wifi_pass", true) && request->hasParam("wifi_host", true))
  {
    AP_SSID = request->getParam("ap_ssid", true)->value();
    if (!request->getParam("ap_pass", true)->value().equals("********"))
    {
      AP_PASS = request->getParam("ap_pass", true)->value();
    }
    WIFI_SSID = request->getParam("wifi_ssid", true)->value();
    if (!request->getParam("wifi_pass", true)->value().equals("********"))
    {
      WIFI_PASS = request->getParam("wifi_pass", true)->value();
    }
    String tmpip = request->getParam("web_ip", true)->value();
    String tmpsubn = request->getParam("subnet", true)->value();
    String WIFI_HOSTNAME = request->getParam("wifi_host", true)->value();
    String tmpua = "false";
    String tmpcw = "false";
    if (request->hasParam("useap", true))
    {
      tmpua = "true";
    }
    if (request->hasParam("usewifi", true))
    {
      tmpcw = "true";
    }
    if (tmpua.equals("false") && tmpcw.equals("false"))
    {
      tmpua = "true";
    }
    File iniFile = SD_MMC.open("/config.ini", "w");
    if (iniFile)
    {
      iniFile.print("\r\nAP_SSID=" + AP_SSID + "\r\nAP_PASS=" + AP_PASS + "\r\nWEBSERVER_IP=" + tmpip + "\r\nSUBNET_MASK=" + tmpsubn + "\r\nWIFI_SSID=" + WIFI_SSID + "\r\nWIFI_PASS=" + WIFI_PASS + "\r\nWIFI_HOST=" + WIFI_HOSTNAME + "\r\nUSEAP=" + tmpua + "\r\nCONWIFI=" + tmpcw + "\r\n");
      iniFile.close();
    }
    String htmStr = "<!DOCTYPE html><html><head><meta http-equiv=\"refresh\" content=\"8; url=/info.html\"><style type=\"text/css\">#loader {z-index: 1;width: 50px;height: 50px;margin: 0 0 0 0;border: 6px solid #f3f3f3;border-radius: 50%;border-top: 6px solid #3498db;width: 50px;height: 50px;-webkit-animation: spin 2s linear infinite;animation: spin 2s linear infinite; } @-webkit-keyframes spin {0%{-webkit-transform: rotate(0deg);}100%{-webkit-transform: rotate(360deg);}}@keyframes spin{0%{ transform: rotate(0deg);}100%{transform: rotate(360deg);}}body {background-color: #1451AE; color: #ffffff; font-size: 20px; font-weight: bold; margin: 0 0 0 0.0; padding: 0.4em 0.4em 0.4em 0.6em;} #msgfmt {font-size: 16px; font-weight: normal;}#status {font-size: 16px; font-weight: normal;}</style></head><center><br><br><br><br><br><p id=\"status\"><div id='loader'></div><br>Config saved<br>Rebooting</p></center></html>";
    request->send(200, "text/html", htmStr);
    delay(1000);
    ESP.restart();
  }
  else
  {
    request->redirect("/config.html");
  }
}

void handleConfigHtml(AsyncWebServerRequest *request)
{
  String tmpUa = "";
  String tmpCw = "";
  if (startAP)
  {
    tmpUa = "checked";
  }
  if (connectWifi)
  {
    tmpCw = "checked";
  }
  String htmStr = "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><title>Config Editor</title><style type=\"text/css\">body {background-color: #303030; color: #ffffff; font-size: 14px;font-weight: bold;margin: 0 0 0 0.0;padding: 0.4em 0.4em 0.4em 0.6em;}input[type=\"submit\"]:hover {background: #ffffff;color: #1d1d1d;}input[type=\"submit\"]:active{outline-color: #1d1d1d;color: #1d1d1d;background: #ffffff; }table {font-family: arial, sans-serif;border-collapse: collapse;}td {border: 1px solid #dddddd;text-align: left;padding: 8px;}th {border: 1px solid #dddddd; background-color:gray;text-align: center;padding: 8px;}</style></head><body><form action=\"/config.html\" method=\"post\"><center><table><tr><th colspan=\"2\"><center>Access Point</center></th></tr><tr><td>AP SSID:</td><td><input name=\"ap_ssid\" value=\"" + AP_SSID + "\"></td></tr><tr><td>AP PASSWORD:</td><td><input name=\"ap_pass\" value=\"********\"></td></tr><tr><td>AP IP:</td><td><input name=\"web_ip\" value=\"" + Server_IP.toString() + "\"></td></tr><tr><td>SUBNET MASK:</td><td><input name=\"subnet\" value=\"" + Subnet_Mask.toString() + "\"></td></tr><tr><td>START AP:</td><td><input type=\"checkbox\" name=\"useap\" " + tmpUa + "></td></tr><tr><th colspan=\"2\"><center>Wifi Connection</center></th></tr><tr><td>WIFI SSID:</td><td><input name=\"wifi_ssid\" value=\"" + WIFI_SSID + "\"></td></tr><tr><td>WIFI PASSWORD:</td><td><input name=\"wifi_pass\" value=\"********\"></td></tr><tr><td>WIFI HOSTNAME:</td><td><input name=\"wifi_host\" value=\"" + WIFI_HOSTNAME + "\"></td></tr><tr><td>CONNECT WIFI:</td><td><input type=\"checkbox\" name=\"usewifi\" " + tmpCw + "></td></tr></table><br><input id=\"savecfg\" type=\"submit\" value=\"Save Config\"></center></form></body></html>";
  request->send(200, "text/html", htmStr);
}

void writeConfig()
{
  File iniFile = SD_MMC.open("/config.ini", "w");
  if (iniFile)
  {
    String tmpua = "false";
    String tmpcw = "false";
    if (startAP)
    {
      tmpua = "true";
    }
    if (connectWifi)
    {
      tmpcw = "true";
    }
    iniFile.print("\r\nAP_SSID=" + AP_SSID + "\r\nAP_PASS=" + AP_PASS + "\r\nWEBSERVER_IP=" + Server_IP.toString() + "\r\nSUBNET_MASK=" + Subnet_Mask.toString() + "\r\nUSEAP=" + tmpua + "\r\n\r\n\r\nWIFI_SSID=" + WIFI_SSID + "\r\nWIFI_PASS=" + WIFI_PASS + "\r\nWIFI_HOST=" + WIFI_HOSTNAME + "\r\nCONWIFI=" + tmpcw + "\r\n");
    iniFile.close();
  }
}
#endif


void handleReboot(AsyncWebServerRequest *request)
{
  //USBSerial.print("Rebooting ESP");
  AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", rebooting_html);
  request->send(response);
  delay(1000);
  ESP.restart();
}


void handleDelete(AsyncWebServerRequest *request) {
  if (!request->hasParam("file", true)) {
    request->redirect("/fileman.html");
    return;
  }
  String path = request->getParam("file", true)->value();
  if (path.length() == 0) {
    request->redirect("/fileman.html");
    return;
  }
  if (SD_MMC.exists("/" + path) && path != "/" && !path.equals("config.ini") && !path.equals("wifi_info.txt")) {
    SD_MMC.remove("/" + path);
  }
  remount_usb();
  request->redirect("/fileman.html");
}


void handleFileMan(AsyncWebServerRequest *request) {
  File dir = SD_MMC.open("/");
  String output = "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><title>File Manager</title><link rel=\"stylesheet\" href=\"style.css\"><style>body{overflow-y:auto;} th{border: 1px solid #dddddd; background-color:gray;padding: 8px;}</style><script>function statusDel(fname) {var answer = confirm(\"Are you sure you want to delete \" + fname + \" ?\");if (answer) {return true;} else { return false; }} </script></head><body><br><table id=filetable></table><script>var filelist = [";
  int fileCount = 0;
  while (dir) {
    File file = dir.openNextFile();
    if (!file) {
      dir.close();
      break;
    }
    String fname = String(file.name());
    if (fname.length() > 0 && !fname.equals("config.ini") && !fname.equals("wifi_info.txt") && !file.isDirectory()) {
      fileCount++;
      fname.replace("|", "%7C");
      fname.replace("\"", "%22");
      output += "\"" + fname + "|" + formatBytes(file.size()) + "\",";
    }
    file.close();
    esp_task_wdt_reset();
  }
  if (fileCount == 0) {
    output += "];</script><center>No files found<br>You can upload files using the <a href=\"/upload.html\" target=\"mframe\"><u>File Uploader</u></a> page.</center></p></body></html>";
  } else {
    output += "];var output = \"\";filelist.forEach(function(entry) {var splF = entry.split(\"|\"); output += \"<tr>\";output += \"<td><a href=\\\"\" +  splF[0] + \"\\\">\" + splF[0] + \"</a></td>\"; output += \"<td>\" + splF[1] + \"</td>\";output += \"<td><a href=\\\"/\" + splF[0] + \"\\\" download><center><button type=\\\"submit\\\">Download</button></center></a></td>\";output += \"<td><form action=\\\"/delete\\\" method=\\\"post\\\"><center><button type=\\\"submit\\\" name=\\\"file\\\" value=\\\"\" + splF[0] + \"\\\" onClick=\\\"return statusDel('\" + splF[0] + \"');\\\">Delete</button></center></form></td>\";output += \"</tr>\";}); document.getElementById(\"filetable\").innerHTML = \"<tr><th colspan='1'><center>File Name</center></th><th colspan='1'><center>File Size</center></th><th colspan='1'><center>Download</center></th><th colspan='1'><center>Delete</center></th></tr>\" + output;</script></body></html>";
  }
  request->send(200, "text/html", output);
}


void handleFileUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
  if (!index)
  {
    String path = request->url();
    if (path != "/upload.html")
    {
      request->send(500, "text/plain", "Internal Server Error");
      return;
    }
    if (!filename.startsWith("/"))
    {
      filename = "/" + filename;
    }
    //USBSerial.printf("Upload Start: %s\n", filename.c_str());
    upFile = SD_MMC.open(filename, "w");
  }
  if (upFile)
  {
    upFile.write(data, len);
  }
  if (final)
  {
    upFile.close();
    //USBSerial.printf("upload Success: %uB\n", index+len);
  }
}


void handleInfo(AsyncWebServerRequest *request)
{
  float flashFreq = (float)ESP.getFlashChipSpeed() / 1000.0 / 1000.0;
  FlashMode_t ideMode = ESP.getFlashChipMode();
  String mcuType = CONFIG_IDF_TARGET;
  mcuType.toUpperCase();
  String output = "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><title>System Information</title><link rel=\"stylesheet\" href=\"style.css\"></head>";
  output += "<hr>###### Software ######<br><br>";
  output += "SDK version: " + String(ESP.getSdkVersion()) + "<br><hr>";
  output += "###### Board ######<br><br>";
  output += "MCU: " + mcuType + "<br>";
  output += "Board: LilyGo T-Dongle-S3<br>";
  output += "Chip Id: " + String(ESP.getChipModel()) + "<br>";
  output += "CPU frequency: " + String(ESP.getCpuFreqMHz()) + "MHz<br>";
  output += "Cores: " + String(ESP.getChipCores()) + "<br><hr>";
  output += "###### Flash chip information ######<br><br>";
  output += "Flash chip Id: " + String(ESP.getFlashChipMode()) + "<br>";
  output += "Estimated Flash size: " + formatBytes(ESP.getFlashChipSize()) + "<br>";
  output += "Flash frequency: " + String(flashFreq) + " MHz<br>";
  output += "Flash write mode: " + String((ideMode == FM_QIO? "QIO" : ideMode == FM_QOUT? "QOUT": ideMode == FM_DIO? "DIO": ideMode == FM_DOUT? "DOUT": "UNKNOWN")) + "<br><hr>";
  output += "###### Storage information ######<br><br>";
  output += "Storage Device: SD<br>";
  output += "Total Size: " + formatBytes(SD_MMC.totalBytes()) + "<br>";
  output += "Used Space: " + formatBytes(SD_MMC.usedBytes()) + "<br>";
  output += "Free Space: " + formatBytes(SD_MMC.totalBytes() - SD_MMC.usedBytes()) + "<br><hr>";
  output += "###### Ram information ######<br><br>";
  output += "Ram size: " + formatBytes(ESP.getHeapSize()) + "<br>";
  output += "Free ram: " + formatBytes(ESP.getFreeHeap()) + "<br>";
  output += "Max alloc ram: " + formatBytes(ESP.getMaxAllocHeap()) + "<br><hr>";
  output += "###### Sketch information ######<br><br>";
  output += "Sketch hash: " + ESP.getSketchMD5() + "<br>";
  output += "Sketch size: " + formatBytes(ESP.getSketchSize()) + "<br>";
  output += "Free space available: " + formatBytes(ESP.getFreeSketchSpace() - ESP.getSketchSize()) + "<br><hr>";
  output += "</html>";
  request->send(200, "text/html", output);
}


void startAccessPoint()
{
  if (startAP)
  {
    WiFi.softAPConfig(Server_IP, Server_IP, Subnet_Mask);
    WiFi.softAP(AP_SSID.c_str(), AP_PASS.c_str());
    dnsServer.setTTL(30);
    dnsServer.setErrorReplyCode(DNSReplyCode::ServerFailure);
    dnsServer.start(53, "*", Server_IP);
    File ipFile =  SD_MMC.open("/wifi_info.txt", "a");
    if (!connectWifi)
    {  
      ipFile.println("Access Point SSID: " + AP_SSID + "\r\nWEB Hostname: " + WIFI_HOSTNAME + "\r\nWeb Server IP: " + Server_IP.toString() + "\r\n");
    }
    else
    {
      ipFile.println("Access Point SSID: " + AP_SSID + "\r\nWeb Server IP: " + Server_IP.toString() + "\r\n");
    }
    ipFile.close();
#if USETFT
    tft.print("AP: ");
    tft.println(AP_SSID);
    if (!connectWifi)
    {
      tft.print("Host: ");
      tft.println(WIFI_HOSTNAME);
    }
    tft.print("IP: ");
    tft.println(Server_IP.toString());
#endif
  }
}


void connectToWIFI()
{
  if (connectWifi && WIFI_SSID.length() > 0 && WIFI_PASS.length() > 0)
  {
    WiFi.setAutoConnect(true);
    WiFi.setAutoReconnect(true);
    WiFi.hostname(WIFI_HOSTNAME);
    WiFi.begin(WIFI_SSID.c_str(), WIFI_PASS.c_str());
    if (WiFi.waitForConnectResult() != WL_CONNECTED)
    {
      //USBSerial.println("Wifi failed to connect");
      File ipFile =  SD_MMC.open("/wifi_info.txt", "a");
      ipFile.println("Failed to connect to: " + WIFI_SSID + "\r\n");
      ipFile.close();
#if USETFT
      tft.setTextColor(TFT_RED, TFT_BLACK);          
      tft.println("Failed to connect to:");
      tft.setTextColor(TFT_WHITE, TFT_BLACK);  
      tft.println(WIFI_SSID);
#endif
    }
    else
    {
      IPAddress LAN_IP = WiFi.localIP();
      if (LAN_IP)
      {        
        String mdnsHost = WIFI_HOSTNAME;
        mdnsHost.replace(".local", "");
        MDNS.begin(mdnsHost.c_str());
        File ipFile =  SD_MMC.open("/wifi_info.txt", "a");
        ipFile.println("Web Hostname: " + WIFI_HOSTNAME + "\r\nWeb Server IP: " + LAN_IP.toString() + "\r\n");
        ipFile.close();
#if USETFT        
        tft.print("Host: ");
        tft.println(WIFI_HOSTNAME);
        tft.print("IP: ");
        tft.println(LAN_IP.toString()); 
#endif
      }
    }
  }
}


void setup()
{
  //USBSerial.begin(115200);
  //USBSerial.begin();
  
  pinMode(38, OUTPUT);
  digitalWrite(38, HIGH); 

#if USETFT   
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextFont(1); //16 col 5 row
  tft.setTextColor(TFT_SKYBLUE, TFT_BLACK);   
  tft.setCursor(0, 0, 2);
  digitalWrite(38, LOW);
#endif

  setup_SD();

  SD_MMC.setPins(12, 16, 14, 17, 21, 18);
  if (SD_MMC.begin())
  {
    if (SD_MMC.exists("/wifi_info.txt"))
    {
      SD_MMC.remove("/wifi_info.txt");
    }

#if USECONFIG
    if (SD_MMC.exists("/config.ini"))
    {
      File iniFile = SD_MMC.open("/config.ini", "r");
      if (iniFile)
      {
        String iniData;
        while (iniFile.available())
        {
          char chnk = iniFile.read();
          iniData += chnk;
        }
        iniFile.close();

        if (instr(iniData, "AP_SSID="))
        {
          AP_SSID = split(iniData, "AP_SSID=", "\r\n");
          AP_SSID.trim();
        }

        if (instr(iniData, "AP_PASS="))
        {
          AP_PASS = split(iniData, "AP_PASS=", "\r\n");
          AP_PASS.trim();
        }

        if (instr(iniData, "WEBSERVER_IP="))
        {
          String strwIp = split(iniData, "WEBSERVER_IP=", "\r\n");
          strwIp.trim();
          Server_IP.fromString(strwIp);
        }

        if (instr(iniData, "SUBNET_MASK="))
        {
          String strsIp = split(iniData, "SUBNET_MASK=", "\r\n");
          strsIp.trim();
          Subnet_Mask.fromString(strsIp);
        }

        if (instr(iniData, "WIFI_SSID="))
        {
          WIFI_SSID = split(iniData, "WIFI_SSID=", "\r\n");
          WIFI_SSID.trim();
        }

        if (instr(iniData, "WIFI_PASS="))
        {
          WIFI_PASS = split(iniData, "WIFI_PASS=", "\r\n");
          WIFI_PASS.trim();
        }

        if (instr(iniData, "WIFI_HOST="))
        {
          WIFI_HOSTNAME = split(iniData, "WIFI_HOST=", "\r\n");
          WIFI_HOSTNAME.trim();
        }

        if (instr(iniData, "USEAP="))
        {
          String strua = split(iniData, "USEAP=", "\r\n");
          strua.trim();
          if (strua.equals("true"))
          {
            startAP = true;
          }
          else
          {
            startAP = false;
          }
        }

        if (instr(iniData, "CONWIFI="))
        {
          String strcw = split(iniData, "CONWIFI=", "\r\n");
          strcw.trim();
          if (strcw.equals("true"))
          {
            connectWifi = true;
          }
          else
          {
            connectWifi = false;
          }
        }
      }
    }
    else
    {
      writeConfig();
    }
#endif

   mount_usb();
    
  }
  else
  {
    //USBSerial.println("SD_MMC failed to mount");
  }

  startAccessPoint();
  connectToWIFI();

  server.on("/connecttest.txt", HTTP_GET, [](AsyncWebServerRequest *request){ request->send(200, "text/plain", "Microsoft Connect Test");});
  server.on("/config.ini", HTTP_ANY, [](AsyncWebServerRequest *request){ request->send(404);});
  server.on("/wifi_info.txt", HTTP_ANY, [](AsyncWebServerRequest *request){ request->send(404);});
#if USECONFIG
  server.on("/config.html", HTTP_GET, [](AsyncWebServerRequest *request){ handleConfigHtml(request);});
  server.on("/config.html", HTTP_POST, [](AsyncWebServerRequest *request){ handleConfig(request);});
#endif
  server.on("/fileman.html", HTTP_GET, [](AsyncWebServerRequest *request) { handleFileMan(request);});
  server.on("/delete", HTTP_POST, [](AsyncWebServerRequest *request){ handleDelete(request);});
  server.on("/reboot.html", HTTP_POST, [](AsyncWebServerRequest *request){ handleReboot(request);});
  server.on("/info.html", HTTP_GET, [](AsyncWebServerRequest *request){ handleInfo(request);});
  
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/css", style_css);
    request->send(response);
  });

  server.on("/upload.html", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", upload_html);
    request->send(response);
  });

  server.on("/upload.html", HTTP_POST, [](AsyncWebServerRequest *request){
      remount_usb();
      request->redirect("/fileman.html");
    },
    handleFileUpload);

  server.on("/index.html", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", index_html);
    request->send(response);});

  server.on("/reboot.html", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", reboot_html);
    request->send(response);});

  server.serveStatic("/", SD_MMC, "/").setDefaultFile("index.html");

  server.onNotFound([](AsyncWebServerRequest *request){
    //USBSerial.println(request->url());
    String path = request->url();
    if (path.endsWith("index.html") || path.endsWith("index.htm") || path.endsWith("/")) {
      AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", index_html);
      request->send(response);
      return;
    }
    request->send(404);});
    
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  server.begin();
  //USBSerial.println("Server started");
}


void mount_usb(){
  dev.vendorID(MDESC);
  dev.productID(PDESC);
  dev.productRevision("1.0");
  dev.onRead(onRead);
  dev.onWrite(onWrite);
  dev.mediaPresent(true);
  dev.begin(card->csd.capacity, card->csd.sector_size);
  USB.productName(PDESC);
  USB.manufacturerName(MDESC);
  USB.begin();
}


void remount_usb(){
  if (!remountSD)
  {
    dev.end();
  }
  unCnt = millis();
  remountSD = true;
}


void setup_SD(void)
{
  esp_err_t ret;
  const char mount_point[] = MOUNT_POINT;
  esp_vfs_fat_sdmmc_mount_config_t mount_config = {.format_if_mount_failed = false, .max_files = 5, .allocation_unit_size = 16 * 1024};
  sdmmc_host_t host = {
      .flags = SDMMC_HOST_FLAG_4BIT | SDMMC_HOST_FLAG_DDR,
      .slot = SDMMC_HOST_SLOT_1,
      .max_freq_khz = SDMMC_FREQ_DEFAULT,
      .io_voltage = 3.3f,
      .init = &sdmmc_host_init,
      .set_bus_width = &sdmmc_host_set_bus_width,
      .get_bus_width = &sdmmc_host_get_slot_width,
      .set_bus_ddr_mode = &sdmmc_host_set_bus_ddr_mode,
      .set_card_clk = &sdmmc_host_set_card_clk,
      .do_transaction = &sdmmc_host_do_transaction,
      .deinit = &sdmmc_host_deinit,
      .io_int_enable = sdmmc_host_io_int_enable,
      .io_int_wait = sdmmc_host_io_int_wait,
      .command_timeout_ms = 0,
  };
  sdmmc_slot_config_t slot_config = {
      .clk = (gpio_num_t)12,
      .cmd = (gpio_num_t)16,
      .d0 = (gpio_num_t)14,
      .d1 = (gpio_num_t)17,
      .d2 = (gpio_num_t)21,
      .d3 = (gpio_num_t)18,
      .cd = SDMMC_SLOT_NO_CD,
      .wp = SDMMC_SLOT_NO_WP,
      .width = 4,
      .flags = SDMMC_SLOT_FLAG_INTERNAL_PULLUP,
  };
  gpio_set_pull_mode((gpio_num_t)16, GPIO_PULLUP_ONLY);
  gpio_set_pull_mode((gpio_num_t)14, GPIO_PULLUP_ONLY);
  gpio_set_pull_mode((gpio_num_t)17, GPIO_PULLUP_ONLY);
  gpio_set_pull_mode((gpio_num_t)21, GPIO_PULLUP_ONLY);
  gpio_set_pull_mode((gpio_num_t)18, GPIO_PULLUP_ONLY);
  ret = esp_vfs_fat_sdmmc_mount(mount_point, &host, &slot_config, &mount_config, &card);
}

static int32_t onWrite(uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t bufsize)
{
  uint32_t count = (bufsize / card->csd.sector_size);
  sdmmc_write_sectors(card, buffer + offset, lba, count);
  return bufsize;
}

static int32_t onRead(uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize)
{
  uint32_t count = (bufsize / card->csd.sector_size);
  sdmmc_read_sectors(card, buffer + offset, lba, count);
  return bufsize;
}


void loop()
{
  if (millis() >= (unCnt + 1500) && remountSD)
  {
    unCnt = 0;
    remountSD = false;
    mount_usb();
  }
#if USETFT    
  if (millis() >= (tftCnt + 60000) && tftOn){ 
    tftCnt = 0;
    tftOn = false;
    digitalWrite(38, HIGH);
    return;
  }
  if (digitalRead(0) == LOW){
    if (tftCnt == 0){
       tftCnt = millis();
       digitalWrite(38, LOW);
       tftOn = true;
    }
  }
#endif
  dnsServer.processNextRequest();
}

#else
#error "Selected board not supported"
#endif
