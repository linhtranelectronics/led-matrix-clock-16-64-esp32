/*
  Name:		Sketch1.ino
  Created:	6/12/2023 9:46:51 AM
  Author:	linhb
*/


#include <DMDESP.h>
#include <fonts/EMSans6x8.h>
#include <fonts/Mono5x7.h>
#include <fonts/ElektronMart6x16.h>
#include <fonts/EMKotak5x7.h>
#include "Solar2luna.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "EEPROM.h"
#include <time.h>


ESP8266WebServer server(80);

const char*     ssid = "configWifiForTime";
const char*     passphrase = "123456789";

String          st;
String          content;
int             statusCode;

int hour, minute, second, day, month, year;
const int oneWireBus = 2;
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);
//SETUP DMD
#define DISPLAYS_WIDE 2 // Kolom Panel
#define DISPLAYS_HIGH 1 // Baris Panel
DMDESP matrix(DISPLAYS_WIDE, DISPLAYS_HIGH);  // Jumlah Panel P10 yang digunakan (KOLOM,BARIS)
converSolar2luna lunaDate;
float temperatureC;
bool apMode = false;
//----------------------------------------------------------------------
// SETUP

void setup() {
  Serial.begin(115200);
  // DMDESP Setup
  matrix.start();
  matrix.setBrightness(100);
  sensors.begin();
  EEPROM.begin(512);
  delay(10);
  Serial.println("Startup");
  // read eeprom for ssid, pass and blynk
  Serial.println("Reading EEPROM ssid");
  String esid;
  for (int i = 0; i < 32; ++i) {
    esid += char(EEPROM.read(i));
  }
  Serial.print("SSID: ");
  Serial.println(esid.c_str());
  esid.trim();

  Serial.println("Reading EEPROM pass");
  String epass = "";
  for (int i = 32; i < 96; ++i) {
    epass += char(EEPROM.read(i));
  }
  Serial.print("PASS: ");
  Serial.println(epass.c_str());
  epass.trim();


  if (esid.length() > 1) {
    WiFi.begin(esid.c_str(), epass.c_str());
    if (testWifi()) {
      matrix.clear();
      launchWeb(0);
      //WiFi.disconnect();
      EEPROM.end();
      int timezone = 7 * 3600;
      int dst = 0;
      configTime(timezone, dst, "pool.ntp.org", "time.nist.gov");
      return;
    }
  }
  setupAP();
  EEPROM.end();

}




//----------------------------------------------------------------------
// LOOP

void loop() {
	while (apMode) {
		server.handleClient();
		matrix.loop();
		matrix.setFont(EMSans6x8);
		matrix.drawText(7, 4, "AP MODE");
	}
  server.handleClient();
  matrix.loop();
  int32_t now = millis();
  static uint32_t lastUpdate;
    if (now - lastUpdate >=  1000) {
    updateTime();
    lastUpdate = now;
    
  }
	showClock(hour, minute, second, 8, 0);
	showDay(day, month, year, 0, 8);
	if (hour > 18 || hour < 7) {
		matrix.setBrightness(10);
	}
	else {
		matrix.setBrightness(100);
	}
}

void showClock(uint8_t h, uint8_t m, uint8_t s, uint8_t x, int8_t y) {
  static uint32_t lastUpdate;
  uint32_t now = millis();
  static bool blink;
  matrix.setFont(Mono5x7);
  if (lastUpdate + 500 <= now) {	
    lastUpdate = now;
    blink = !blink;
    String dot = blink ? ":" : " ";
    String timeShow = addZero(h) + dot + addZero(m) + dot + addZero(s);
    
    matrix.drawText(x, y, timeShow );
  }
}
void getLunaDate(uint8_t dd, uint8_t mm, uint8_t yy)
{
	static uint8_t lastDayConver;
	if (lastDayConver != day) {
		lunaDate.Solar2Lunar(dd, mm, yy);
		lastDayConver = day;
	}
}
void showDay(uint8_t dd, uint8_t mm, uint8_t yy, uint8_t x, uint8_t y) {
	getLunaDate(dd, mm, yy);
	uint8_t lunar_dd = lunaDate.get_lunar_dd();
	uint8_t lunar_mm = lunaDate.get_lunar_mm();
	matrix.setFont(Mono5x7);
	matrix.drawText(x, y,addZero(dd) + "/" +addZero(mm) +"-"+ addZero(lunar_dd) + "/" + addZero(lunar_mm));
	
}
String addZero(uint16_t num) 	{
	String t="";
	if (num<10) {
		t = "0";
	}
	return t+ String(num);
}
bool testWifi(void) {
  int c = 0;
  uint32_t now = millis();
  uint32_t last;
  Serial.println("Xin vui long doi ket noi WIFI");
  while (c < 200) {
    now = millis();
    if (WiFi.status() == WL_CONNECTED) {
      return true;
    }
    if (now - last >=  100)
    {
      last = now;
      c++;
      }
  matrix.loop(); 
  matrix.setFont(Mono5x7);
  matrix.drawText(1, 4, "Connecting." ); 
  }
  Serial.println("");
  Serial.println("Thoi gian ket noi cham, Mo AP");
  apMode = true;
  matrix.clear();
  return false;
}

