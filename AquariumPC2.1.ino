
//#define BLYNK_PRINT Serial

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <NeoPixelAnimator.h>
#include <TimeAlarms.h>
#include <HTTP_Method.h>
#include <ArduinoOTA.h>
#include "bitmaps.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <BlynkSimpleEsp32.h>
#include <Time.h>
#include <SPI.h>
#include <NeoPixelBrightnessBus.h>
#include <OneWire.h>
#include <DallasTemperature.h>


BlynkTimer Timer;

/*********** NeoPixel Einstellungen ***********/

#define PIN_STRIPE1			13
#define NUMLEDS				166

NeoPixelBrightnessBus<NeoRgbwFeature, NeoEsp32Rmt0800KbpsMethod> strip1(NUMLEDS, PIN_STRIPE1);

/************ TFT Einstellungen ***************/

TFT_eSPI tft = TFT_eSPI();
#define MAX_GEN_COUNT 500

/************* One Wire (Tempfühler) **********/

#define ONE_WIRE_BUS			26				// Anschlusspin für OneWire
#define TEMPERATURE_PRECISION	9				// 0.25 Grad Genauigkeit
OneWire oneWire(ONE_WIRE_BUS);					// One Wire Setup
DallasTemperature Tempfueh(&oneWire);			// Pass our oneWire reference to Dallas Temperature.


/**********************************************/


/********** Blynk und WiFi Einstellungen ******/


// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "06a15068bcdb4ae89620f5fd2e67c672";
const char* host = "aquarium-webupdate";

/****** BETA Token *****************************/
//char auth[] = "b93c1e342a6c4217b466ec684e61679b";
//const char* host = "aquarium-webupdate-beta";

char ssid[] = "Andre+Janina-EXT";
char pass[] = "sommer12";

bool Connected2Blynk = false;

char serverblynk[] = "blynk-cloud.com";
unsigned int port = 8442;

/*************** WEB Server für OTA **************/

//const char* host = "aquarium-webupdate";

WebServer server(80);
const char* serverIndex = "<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
"<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
"<input type='file' name='update'>"
"<input type='submit' value='Update'>"
"</form>"
"<div id='prg'>progress: 0%</div>"
"<script>"
"$('form').submit(function(e){"
"e.preventDefault();"
"var form = $('#upload_form')[0];"
"var data = new FormData(form);"
" $.ajax({"
"url: '/update',"
"type: 'POST',"
"data: data,"
"contentType: false,"
"processData:false,"
"xhr: function() {"
"var xhr = new window.XMLHttpRequest();"
"xhr.upload.addEventListener('progress', function(evt) {"
"if (evt.lengthComputable) {"
"var per = evt.loaded / evt.total;"
"$('#prg').html('progress: ' + Math.round(per*100) + '%');"
"}"
"}, false);"
"return xhr;"
"},"
"success:function(d, s) {"
"console.log('success!')"
"},"
"error: function (a, b, c) {"
"}"
"});"
"});"
"</script>";

/******* Variablen *******************************/

uint8_t TFTRotation;

uint8_t wifi_retry = 0;

uint8_t SoAuStd = 10;
uint8_t SoAuMin = 00;
uint8_t SoAuZei;
uint8_t SoUnStd = 20;
uint8_t SoUnMin = 00;
uint8_t SoUnZei;
uint8_t SoNaStd = 22;
uint8_t SoNaMin = 00;

uint8_t SoMiAnStd = 13;
uint8_t SoMiAnMin = 00;
uint8_t SoMiAusStd = 15;
uint8_t SoMiAusMin = 00;

uint8_t FutterStd;
uint8_t FutterMin;

uint8_t maxHell = 250;
uint8_t minHell;
uint8_t mittagHell = 5;
uint8_t aktHell;

float_t SollTemp = 26.0;
float_t IstTemp;
float_t LuefTemp = 30.0;
float_t Hysterese = 0.50;

