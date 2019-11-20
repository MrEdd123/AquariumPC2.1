void PowerLEDplus()
{
	if (currentMillis - previousMillis > 2000) {
		previousMillis = currentMillis;
		 if (Powerledwert <= Powerledmax){
		 Powerledwert++;
			 ledcWrite(PowerledKanal, Powerledwert);
		 }
		 Serial.print("PowerLED++ ");
		 Serial.println(Powerledwert);
	}
}

void PowerLEDminus()
{
	if (currentMillis - previousMillis > 2000) {
		previousMillis = currentMillis;
		if (Powerledwert >= 1) {
			Powerledwert--;
			ledcWrite(PowerledKanal, Powerledwert);
		}
		Serial.print("PowerLED-- ");
		Serial.println(Powerledwert);
	}
}