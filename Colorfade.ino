
void SonneAuf()
{
	lcd.print(0, 0, "Sonnenaufgang");
	AblaufI ++;
	
	if (AblaufI >= 46) {
		AblaufX ++;
		AblaufI = 0;
		tft.drawFastVLine(AblaufX, 116, 11, TFT_WHITE);
		
		}

		switch (Durchlauf)
		{
		case 1:
			crossFade(SonAu1);
			break;
		case 2:
			crossFade(SonAu2);
			break;
		case 3:
			crossFade(SonAu3);
			break;
		case 4:
			crossFade(SonAu4);
			break;
		case 5:
			crossFade(SonAu5);
			break;
		case 6:
			crossFade(SonAu6);
			break;
		case 7:
			crossFade(SonAu7);
			break;
		/*case 8:
			PowerLEDplus();
			break;*/

		case 8:

				Durchlauf = 1;
				SonneIndex = 0;
				AblaufX = 1;
				lcd.print(0, 0, "                ");
			
			break;
		}

	}


void SonneUn()
{
	lcd.print(0, 0, "Sonnenuntergang");
	AblaufI ++;
	
	if (AblaufI >= 48) {
		AblaufY --;
		AblaufI = 0;
		tft.drawFastVLine(AblaufY, 116, 11, TFT_BLACK);
	
	}

	switch (Durchlauf)
	{
	case 1:
		crossFade(SonUn1);
		break;
	case 2:
		crossFade(SonUn2);
		break;
	case 3:
		crossFade(SonUn3);
		break;
	case 4:
		crossFade(SonUn4);
		break;
	case 5:
		crossFade(SonUn5);
		break;
	case 6:
		crossFade(SonUn6);
		break;
	case 7:
		crossFade(SonUn7);
		break;
	case 8:
		for (int i = 0; i < 10; i++) {
			
				strip1.SetPixelColor(i, RgbwColor(0, 0, 0, 0));
				delay(10);
				strip1.Show();
		}

		for (int i = 166; i > 17; i--) {

				strip1.SetPixelColor(i, RgbwColor(0, 0, 0, 0));
				delay(10);
				strip1.Show();
		}

		for (int i = 10; i < 17; i++) {

				strip1.SetPixelColor(i, RgbwColor(0, 0, 240, 0));
				delay(10);
				strip1.Show();
		}
		Durchlauf++;
		break;

	case 9:
		Durchlauf = 1;
		SonneIndex = 0; 
		AblaufY = 159;
		lcd.print(0, 0, "                ");
		break;
	}
}

void SonneMitAn() 
{

	lcd.print(0, 0, "Mittagssonne AN");
	strip1.SetBrightness(mittagHell);
	aktHell = mittagHell;
	strip1.Show();
	lcd.print(0, 0, "                ");
	SonneIndex = 0;
}

void SonneMitAus() 
{

	lcd.print(0, 0, "Mittagssonne AUS");
	strip1.SetBrightness(maxHell);
	aktHell = maxHell;
	strip1.Show();
	lcd.print(0, 0, "                ");
	SonneIndex = 0;
}

void SonneNaAus()
{
	lcd.print(0, 0, "Nachtlicht AUS");
	switch (Durchlauf)
	{
	case 1:
		crossFade(Nachtlicht1);
		break;

	case 2:
		Durchlauf = 1;
		SonneIndex = 0;
		lcd.print(0, 0, "                ");
		break;
	}
}

int calculateStep(int prevValue, int endValue) 
{
	int step = endValue - prevValue; // What's the overall gap?
	if (step) {                      // If its non-zero, 
		step = StepWert / step;              //   divide by 1020
	}
	return step;
}

/* The next function is calculateVal. When the loop value, i,
*  reaches the step size appropriate for one of the
*  colors, it increases or decreases the value of that color by 1.
*  (R, G, and B are each calculated separately.)
*/

int calculateVal(int step, int val, int i) 
{

	if ((step) && i % step == 0) { // If step is non-zero and its time to change a value,
		if (step > 0) {              //   increment the value if step is positive...
			val += 2;
		}
		else if (step < 0) {         //   ...or decrement it if step is negative
			val -= 2;
		}
	}
	// Defensive driving: make sure val stays in the range 0-255
	if (val > 255) {
		val = 255;
	}
	else if (val < 0) {
		val = 0;
	}
	return val;
}

/* crossFade() converts the percentage colors to a
*  0-255 range, then loops 1020 times, checking to see if
*  the value needs to be updated each time, then writing
*  the color values to the correct pins.
*/

void crossFade(int color[4]) 
{

	int stepR = calculateStep(prevR, (color[0] * 255) / 100);
	int stepG = calculateStep(prevG, (color[1] * 255) / 100);
	int stepB = calculateStep(prevB, (color[2] * 255) / 100);
	int stepW = calculateStep(prevW, (color[3] * 255) / 100);

	do {										// Delay schleife f�r�s Fading

		if (LEDStep <= StepWert) {

			LEDStep++;
			if (LEDStep >= (StepWert + 1)) {
				LEDStep = 0;
				Durchlauf++;
			}

			redVal = calculateVal(stepR, redVal, LEDStep);
			grnVal = calculateVal(stepG, grnVal, LEDStep);
			bluVal = calculateVal(stepB, bluVal, LEDStep);
			whiteVal = calculateVal(stepW, whiteVal, LEDStep);

			for (int i = 0; i < NUMLEDS; i++) {

				strip1.SetPixelColor(i, RgbwColor(grnVal, redVal, bluVal, whiteVal));
			}

			prevR = redVal;						// Update current values for next loop
			prevG = grnVal;
			prevB = bluVal;
			prevW = whiteVal;

			//delay(DurchWait);
			strip1.Show();

			crossFadeWait = DurchWait;
		}
	} while (crossFadeWait = 0);
	crossFadeWait --;


}

