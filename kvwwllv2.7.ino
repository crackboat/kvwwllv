//#include<math.h>
//#include <stdio.h>

#include <LiquidCrystal.h>

///----------------------------------------------------------------------------------------------------------------------------------

LiquidCrystal lcd(12, 11, 9, 4, 3, 2);

int 	SignalPin = 13;

int 	LEDpinStatus = 10;

int 	switchPinUp = 6;  				//Active LOW
int 	switchPinDown = 7;  			//Active LOW
int 	switchPinEnter = 8;  			//Active LOW
int 	switchPinStopEmergency = 1;		//Active LOW

int 	TimeSweep = 30;		//Sec.
int 	SweepFreqStart = 1;	//kHz
int 	SweepFreqStop = 5;	//kHz
float	SweepParam = 0.06;		//Sweep Parameter in for loop. Will be calculate from TimeSweep.

		// 0.006 for 5 mins.
		// div  <-->  mul

bool	IsStop = true;

// the setup function runs once when you press reset or power the board
void setup() {
	lcd.begin(16, 2);
	
	lcd.setCursor(0, 0);
	lcd.print("Initializing");
	delay(200);
	lcdDot(13,0,100);

	pinMode(SignalPin, OUTPUT);   					//  Signal out

	pinMode(LEDpinStatus, OUTPUT);  				//   LED Status  start/stop

	pinMode(switchPinUp, 			INPUT_PULLUP);  //  User change
	pinMode(switchPinDown, 			INPUT_PULLUP);  //  User change
	pinMode(switchPinEnter, 		INPUT_PULLUP);	//  User change
	pinMode(switchPinStopEmergency, INPUT_PULLUP);	//  User stop/start

	lcd.setCursor(0, 0);
	lcd.print("Done.           ");
	delay(800);
}
	// the loop function runs over and over again forever
void loop() {
	String 	FreqStart;
	String 	FreqStop;
	String  RUN;

	if(IsStop == true){IsStop = false;}

    while(!IsStop){
		FreqStart = String(SweepFreqStart);
		FreqStop = String(SweepFreqStop);

		RUN = "Running ";
		RUN += FreqStart;
		RUN += "-";
		RUN += FreqStop;
		RUN += "kHz  ";

		lcd.setCursor(0, 0);
		lcd.print(RUN);

		digitalWrite(LEDpinStatus, HIGH);

        switch (Sweep(SweepParam)) {
            case 0:    // User does nothing
                break;
            case 1:    // User change some
				if(IsStop == false){IsStop = true;}
				digitalWrite(LEDpinStatus, LOW);
				lcdStop();
				setTimeSweep();
                break;
            case 2:    // User stop
				IsStop = true;
				digitalWrite(LEDpinStatus, LOW);
				lcd.setCursor(0, 0);
				lcd.print("Emergency Stop. ");
				lcdClear(0,1);

				while(digitalRead(switchPinStopEmergency) == HIGH){
					if(digitalRead(switchPinStopEmergency) == LOW){
						while(digitalRead(switchPinStopEmergency) == LOW){
							digitalWrite(LEDpinStatus, LOW);
							delay(400);
							digitalWrite(LEDpinStatus, HIGH);
							delay(400);
						}
						digitalWrite(LEDpinStatus, LOW);
						lcdClear(0,0);
						lcd.setCursor(0, 0);
						lcd.print("Starting");
						delay(500);
						lcdDot(9,0,100);
						lcdClear(0,1);
						break;
					}
				}

				IsStop = false;
				digitalWrite(LEDpinStatus, HIGH);

				break;
		}
	}
}

