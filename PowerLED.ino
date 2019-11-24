void PowerLEDplus()
{
	uint8_t PowerLEDFade;
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
		}
}

void PowerLEDminus()
{
	uint8_t PowerLEDFade;
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