/******************* Ueberwachung Heizung ******************/

void Heizung() {

	//portDISABLE_INTERRUPTS();
	Tempfueh.requestTemperatures();
	IstTemp = (Tempfueh.getTempCByIndex(0));
	//Serial.println(IstTemp);
	//portENABLE_INTERRUPTS();
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