uint8_t CO2AnStd = 11;
uint8_t CO2AnMin = 00;
uint8_t CO2AusStd = 18;
uint8_t CO2AusMin = 00;

uint16_t Zeit;

uint8_t AblaufX = 0;
uint8_t AblaufY = 159;
uint8_t AblaufI = 0;
uint8_t AblaufBlynk;

uint8_t BacklightPin = 22;
uint16_t BacklightFrequenz = 500;
uint8_t BacklightKanal = 2;
uint8_t BacklightBit = 8;
uint8_t BacklightWert = 100;

uint8_t FutterPin = 14;
uint16_t Futterfreq = 250;
uint8_t FutterKanal = 0;
uint8_t FutterRes = 8;
uint16_t Futtergesch = 0;
uint16_t Futterdauer = 1000;

uint8_t PowerledPin = 16;
uint16_t Powerledfreq = 2000;
uint8_t PowerledKanal = 1;
uint8_t PowerledBit = 8;
uint8_t Powerledwert;
uint8_t Powerledmax = 250;

uint8_t LEDRot;
uint8_t LEDGruen;
uint8_t LEDBlau;
uint8_t LEDWeiss;

unsigned long previousMillis = 0;

/**************** Pin Belegung *******************/

#define heizung				12      //Pin Relais Heizung
#define luefter				25      //Pin Lüfter
#define co2					27		//Pin Relais CO2 Abschaltung

/**************** NeoPixel Init ******************/

// Sonnenaufgang Color Array
//				{ R, G , B, W }
int SonAu1[4] = { 30,0,0,0 };
int SonAu2[4] = { 150,5,0,0 };
int SonAu3[4] = { 157,13,0,0 };
int SonAu4[4] = { 163,21,1,0 };
int SonAu5[4] = { 186,67,1,0 };
int SonAu6[4] = { 240,180,30,100 };
int SonAu7[4] = { 200,150,255,150 };

// Sonnenuntergang Color Array
//				{ R, G, B, W }
int SonUn1[4] = { 250,200,100,40 };
int SonUn2[4] = { 240,255,30,0 };
int SonUn3[4] = { 186,68,2,0 };
int SonUn4[4] = { 150,20,0,0 };
int SonUn5[4] = { 50, 14,1, 0 };
int SonUn6[4] = { 0, 0, 6, 0 };
int SonUn7[4] = { 0, 0, 4, 0 };

//Nachtlicht AUS Color Array
//					 { R, G, B, W }
int Nachtlicht1[4] = { 0, 0, 0, 0 };

// Set initial color
uint8_t redVal = 0;
uint8_t grnVal = 0;
uint8_t bluVal = 0;
uint8_t whiteVal = 0;

uint16_t DurchWait;
uint16_t crossFadeWait;

uint8_t prevR = redVal;
uint8_t prevG = grnVal;
uint8_t prevB = bluVal;
uint8_t prevW = whiteVal;
uint16_t LEDStep = 0;
uint8_t Durchlauf = 1;
uint8_t SonneIndex = 0;

uint16_t StepWert = 1020;

/***** NTP Server abrufen für Local Time ********/

static const char ntpServerName[] = "de.pool.ntp.org";
const int timeZone = 1;

const uint32_t syncIntervalMax = 300; // could be the same value like the syncInterval in time.cpp | 300
const uint32_t syncIntervalAfterFail = 60; // if sync with NTP fails, retry earlier | 60
const uint32_t ntpWaitMax = 900; // maximum time in ms to wait for an answer of NTP Server, most used value 1500 I prefere below one second: 900
uint32_t ntpWaitActual = ntpWaitMax; // optimized/reduced wait time, start with maximum.

WiFiUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets

const int NTP_PACKET_SIZE = 48;		// NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime();
void digitalClockDisplay();
void sendNTPpacket(IPAddress& address);

/***********************************************/

/******** BLYNK FUNKTIONEN  ********************/