int Sweep(float SweepParam){
	float	i, j;
	double	timeCounter = 0;
	int		UserDoes = 0;
	int		DisplaySec = TimeSweep;
	bool	IsUserStop = false;
	bool	IsUserChange = false;
	
	lcdClear(8,0);
  	lcd.setCursor(14, 0);
  	lcd.print("Hz");
	lcd.setCursor(0, 1);
	lcd.print("Remaining       ");

	// increment each 0.00115 approx. 27 min 40 sec
	// 0.006 for 5 mins.
	// div  <-->  mul

	// i start 999 final 6300
	// Freq. 970 Hz to 5.0 kHz
	for(i = 999 ; i < 6300 ; i = i + SweepParam){
		int k = i;
		i = 1/(2*i);
		i = i*1000000;

		digitalWrite(SignalPin, HIGH);						//---------------------------------------------------------------------------

		// MCU delay 91.16 uSec. Prevent overflow
		if(i>83.1){delayMicroseconds(i-83.1);}
			else{break;}

		j = i;
		i = i/1000000;
		i = 1/(2*i);

		if(IsUserChange == true){UserDoes = 1;	break;}
		if(IsUserStop == true){UserDoes = 2;	break;}

		while(digitalRead(switchPinUp) == LOW){
			IsUserChange = true;
		}
		while(digitalRead(switchPinDown) == LOW){
			IsUserChange = true;
		}
		while(digitalRead(switchPinEnter) == LOW){
			IsUserChange = true;
		}
		while(digitalRead(switchPinStopEmergency) == LOW){
			lcdClear(0,0);
			lcdClear(0,1);
			IsUserStop = true;
		}

		digitalWrite(SignalPin, LOW);						//---------------------------------------------------------------------------

		timeCounter += j*2;
		if(j>70){delayMicroseconds(j-70);}
		else{break;}

		if(timeCounter >= 1000000){

			timeCounter=0;
			lcd.setCursor(9, 0);
			lcd.print(k);

			if(DisplaySec/1000 >= 1){
				lcdClear(12,1);
				lcd.setCursor(12, 1);
				lcd.print(DisplaySec--);
			}
			else if(DisplaySec/100 >= 1){
				lcdClear(12,1);
				lcd.setCursor(13, 1);
				lcd.print(DisplaySec--);
			}
			else if(DisplaySec/10 >= 1){
				lcdClear(12,1);
				lcd.setCursor(14, 1);
				lcd.print(DisplaySec--);
			}
			else{
				lcdClear(12,1);
				lcd.setCursor(15, 1);
				lcd.print(DisplaySec--);
			}
		}
	}
	return  UserDoes;
}

void lcdClear(int cur, int row){
	lcd.setCursor(cur, row);
	lcd.print("                ");
}

void lcdStop(){
	lcdClear(0,0);
	lcdClear(0,1);
	lcd.setCursor(0, 0);
	lcd.print("Stopping");
	delay(200);
	lcdDot(8,0,100);
	lcd.setCursor(0, 0);
	lcd.print("Stopped           ");
	lcdClear(0,1);
	delay(500);
}

void setTimeSweep(){
	int TimeEnter[] = {0, 0, 0, 0};
	int i;
	bool next = false;
	String TimeCon;

	lcdClear(0,0);
	lcdClear(0,1);

	lcd.setCursor(0, 0);
	lcd.print("Setting Time");
	lcd.setCursor(0, 1);
	lcd.print("0000");
	lcd.setCursor(12, 1);
	lcd.print("Sec.");

	lcd.setCursor(0, 1);
	lcd.cursor();
	delay(300);

	for(i=0;i<=3;i++){
		next = false;
		while(!next){
			lcd.setCursor(i, 1);
			if(digitalRead(switchPinUp) == LOW){
				delay(200);
				if(TimeEnter[i] == 9){
					TimeEnter[i] = 0;
				}
				else{TimeEnter[i]++;}

				lcd.print(TimeEnter[i]);
			}

			if(digitalRead(switchPinDown) == LOW){
				delay(200);
				if(TimeEnter[i] == 0){
					TimeEnter[i] = 9;
				}
				else{TimeEnter[i]--;}

				lcd.print(TimeEnter[i]);
			}
			if(digitalRead(switchPinEnter) == LOW){
				delay(200);
				next = true;
			}
		}
	}

	for(i=0;i<=3;i++){
		TimeCon += TimeEnter[i];
	}
	TimeEnter[5] = TimeCon.toInt();

	lcdClear(0,0);
	lcdClear(0,1);
	lcd.setCursor(0, 0);
	lcd.print("Time Enter");
	lcd.setCursor(12, 1);
	lcd.print("Sec.");
	lcd.setCursor(0, 1);
	lcd.print(TimeEnter[5]);
	delay(2000);

	TimeSweep = TimeEnter[5];
	SweepParam = 1.8/TimeEnter[5];

	lcd.noCursor();
}

void lcdDot(int column, int row, int delaySec){
	lcd.setCursor(column, row);
	lcd.print(".");
	delay(delaySec);
	
	lcd.setCursor(column+1, row);
	lcd.print(".");
	delay(delaySec);

	lcd.setCursor(column+2, row);
	lcd.print(".               ");
	delay(delaySec);
}
