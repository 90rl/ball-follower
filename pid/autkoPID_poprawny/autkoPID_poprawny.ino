#include "ISAMobile.h"
#include <AutoPID.h>
#define M_PI 3.14159265358979323846


QMC5883 qmc;

void SetPowerLevel(PowerSideEnum side, int level)
{
	level = constrain(level, -255, 255);
	
	if (side == PowerSideEnum::Right) {
		if (level > 0) {
			// do przodu
			digitalWrite(A_PHASE, 1);
			analogWrite(A_ENABLE, level);
		} else if (level < 0) {
			// do tyłu
			digitalWrite(A_PHASE, 0);
			analogWrite(A_ENABLE, -level);
		} else {
			// stop
			digitalWrite(A_PHASE, 0);
			analogWrite(A_ENABLE, 0);
		}
	}
	
	if (side == PowerSideEnum::Left) {
		if (level > 0) {
			// do przodu
			digitalWrite(B_PHASE, 1);
			analogWrite(B_ENABLE, level);
		} else if (level < 0) {
			// do tyłu
			digitalWrite(B_PHASE, 0);
			analogWrite(B_ENABLE, -level);
		} else {
			// stop
			digitalWrite(B_PHASE, 0);
			analogWrite(B_ENABLE, 0);
		}
	}	
}

void setup(void)
{
	// Czujniki ultradźwiekowe
	for (int i = (int)UltraSoundSensor::__first; i <= (int)UltraSoundSensor::__last; i++)
	{
		pinMode(ultrasound_trigger_pin[i], OUTPUT);
		pinMode(ultrasound_echo_pin[i], INPUT);
		
		digitalWrite(ultrasound_trigger_pin[i], 0);
	}
	
	// Silniki
	pinMode(A_PHASE, OUTPUT);
	pinMode(A_ENABLE, OUTPUT);
	pinMode(B_PHASE, OUTPUT);
	pinMode(B_ENABLE, OUTPUT);
	
	//pinMode(MODE, OUTPUT); -- podłaczone na krótko ze stanem wysokim
	//digitalWrite(MODE, true);  -- podłaczone na krótko ze stanem wysokim
	
	SetPowerLevel(PowerSideEnum::Left, 0);
	SetPowerLevel(PowerSideEnum::Right, 0);

	// Wejścia enkoderowe
	pinMode(ENCODER_LEFT, INPUT);
	pinMode(ENCODER_RIGHT, INPUT);
	
	Serial.begin(9600);
	Serial.print("Test... ");
	
	Wire.begin();
	qmc.init();
	
	Serial1.begin(9600); // HC06

}

int measureSoundSpeed(int trigger_pin, int echo_pin)
{
	digitalWrite(trigger_pin, false);
	delayMicroseconds(2);

	digitalWrite(trigger_pin, true);
	delayMicroseconds(10);
	digitalWrite(trigger_pin, false);

	// zmierz czas przelotu fali dźwiękowej
	int duration = pulseIn(echo_pin, true, 50 * 1000);
	

	// przelicz czas na odległość (1/2 Vsound(t=20st.C))
	int distance = (int)((float)duration * 0.03438f * 0.5f);
	return distance;
}

#if 0
void loop(void)
{
	delay(1000);


	SetPowerLevel(Side_Left, 100);
	delay(2000);

	SetPowerLevel(Side_Left, 200);
	delay(2000);

	SetPowerLevel(Side_Left, 255);
	delay(2000);


	SetPowerLevel(Side_Left, 0);
	SetPowerLevel(Side_Right, 0);

	SetPowerLevel(Side_Left, -100);
	delay(2000);

	SetPowerLevel(Side_Left, -200);
	delay(2000);

	SetPowerLevel(Side_Left, -255);
	delay(2000);

	SetPowerLevel(Side_Left, 0);
	SetPowerLevel(Side_Right, 0);
	delay(4000);
	
	
		
	
	delay(1000);

	SetPowerLevel(Side_Right, 100);
	delay(2000);

	SetPowerLevel(Side_Right, 200);
	delay(2000);

	SetPowerLevel(Side_Right, 255);
	delay(2000);


	SetPowerLevel(Side_Right, 0);
	SetPowerLevel(Side_Right, 0);

	SetPowerLevel(Side_Right, -100);
	delay(2000);

	SetPowerLevel(Side_Right, -200);
	delay(2000);

	SetPowerLevel(Side_Right, -255);
	delay(2000);

	SetPowerLevel(Side_Right, 0);
	SetPowerLevel(Side_Right, 0);
	delay(4000);
}