WidgetLCD lcd(V127);
WidgetLED ledluefter(V22);
WidgetLED ledheizung(V23);
WidgetLED ledco2(V24);
WidgetLED led(V25);

/******* LED Timer Sonne Auf/Untergang *******/

BLYNK_WRITE(V0) {
	TimeInputParam t(param);

	// Process start time

	if (t.hasStartTime())
	{
		SoAuStd = t.getStartHour();
		SoAuMin = t.getStartMinute();
	}

	if (t.hasStopTime())
	{
		SoUnStd = t.getStopHour();
		SoUnMin = t.getStopMinute();
	}

	SunTimer();
}

/*********** LED Timer Mittagssonne ************/

BLYNK_WRITE(V6) {
	TimeInputParam t(param);

	if (t.hasStartTime())
	{
		SoMiAnStd = t.getStartHour();
		SoMiAnMin = t.getStartMinute();

	}

	if (t.hasStopTime())
	{
		SoMiAusStd = t.getStopHour();
		SoMiAusMin = t.getStopMinute();

	}
}

/************ LED Timer Nachtlicht *****************/

BLYNK_WRITE(V11) {

	TimeInputParam t(param);

	if (t.hasStartTime())
	{
		SoNaStd = t.getStartHour();
		SoNaMin = t.getStartMinute();
	}
}

/**************** CO2 Timer ******************/

BLYNK_WRITE(V4) {
	TimeInputParam t(param);

	// Process start time

	if (t.hasStartTime())
	{
		CO2AnStd = t.getStartHour();
		CO2AnMin = t.getStartMinute();
	}

	if (t.hasStopTime())
	{
		CO2AusStd = t.getStopHour();
		CO2AusMin = t.getStopMinute();
	}

	CO2Timer();
}

/*************** Soll Temperatur ***********/

BLYNK_WRITE(V2) {

	Blynk.virtualWrite(V1, param.asFloat());
	SollTemp = param.asFloat();
}

/******** Luefter **************************/

BLYNK_WRITE(V20) {

	Blynk.virtualWrite(V20, param.asFloat());
	LuefTemp = param.asFloat();
}

/************* Durchlauf Zeit *************/

BLYNK_WRITE(V5) {

	Blynk.virtualWrite(V5, param.asFloat());
	DurchWait = param.asFloat();
}

/******* Futterautomat ********************/

BLYNK_WRITE(V7) {

	TimeInputParam t(param);

	if (t.hasStartTime())
	{
		FutterStd = t.getStartHour();
		FutterMin = t.getStartMinute();
	}

	FutterTimer();
}

BLYNK_WRITE(V34) {

	Blynk.virtualWrite(V34, param.asFloat());
	Futterdauer = param.asFloat();
}

BLYNK_WRITE(V35) {

	Blynk.virtualWrite(V35, param.asFloat());
	Futtergesch = param.asFloat();
}

/********** Temp Hysterese *****************/

BLYNK_WRITE(V8) {

	Blynk.virtualWrite(V8, param.asFloat());
	Hysterese = param.asFloat();
}

/****** Maximale Helligkeit *****************/

BLYNK_WRITE(V9) {

	Blynk.virtualWrite(V9, param.asFloat());
	maxHell = param.asFloat();
}

/******* Mittags Helligkeit *****************/

BLYNK_WRITE(V25) {

	Blynk.virtualWrite(V25, param.asFloat());
	mittagHell = param.asFloat();
}

/******* PowerLED Max Hellighkeit *********/

BLYNK_WRITE(V36) {

	Blynk.virtualWrite(V36, param.asFloat());
	Powerledmax = param.asFloat();
}

/******* Aktuelle Helligkeit *****************/

BLYNK_WRITE(V28) {

	Blynk.virtualWrite(V28, param.asFloat());
	aktHell = param.asFloat();
}

/******** TFT Helligkeit ********************/

