
void Futterautomat()
{

	ledcWrite(FutterKanal, Futtergesch);

	delay(Futterdauer);

	ledcWrite(FutterKanal, 0);

	Serial.println("Futterautomat");

}