void xloop() {
	
	qmc.measure();
	int16_t x = qmc.getX();
	int16_t y = qmc.getY();
	int16_t z = qmc.getZ();

	char buf[100];
	sprintf(buf, "\n %5d %5d %5d", x, y, z);
	Serial.print(buf);
	delay(100);
}



#endif


void cmd_proximity(const char* msg, UltraSoundSensor sensor)
{
	if (sensor == UltraSoundSensor::All) {

		char buffer[128];
			
		int d[4][5] = {0};
		int sum[4] = {0};
		int id[4] = {0};
		int dist[4] = {0};
		
		while (Serial.available() == 0)
		{
			for (int sens = (int)UltraSoundSensor::Front; sens <= (int)UltraSoundSensor::Right; sens++) {
				dist[sens] = measureSoundSpeed(
					ultrasound_trigger_pin[sens],
					ultrasound_echo_pin[sens]);

				// średnia krocząca
				sum[sens] -= d[sens][id[sens]];
				sum[sens] += d[sens][id[sens]] = dist[sens];
				id[sens] = (id[sens] + 1) % 5;
				dist[sens] = sum[sens] / 5;

			}
			sprintf(buffer, "\nFRONT: %4dcm; BACK: %4dcm; LEFT: %4dcm; RIGHT: %4dcm; ",
				dist[(int)UltraSoundSensor::Front],
				dist[(int)UltraSoundSensor::Back],
				dist[(int)UltraSoundSensor::Left],
				dist[(int)UltraSoundSensor::Right]);
			Serial.print(buffer);
		}

	
	} else {
		
		char buffer[64];
		int d[5] = {};
		int sum = 0;
		int id = 0;
		
		while (Serial.available() == 0)
		{
			int dist = measureSoundSpeed(
				ultrasound_trigger_pin[(int)sensor],
				ultrasound_echo_pin[(int)sensor]);

			// średnia krocząca
			sum -= d[id];
			sum += d[id] = dist;
			id = (id + 1) % 5;
			dist = sum / 5;

			sprintf(buffer, "\n%s: %0dcm", msg, dist);
			Serial.print(buffer);
		}
		
	}
	
	while (Serial.available())
		Serial.read();	
}


void cmd_qmc(void)
{
	char buffer[64];

	qmc.reset();
	while (Serial.available() == 0)
	{
		qmc.measure();
		int16_t x = qmc.getX();
		int16_t y = qmc.getY();
		int16_t z = qmc.getZ();

		sprintf(buffer, "\n X=%5d Y=%5d Z=%5d", x, y, z);
		Serial.print(buffer);
	}
	
	while (Serial.available())
		Serial.read();	
}