BLYNK_WRITE(V10) {

	Blynk.virtualWrite(V10, param.asFloat());
	BacklightWert = param.asFloat();

	ledcWrite(BacklightKanal, BacklightWert);
}

/****** TFT Rotation ************************/

BLYNK_WRITE(V19) {

	int i = param.asInt();
	if (i == 1) {
		TFTRotation = 3;
		delay(250);
	}
	else
		TFTRotation = 1;

	TFT_Layout();
	
	WIFI_TFT();
	
	CO2Timer();
	
	SunTimer();
	
	FutterTimer();
	
}

/************ Manuelle Funktionen ************/

BLYNK_WRITE(V12) { /*Sonne An*/

	int i = param.asInt();
	if (i == 1) {
		SonneIndex = 1;
		delay(250);
	}
}

BLYNK_WRITE(V14) { /*Sonne Aus*/

	int i = param.asInt();
	if (i == 1) {
		SonneIndex = 2;
		delay(250);
	}
}

BLYNK_WRITE(V13) { /*Sonne Mittag AN*/

	int i = param.asInt();
	if (i == 1) {
		SonneIndex = 3;
		delay(250);
		SonneIndex = 0;
	}
}

BLYNK_WRITE(V21) { /*Sonne Mittag Aus*/

	int i = param.asInt();
	if (i == 1) {
		SonneIndex = 4;
		delay(250);
		SonneIndex = 0;
	}
}

BLYNK_WRITE(V15) { /*Nachlicht Aus*/

	int i = param.asInt();
	if (i == 1) {
		SonneIndex = 5;
		delay(250);
		SonneIndex = 0;
	}
}

BLYNK_WRITE(V16) { /*Futterautomat*/

	ledcWrite(FutterKanal, Futtergesch);

	delay(Futterdauer);

	ledcWrite(FutterKanal, 0);

}

BLYNK_WRITE(V17) { /*Luefter*/

	int i = param.asInt();
	if (i == 1) {
		digitalWrite(luefter, HIGH);
		ledluefter.on();
		delay(250);
	}
}


/*********** System Neustart ************/

BLYNK_WRITE(V18) {

	int i = param.asInt();
	if (i == 1) {
		ESP.restart();
		delay(250);
	}
}

/************* LED Farbe Manuell *******/

BLYNK_WRITE(V29) {
	Blynk.virtualWrite(V29, param.asFloat());
	LEDRot = param.asFloat();

		for (int i = 0; i < NUMLEDS; i++) {

			strip1.SetPixelColor(i, RgbwColor(LEDGruen, LEDRot, LEDBlau, LEDWeiss));
		}
}

BLYNK_WRITE(V30) {
	Blynk.virtualWrite(V30, param.asFloat());
	LEDBlau = param.asFloat();
	
		for (int i = 0; i < NUMLEDS; i++) {

			strip1.SetPixelColor(i, RgbwColor(LEDGruen, LEDRot, LEDBlau, LEDWeiss));
		}
}

BLYNK_WRITE(V31) {
	Blynk.virtualWrite(V31, param.asFloat());
	LEDGruen = param.asFloat();
	
		for (int i = 0; i < NUMLEDS; i++) {

			strip1.SetPixelColor(i, RgbwColor(LEDGruen, LEDRot, LEDBlau, LEDWeiss));
		}
	

}

BLYNK_WRITE(V32) {
	Blynk.virtualWrite(V32, param.asFloat());
	LEDWeiss = param.asFloat();

		for (int i = 0; i < NUMLEDS; i++) {

			strip1.SetPixelColor(i, RgbwColor(LEDGruen, LEDRot, LEDBlau, LEDWeiss));
		}
}

BLYNK_WRITE(V37 ) {
	uint8_t Powerledmanu;
	Blynk.virtualWrite(V37, param.asFloat());
	Powerledmanu = param.asFloat();
	ledcWrite(PowerledKanal, Powerledmanu);
}

/*************************************************/

unsigned long currentMillis;