void launchWeb(int webtype) {
  Serial.println("");
  Serial.println("WiFi ket noi");
  Serial.print("Dia chi IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("SoftAP IP: ");
  Serial.println(WiFi.softAPIP());
  createWebServer(webtype);
  // Start the server
  server.begin();
  Serial.println("May chu bat dau");
}

void setupAP(void) {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  int n = WiFi.scanNetworks();
  Serial.println("Tim hoan tat");
  if (n == 0) {
    Serial.println("khong tim thay wifi");
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      delay(10);
    }
  }
  Serial.println("");
  st = "<ol>";
  for (int i = 0; i < n; ++i) {
    // Print SSID and RSSI for each network found
    st += "<li>";
    st += WiFi.SSID(i);
    st += " (";
    st += WiFi.RSSI(i);
    st += ")";
    st += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*";
    st += "</li>";
  }
  st += "</ol>";
  delay(100);
  Serial.println("softap");
  Serial.println(ssid);
  Serial.println(passphrase);
  WiFi.softAP(ssid, passphrase, 6);

  launchWeb(1);
  Serial.println("over");
}

void createWebServer(int webtype) {
  if (webtype == 1) {
    server.on("/", []() {
      IPAddress ip = WiFi.softAPIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
      content = "<!DOCTYPE HTML>\r\n<html><h2>linhtran_electronics 0335644677</h2>";
      //content += ipStr;
      //content += "<form method='get' action='setting'><table width='100%' border='1'><tr><td width=\"30%\"><label>Wifi</label></td><td width=\"70%\><input name='ssid' length=32 width='500'></td></tr><tr><td><label>Password</label></td><td><input name='pass' length=64 width='500'></td></tr><tr><td><label>Blynk</label></td><td><input name='blynk' length=32 width='500'></td></tr><tr><td></td><td><input type='submit'></tr></tr></table></form>";
      content += "<form method=\"get\" action=\"setting\">";
      content += "<div>Wifi</div>";
      content += "<div><input name=\"ssid\" size=\"40\"></div>";
      content += "<div>Mat Khau</div>";
      content += "<div><input name=\"pass\" size=\"40\"></div>";
      content += "<div><input type='submit'></div>";

      content += "<p>";
      content += st;
      content += "</p>";
      content += "</html>";
      server.send(200, "text/html", content);
    });
    server.on("/setting", []() {
      String qsid = server.arg("ssid");
      String qpass = server.arg("pass");
      String qblynk = server.arg("blynk");
      if (qsid.length() > 0 && qpass.length() > 0) {
        EEPROM.begin(512);
        Serial.println("clearing eeprom");
        for (int i = 0; i < 128; ++i) {
          EEPROM.write(i, 0);
        }
        EEPROM.commit();
        Serial.println(qsid);
        Serial.println("");
        Serial.println(qpass);
        Serial.println("");


        Serial.println("writing eeprom ssid:");
        for (int i = 0; i < qsid.length(); ++i) {
          EEPROM.write(i, qsid[i]);
          Serial.print("Wrote: ");
          Serial.println(qsid[i]);
        }

        Serial.println("writing eeprom pass:");
        for (int i = 0; i < qpass.length(); ++i) {
          EEPROM.write(32 + i, qpass[i]);
          Serial.print("Wrote: ");
          Serial.println(qpass[i]);
        }

        Serial.println("writing eeprom blynk:");
        for (int i = 0; i < qblynk.length(); ++i) {
          EEPROM.write(96 + i, qblynk[i]);
          Serial.print("Wrote: ");
          Serial.println(qblynk[i]);
        }
        EEPROM.commit();
        EEPROM.end();
        content = "{\"Success\":\"luu hoan tat. khoi dong lai thiet bi! LINHTRAN_ELECTRONICS xin cam on.\"}";
        statusCode = 200;
      } else {
        content = "{\"Error\":\"404 not found\"}";
        statusCode = 404;
        Serial.println("Sending 404");
      }
      server.send(statusCode, "application/json", content);
    });
  } else if (webtype == 0) {
    server.on("/", []() {
      IPAddress ip = WiFi.localIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
      server.send(200, "application/json", "{\"IP\":\"" + ipStr + "\"}");
    });
    server.on("/cleareeprom", []() {
      content = "<!DOCTYPE HTML>\r\n<html>";
      content += "<h2>XSwitch</h2><p>Clearing the EEPROM</p></html>";
      server.send(200, "text/html", content);
      Serial.println("clearing eeprom");
      for (int i = 0; i < 128; ++i) {
        EEPROM.write(i, 0);
      }
      EEPROM.commit();
    });
  }
}
void updateTime() {
  time_t now = time(nullptr);
  struct tm* p_tm = localtime(&now);
  hour = p_tm->tm_hour;
  minute = p_tm->tm_min;
  second = p_tm->tm_sec;
  day = p_tm->tm_mday;
  month = p_tm->tm_mon + 1;
  year = p_tm->tm_year + 1900;
  year = year - 2000;
}
void connecting(bool  s) {
  String text = "Connecting";
  for(int i = 0;i<= s; i++)
  {
    text = text + ".";
    }
  matrix.setFont(Mono5x7);
  matrix.drawText(0, 0, text );
  Serial.println("connecting.");
}