//void cmd_bluetooth(void)
//{
//	Serial.println("### HC06: Tryb komunikacji z modułem HC06. Aby wyjść, wpisz \"++++++\"...");
//	Serial.println("### Protokół: Moduł analizuje czas otrzymywania danych; polecenie musi");
//	Serial.println("###           kończyć się krótką przerwą (ok. 500ms) BEZ znaku nowej linii");
//	Serial.println("### Testy:    Wyślij AT (dokładnie dwa bajty)");
//	Serial.println("### Klient:   Wykorzystaj apkę androidową (np. Serial Bluetooth Terminal");
//	Serial.println("### Moduł:    Miganie diod oznacza brak sparowanego urządzenia; pin=1234");
//	
//	Serial.print("\n> ");
//	
//	int plus_counter = 0;
//	while (true) {
//		int b = 0;
//		if (Serial.available()) {
//			
//			b = Serial.read();
//		
//			if (b == '+') {
//				plus_counter++;
//				if (plus_counter >= 6)
//					break; // wyjdź na 6 plusów
//			}
//		
//		
//			if (b != '\n') // HC06 nie lubi znaków nowej linii ;)
//				Serial1.write(b);	// wyślij do hc06
//			
//			Serial.write(b);	// echo lokalne
//		}
//			
//		if (Serial1.available()) {
//			int b = Serial1.read();
//			Serial.write(b);
//		}
//		
//	}
//	
//	Serial.println("HC06: Koniec.");
//}



void cmd_serial0(void)
{
	Serial.println("### Komunikacja po porcie szeregowym Serial0 Aby wyjść, wpisz \"++++++\"...");
	Serial.println("### Parametry łacza: 9600bps, 8 bitów danych, brak parzystości, 1 bit stopu (9600,8N1)");

	Serial.print("\n> ");
	
	int plus_counter = 0;
	while (true) {
		int b = 0;
		if (Serial.available()) {
			
			b = Serial.read();
		
			if (b == '+') {
				plus_counter++;
				if (plus_counter >= 6)
					break; // wyjdź na 6 plusów
			}
		
		
			Serial1.write(b);	// wyślij do urządzenia zewnętrznego (np. raspberry pi)
			Serial.write(b);	// echo lokalne
		}
			
		if (Serial1.available()) {
			int b = Serial1.read();
			Serial.write(b);
		}
		
	}
	
	Serial.println("Serial0: Koniec.");
}