/*************************************************/
void setup() {


	Serial.begin(9600);

	/********* TFT Layout setzen ***************/

	TFT_Layout();

	

	pinMode(heizung, OUTPUT);
	pinMode(luefter, OUTPUT);
	pinMode(co2, OUTPUT);


	/**** Alarm Timer setzen für Funktionen ******/ 

	//Alarm.timerRepeat(1, digitalClockDisplay);
	//Alarm.timerRepeat(1, ProgrammTimer);
	//Alarm.timerRepeat(5, Heizung);

	Timer.setInterval(1000, digitalClockDisplay);
	Timer.setInterval(1000, ProgrammTimer);
	Timer.setInterval(5000, Heizung);
	//Alarm.timerRepeat(120, reconnectBlynk);

	/******* Blynk LCD löschen ******************/

	lcd.clear();

	/*********** Neopixel Starten ***************/

	strip1.Begin();
	strip1.ClearTo(0);
	strip1.SetPixelColor(10, RgbwColor(0, 0, 200, 0));
	strip1.SetBrightness(250);
	aktHell = maxHell;
	strip1.Show();

	/************ Tempfuehler *******************/

	Tempfueh.begin();
	//Tempfueh.setResolution(9);

	/******** TFT Backlight *******************/

	ledcSetup(BacklightKanal, BacklightFrequenz, BacklightBit);	//ledcSetup(Kanal, Frequenz, Bit);
	ledcAttachPin(BacklightPin, BacklightKanal);				//ledcAttachPin(Pin, Kanal);

	/******** Futterautomat *******************/

	ledcSetup(FutterKanal, Futterfreq, FutterRes);
	ledcAttachPin(FutterPin, FutterKanal);

	/******* PowerLED ************************/

	ledcSetup(PowerledKanal, Powerledfreq, PowerledBit);
	ledcAttachPin(PowerledPin, PowerledKanal);

	
	///******** Blynk Verbinden / WIFI Verbinden **********/
	WIFI_login();

	//Blynk.begin(auth, ssid, pass);

	Blynk.syncAll();								  // Werte aus Blynk Cloud laden


	/*************** WEB Server für OTA *******/

	if (WiFi.waitForConnectResult() == WL_CONNECTED) {
		MDNS.begin(host);
		server.on("/", HTTP_GET, []() {
			server.sendHeader("Connection", "close");
			server.send(200, "text/html", serverIndex);
			});
		server.on("/update", HTTP_POST, []() {
			server.sendHeader("Connection", "close");
			server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "Update OK Neustart!!");
			ESP.restart();
			}, []() {
				HTTPUpload& upload = server.upload();
				if (upload.status == UPLOAD_FILE_START) {
					Serial.setDebugOutput(true);
					Serial.printf("Update: %s\n", upload.filename.c_str());
					if (!Update.begin()) { //start with max available size
						Update.printError(Serial);
					}
				}
				else if (upload.status == UPLOAD_FILE_WRITE) {
					if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
						Update.printError(Serial);
					}
				}
				else if (upload.status == UPLOAD_FILE_END) {
					if (Update.end(true)) { //true to set the size to the current progress
						Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
					}
					else {
						Update.printError(Serial);
					}
					Serial.setDebugOutput(false);
				}
			});
		server.begin();
		MDNS.addService("http", "tcp", 80);

		Serial.printf("Ready! Open http://%s.local in your browser\n", host);
	}
	else {
		Serial.println("WiFi Failed");
	}


	/******** Time Server Satr und setzen *****/

	Udp.begin(localPort);
	setSyncProvider(getNtpTime);
	setSyncInterval(300);
	digitalClockDisplay();

}

time_t prevDisplay = 0;						// when the digital clock was displayed

void reconnectBlynk() {
	WIFI_TFT();

	if (!Blynk.connected()) {
		if (Blynk.connect()) {
			tft.drawBitmap(140, 0, wlan, 20, 20, TFT_GREEN);
		}
		else {
			tft.drawBitmap(140, 0, wlan, 20, 20, TFT_RED);
			delay(2);
			Blynk.begin(auth, ssid, pass);
		}
	}
}


