/******************* �berwachung Heizung ******************/

void Heizung() {

	portDISABLE_INTERRUPTS();
	Tempfueh.requestTemperatures();
	IstTemp = (Tempfueh.getTempCByIndex(0));
	portENABLE_INTERRUPTS();
	lcd.print(0, 1, IstTemp);
	//lcd.print(6, 1, "C");
	//lcd.print(7, 1,(char)178);
	tft.setTextColor(TFT_WHITE, TFT_BLACK);
	tft.drawFloat(SollTemp, 1, 27, 47, 2);				//SollWert in TFT einblenden
if (IstTemp > -127)
{

	Blynk.virtualWrite(V3, IstTemp);					//IstTemp an Blynk senden

		if (IstTemp < SollTemp - Hysterese)
		{
			digitalWrite(heizung, HIGH);
			tft.setTextColor(TFT_RED, TFT_BLACK);
			tft.drawFloat(IstTemp, 1, 90, 33, 4);
			ledheizung.on();
			//Blynk.virtualWrite(V3, IstTemp);			//IstTemp an Blynk senden
		}

		if (IstTemp > SollTemp + Hysterese)
		{
			digitalWrite(heizung, LOW);
			tft.setTextColor(TFT_GREEN, TFT_BLACK);
			tft.drawFloat(IstTemp, 1, 90, 33, 4);
			ledheizung.off();
			//Blynk.virtualWrite(V3, IstTemp);			//IstTemp an Blynk senden
		}

		/******************* Luefter Temperatur *******************/
		if (IstTemp >= LuefTemp)
		{
			digitalWrite(luefter, HIGH);
			ledluefter.on();
		}
		else
		{
			digitalWrite(luefter, LOW);
			ledluefter.off();
		}
}
}

/************************** NTP code ********************************************/

time_t getNtpTime()
{
	IPAddress ntpServerIP; // NTP server's ip address

	while (Udp.parsePacket() > 0); // discard any previously received packets
	Serial.println("Transmit NTP Request");
	// get a random server from the pool
	WiFi.hostByName(ntpServerName, ntpServerIP);
	Serial.print(ntpServerName);
	Serial.print(": ");
	Serial.println(ntpServerIP);
	sendNTPpacket(ntpServerIP);
	uint32_t beginWait = millis();
	while (millis() - beginWait < 1500) {
		int size = Udp.parsePacket();
		if (size >= NTP_PACKET_SIZE) {
			Serial.println("Receive NTP Response");
			tft.setTextColor(TFT_BLACK);
			tft.drawString("Zeit Server :-(", 1, 5);
			Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
			unsigned long secsSince1900;
			// convert four bytes starting at location 40 to a long integer
			secsSince1900 = (unsigned long)packetBuffer[40] << 24;
			secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
			secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
			secsSince1900 |= (unsigned long)packetBuffer[43];
			setSyncInterval(syncIntervalMax);
			return secsSince1900 - 2208988800UL; // Important: Systemtime = UTC!!!

		}
	}
	Serial.println("No NTP Response :-(");
	tft.setTextColor(TFT_RED);
	tft.drawString("0", 3, 2, 2);
	tft.drawString("0", 12, 2, 2);
	tft.drawString(":", 21, 2, 2);
	tft.drawString("0", 25, 2, 2);
	tft.drawString("0", 34, 2, 2);
	tft.drawString(":", 43, 2, 2);
	tft.drawString("0", 48, 2, 2);
	tft.drawString("0", 57, 2, 2);

	if (ntpWaitActual + 200 < ntpWaitMax) ntpWaitActual += 200; // expand wait time if necessary - I'm pretty aware of that this value will hardly reach ntpWaitMax with this simple condition.
	setSyncInterval(syncIntervalAfterFail); // in both cases: no NTP response or no DNS lookup

	return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
	// set all bytes in the buffer to 0
	memset(packetBuffer, 0, NTP_PACKET_SIZE);
	// Initialize values needed to form NTP request
	// (see URL above for details on the packets)
	packetBuffer[0] = 0b11100011;   // LI, Version, Mode
	packetBuffer[1] = 0;     // Stratum, or type of clock
	packetBuffer[2] = 6;     // Polling Interval
	packetBuffer[3] = 0xEC;  // Peer Clock Precision
							 // 8 bytes of zero for Root Delay & Root Dispersion
	packetBuffer[12] = 49;
	packetBuffer[13] = 0x4E;
	packetBuffer[14] = 49;
	packetBuffer[15] = 52;
	// all NTP fields have been given values, now
	// you can send a packet requesting a timestamp:
	Udp.beginPacket(address, 123); //NTP requests are to port 123
	Udp.write(packetBuffer, NTP_PACKET_SIZE);
	Udp.endPacket();
}
// returns the current date/time as UNIX timestamp, incl. timezone, including daylightsaving
uint32_t nowLocal()
{
	uint32_t local_t = now();
	if (isDayLightSaving(local_t))
		local_t += 3600 + timeZone * SECS_PER_HOUR;
	else
		local_t += timeZone * SECS_PER_HOUR;
	return local_t;
}

// calculates the daylight saving time for middle Europe. Input: Unixtime in UTC (!)
boolean isDayLightSaving(uint32_t local_t)
{
	if (month(local_t) < 3 || month(local_t) > 10) return false; // no DSL in Jan, Feb, Nov, Dez
	if (month(local_t) > 3 && month(local_t) < 10) return true; // DSL in Apr, May, Jun, Jul, Aug, Sep
																// if (month == 3 && (hour + 24 * day) >= (1 + tzHours + 24 * (31 - (5 * year / 4 + 4) % 7)) || month == 10 && (hour + 24 * day) < (1 + tzHours + 24 * (31 - (5 * year / 4 + 1) % 7)));
	if (month(local_t) == 3 && (hour(local_t) + 24 * day(local_t)) >= (1 + 24 * (31 - (5 * year(local_t) / 4 + 4) % 7)) || month(local_t) == 10 && (hour(local_t) + 24 * day(local_t)) < (1 + 24 * (31 - (5 * year(local_t) / 4 + 1) % 7)))
		return true;
	else
		return false;
}

/********************************************************************************/