void cmd_encoders(void)
{
	pinMode(ENCODER_LEFT, INPUT);
	pinMode(ENCODER_RIGHT, INPUT);
	
	char buffer[] = {'\n', 'L', '-','R','-','\x0'}; // 2, 4
	
	while (Serial.available() == 0)
	{
		buffer[2] = '0' + digitalRead(50);
		buffer[4] = '0' + digitalRead(51);
		
		Serial.print(buffer);
	}
	
	while (Serial.available())
		Serial.read();	
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define TEMP_READ_DELAY 800

//pid settings and gains
#define OUTPUT_MIN -5
#define OUTPUT_MAX 5
#define KP 0.12
#define KI 0.0003
#define KD 0

double input, setPoint, outputVal=0;

AutoPID myPID(&input, &setPoint, &outputVal, OUTPUT_MIN, OUTPUT_MAX, KP, KI, KD);

unsigned long lastTempUpdate;


void loop(void)
{
	delay(800);
	qmc.measure();
  int16_t x = qmc.getX();
  int16_t y = qmc.getY();

  

//Sprawdzenie gdzie jesteśmy z odczytami
    char buffer[64];
    sprintf(buffer, "\n X=%5d Y=%5d", x, y);
    Serial.print(buffer);
  // heading jest podany w radianach
  float heading = atan2(y, x);
  //przeliczenie z radianów na stopnie
  float  headingRad=heading*180/M_PI;
  
    sprintf(buffer, "\nheadingRad  X=%5f", headingRad);
    Serial.print(buffer);

   //wartość wejściowa
  setPoint = -60; // 0 stopni deg
  input=headingRad;
  
  myPID.run();
    sprintf(buffer, "\noutputVal  X=%5f", outputVal);
    Serial.print(buffer);

float normalSpeed=150;
float s=10;


  if((float)outputVal>0 ){
   SetPowerLevel(PowerSideEnum::Right, normalSpeed+outputVal*s);
   SetPowerLevel(PowerSideEnum::Left, normalSpeed-outputVal*s);
  } else if((float)outputVal<0 ){
   SetPowerLevel(PowerSideEnum::Right, normalSpeed+outputVal*s);
   SetPowerLevel(PowerSideEnum::Left, normalSpeed-outputVal*s); 
  } else{
   SetPowerLevel(PowerSideEnum::Right, normalSpeed);
   SetPowerLevel(PowerSideEnum::Left, normalSpeed);
  }









///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////








//if((float)outputVal>0){
//  
//    SetPowerLevel(PowerSideEnum::Right, 150);
//    SetPowerLevel(PowerSideEnum::Left, 150-((float)outputVal-0)*40);
//  Serial.println("\nSkręć w lewo");
//}else if((float)outputVal<0) {
//    SetPowerLevel(PowerSideEnum::Right, 150-(0-(float)outputVal)*40);
//    SetPowerLevel(PowerSideEnum::Left, 150);
//  Serial.println("\nSkręć w prawo");
//}else{
//    SetPowerLevel(PowerSideEnum::Right, 150);
//    SetPowerLevel(PowerSideEnum::Left, 150);
//}
//    SetPowerLevel(PowerSideEnum::Right, isTurnLeft ? 100 + s : 100);
//    SetPowerLevel(PowerSideEnum::Left, !isTurnLeft ? 100 + s : 100);
//
//  switch(flaga){
//    case 1:
//        SetPowerLevel(PowerSideEnum::Left, normal);
//        SetPowerLevel(PowerSideEnum::Right, skret);
//
//        break;
//    case 2:
//        SetPowerLevel(PowerSideEnum::Left, normal);
//        SetPowerLevel(PowerSideEnum::Right, zawrot);
//        break;
//    case 3:
//        SetPowerLevel(PowerSideEnum::Left, zawrot);
//        SetPowerLevel(PowerSideEnum::Right, normal);
//        break;
//    case 4:
//        SetPowerLevel(PowerSideEnum::Left, skret);
//        SetPowerLevel(PowerSideEnum::Right, normal);
//        break;
//    case 5: //jedz prosto
//        SetPowerLevel(PowerSideEnum::Left, normal);
//        SetPowerLevel(PowerSideEnum::Right, normal);
//        break;        
//  }
//  








////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  
//	for (int i = 0; i < 10; i++)
//	{
//		Serial.print((char)(43+2*(i&1)));
//		delay(200);
//	}
//	Serial.println();
//	Serial.println("=======================================================");
//	Serial.println("# Programowanie Systemow Autonomicznych               #");
//	Serial.println("# Tester autek v1.0 Tomasz Jaworski, 2018             #");
//	Serial.println("=======================================================");
//	Serial.println("Polecenia powinny konczyc sie wylacznie znakiem '\\n'.");
//	Serial.println("ARDUINO IDE: Zmień 'No line ending' na 'Newline' w dolnej części okna konsoli...\n");
//	
//	while(1)
//	{
//		Serial.print("> ");
//
//		String s = "";
//		while(true)
//		{
//			while(Serial.available() == 0);
//			int ch = Serial.read();
//			if (ch == '\n')
//				break;
//			s += (char)ch;
//		}
//		
//		s.trim();
//		s.toLowerCase();
//		Serial.println(s);
//		
		//
//		
//		if (s == "help")
//		{
//			Serial.println("Pomoc:");
//			Serial.println("  proxf   - odczytuj czujnik odleglosc (PRZEDNI)");
//			Serial.println("  proxb   - odczytuj czujnik odleglosc (TYLNY)");
//			Serial.println("  proxl   - odczytuj czujnik odleglosc (LEWY)");
//			Serial.println("  proxr   - odczytuj czujnik odleglosc (PRAWY)");
//			Serial.println("  prox    - odczytuj WSZYSTKICH czujniki odległości");
//			
//			Serial.println("  mSD p   - ustaw wysterowanie silnika napędowego");
//			Serial.println("	  S (strona): 'L'-lewa, 'R'-prawa, 'B'-obie");
//			Serial.println("  	  D (kierunek): 'F'-do przodu, 'B'-do tyłu, 'S'-stop");
//			Serial.println("   	  p (wysterowanie): poziom sterowania 0-255");
//			Serial.println("  enc     - Odczyt wejść enkoderów; wcześniej uruchom silniki");
//			Serial.println("  reset   - reset");
//			Serial.println("  qmc     - odczytuj pomiary pola magnetycznego w trzech osiach");
//			Serial.println("  bt      - komunikacja z modułem HC06 (Bluetooth)");
//			Serial.println("  serial0 - komunikacja porcie Serial0 (TX0/RX0, 96008N1)");
//			continue;
//		}
		
//		if (s == "reset") {
//			Serial.println("Ok.");
//			delay(1000);
//			RSTC->RSTC_MR = 0xA5000F01;
//			RSTC->RSTC_CR = 0xA500000D;
//			while(1);
//		}
//		
//		if (s == "bt") {
//			cmd_bluetooth();
//			continue;
//		}
//		
//		if (s == "proxf") {
//			cmd_proximity("PRZOD", UltraSoundSensor::Front);
//			continue;
//		}
//		
//		if (s == "proxb") {
//			cmd_proximity("TYL", UltraSoundSensor::Back);
//			continue;
//		}
//		
//		if (s == "proxl") {
//			cmd_proximity("LEWY", UltraSoundSensor::Left);
//			continue;
//		}
//		
//		if (s == "proxr") {
//			cmd_proximity("PRAWY", UltraSoundSensor::Right);
//			continue;
//		}
//
//		if (s == "prox") {
//			cmd_proximity(nullptr, UltraSoundSensor::All);
//			continue;
//		}
//		
//		if (s == "qmc") {
//			cmd_qmc();
//			continue;
//		}
//
//		if (s == "enc") {
//			cmd_encoders();
//			continue;
//		}
//
//		if (s == "serial0") {
//			cmd_serial0();
//			continue;
//		}
//
//		
//		if (s.startsWith("m")) {
//			if (s.length() < 3) {
//				Serial.println("Polecenie 'm': bład w poleceniu");
//				continue;
//			}
//			
//			int side = tolower(s[1]);
//			int direction = tolower(s[2]);
//			int power = -1;
//			if (s.indexOf(" ") != -1) {
//				s = s.substring(s.lastIndexOf(" ") + 1);
//				char *endptr = NULL;
//				power = strtol(s.c_str(), &endptr, 10);
//				if (*endptr != '\0') {
//					Serial.println("Polecnie 'm': bład w zapisie wartości wystarowania");
//					continue;
//				}
//			}
//			
//			if (strchr("lrb", side) == NULL) {
//				Serial.println("Polecnie 'm': bład w formacie strony");
//				continue;
//			}
//				
//			if (strchr("fbs", direction) == NULL) {
//				Serial.println("Polecnie 'm': bład w formacie kierunku");
//				continue;
//			}
//			
//			if (direction != 's' && power == -1) {
//				Serial.println("Polecnie 'm': brak podanej wartości wysterowania");
//				continue;
//			}
//				
//			// przekształcenia
//			bool left = side == 'l' || side == 'b';
//			bool right = side == 'r' || side == 'b';
//			power = direction == 's' ? 0 : power;
//			power = direction == 'b' ? -power : power;
//
//			char msg[128];
//			sprintf(msg, "Ustawienia: L=%d, R=%d, power=%d\n", left, right, power);
//			Serial.print(msg);
//			if (left)
//				SetPowerLevel(PowerSideEnum::Left, power);
//			if (right)
//				SetPowerLevel(PowerSideEnum::Right, power);
//			
//			continue;
//		}
//
//		Serial.print(" Polecenie '");
//		Serial.print(s);
//		Serial.print("' jest nieznane; Może 'help'?\n");
//	}
//	}
}