void WIFI_login() {

	tft.drawBitmap(140, 0, wlan, 20, 20, TFT_GREEN);
	Serial.print("U");
	while (WiFi.status() != WL_CONNECTED && wifi_retry <= 5) {
		wifi_retry++;
		WiFi.persistent(false);   // daten nicht in Flash speichern
		WiFi.mode(WIFI_STA);
		Serial.printf("Connecting to %s ", ssid);
		tft.drawBitmap(140, 0, wlan, 20, 20, TFT_RED);
		WiFi.begin(ssid, pass);
		Blynk.config(auth);
		Blynk.connect();
		while (WiFi.status() != WL_CONNECTED)
		{
			delay(500);
			Serial.print(".");
		}
		
		Serial.println(" connected");
		Serial.print("local IP:");
		Serial.println(WiFi.localIP());
	}

	if (wifi_retry >= 5) {
		wifi_retry == 0;
		Serial.println("\nReboot");
		ESP.restart();
	}

}


void loop() {

	Blynk.run();
	Timer.run();
	server.handleClient();

	strip1.SetBrightness(aktHell);
	strip1.Show();

	currentMillis = millis();

	
	/******** Schalter für Beleuchtung ********/

	switch (SonneIndex) {

	case 1:
		SonneAuf();
		break;
	case 2:
		SonneUn();
		break;
	case 3:
		SonneMitAn();
		break;
	case 4:
		SonneMitAus();
		break;
	case 5:
		SonneNaAus();
		break;
	}

}

void ProgrammTimer() {
	/*************** Timer Programme ******************/

	uint32_t local_t = nowLocal();							//für Zeitumstellung nötig
	Zeit = (minute(local_t)) + (hour(local_t) * 100);		//RTC zeit in vier stellig Umrechnen (z.B. 12:30 nach 1230)

	uint16_t SoTimerAu;
	SoTimerAu = ((SoAuMin)+(SoAuStd) * 100);

	if ((Zeit == SoTimerAu) & (second(local_t) == 0))
	{
		SonneIndex = 1;
	}

	uint16_t SoTimerUn;
	SoTimerUn = ((SoUnMin)+(SoUnStd) * 100);

	if ((Zeit == SoTimerUn) & (second(local_t) == 0))
	{
		SonneIndex = 2;
	}

	uint16_t SoTimerMiAn;
	SoTimerMiAn = ((SoMiAnMin)+(SoMiAnStd) * 100);
	if ((Zeit == SoTimerMiAn) & (second(local_t) == 0))
	{
		SonneIndex = 3;
	}

	uint16_t SoTimerMiAus;
	SoTimerMiAus = ((SoMiAusMin)+(SoMiAusStd) * 100);
	if ((Zeit == SoTimerMiAus) & (second(local_t) == 0))
	{
		SonneIndex = 4;
	}

	uint16_t SoTimerNaAn;
	SoTimerNaAn = ((SoNaMin)+(SoNaStd) * 100);
	if ((Zeit == SoTimerNaAn) & (second(local_t) == 0))
	{
		SonneIndex = 5;
	}

	uint16_t FutterTimer;
	FutterTimer = ((FutterMin)+(FutterStd) * 100);
	if ((Zeit == FutterTimer) & (second(local_t) == 0))
	{
		Futterautomat();
	}

	uint16_t CO2AN;
	uint16_t CO2AUS;
	CO2AN = ((CO2AnMin)+(CO2AnStd) * 100);
	CO2AUS = ((CO2AusMin)+(CO2AusStd) * 100);
	if ((Zeit > (CO2AN - 1)) && (Zeit < CO2AUS))
	{
		digitalWrite(co2, HIGH);
		ledco2.on();
	}
	else
	{
		digitalWrite(co2, LOW);
		ledco2.off();
	}
}



