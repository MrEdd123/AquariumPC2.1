void PowerLEDplus()
{
	uint16_t PowerLEDFade;
	PowerLEDFade = DurchWait * 50;
	
		if (currentMillis - previousMillis > PowerLEDFade) {
			previousMillis = currentMillis;
			if (Powerledwert <= Powerledmax) {
				Powerledwert++;
				ledcWrite(PowerledKanal, Powerledwert);
				}
			else
			{
				Durchlauf++;
			}

			Serial.print("PowerLED++ ");
			Serial.println(Powerledwert);	
			Serial.print(DurchWait);
			Serial.print("  ");
			Serial.println(PowerLEDFade);
		}
}

void PowerLEDminus()
{
	uint16_t PowerLEDFade;
	PowerLEDFade = DurchWait * 50;
	if (currentMillis - previousMillis > PowerLEDFade) {
		previousMillis = currentMillis;
		if (Powerledwert >= 0) {
			Powerledwert--;
			ledcWrite(PowerledKanal, Powerledwert);
		}
		if(Powerledwert == 0) 
		{
			Durchlauf++;
		}
		Serial.print("PowerLED-- ");
		Serial.println(Powerledwert);	
	}
}