#include <LiquidCrystal.h>
#include <avr/io.h>
#include <avr/interrupt.h>
// include I2C library
#include <Wire.h>

#define NR_SONGS 4

#define PAUSE_ON 1
#define PAUSE_OFF 2
#define NEXT_SONG 3
#define PREVIOUS_SONG 4

volatile int songNumber = 0;
int receivedCommand;
int currentCounterValue;

volatile bool writeC = false;
volatile bool switchSong = false;
volatile bool newSecondPassed = false;
volatile bool inPause = false;
volatile bool buttonPressed;
int secondCounter;
LiquidCrystal lcd(7, 6, 5, 4, 3, 8);
String songNames[] = {"Imperial March", "Cantina Band", "Game of Thrones", "Ode to Joy", "Green Hill", "Song of Storms", "Super Mario"};

// Character matrix for the first character: every line is a
//row of pixels of the character
byte pause[8] = {
 B00000,
 B11011,
 B11011,
 B11011,
 B11011,
 B11011,
 B00000,
};
// Matrix for the second character
byte play[8] = {
 B10000,
 B11000,
 B11100,
 B11110,
 B11100,
 B11000,
 B10000,
};


void setup() {
 
  secondCounter = 0;
  
  pinMode(2 , INPUT);
  digitalWrite(2, LOW);
  attachInterrupt(digitalPinToInterrupt(2), previousFunction, RISING);
  
  Wire.begin(9);
  // attach a function to be called when we receive
  //something on the I2C bus
  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);
  
  // Initialize Timer1
  cli();// disable the global interrupts system in order to
  //setup the timers
  TCCR1A = 0; // SET TCCR1A and B to 0
  TCCR1B = 0;
  // Set the OCR1A with the desired value:
  OCR1A = 15624;
  // Active CTC mode:
  TCCR1B |= (1 << WGM12);
  // Set the prescaler to 1024:
  TCCR1B |= (1 << CS10);
  TCCR1B |= (1 << CS12);
  // Enable the Output Compare interrupt (by setting the
  //mask bit)
  TIMSK1 |= (1 << OCIE1A);
  
  
  lcd.begin(16, 2);
  
  lcd.createChar(0, pause);
  lcd.createChar(1, play);

  
  lcd.clear();
  lcd.setCursor(0, 0); // display received character
  lcd.print(songNames[songNumber]);
  
  lcd.setCursor(0, 1);
  lcd.print(secondCounter);
  
  lcd.setCursor(10, 1);
  lcd.write(byte(0));
  
  sei();
}

void receiveEvent(int bytes) {
  
	receivedCommand = Wire.read();
  
  	Serial.println(receivedCommand);
  
	switch(receivedCommand){
   
		case PAUSE_ON:
    
    		inPause = true;
      		writeC = true;
      		currentCounterValue = TCNT1;
    
    		break;
    	case PAUSE_OFF:
    
    		inPause = false;
      		writeC = true;
      		TCNT1 = currentCounterValue;
    
    		break;
    	case NEXT_SONG:
    		
            songNumber = (songNumber + 1) % NR_SONGS;
    		switchSong = true;
      		writeC = true;
    
    		break;
        case PREVIOUS_SONG:
      		
            (songNumber - 1 < 0) ? songNumber = (NR_SONGS - 1) : songNumber--;
      		switchSong = true;
      		writeC = true;
      		
      		break;
    	default:
    		break;
	}
  
}

ISR(TIMER1_COMPA_vect)
{
  newSecondPassed = true;
}

void loop() {
  
  
  if(inPause){
    TCNT1 = 0;
  }
  
  if(newSecondPassed){
     
      secondCounter++;
    
      lcd.setCursor(0, 1);
      lcd.print(secondCounter);
    
      newSecondPassed = false;
    
  }
  
  if(switchSong && !inPause){
    
    lcd.clear();
    lcd.setCursor(0, 0); // display received character
    lcd.print(songNames[songNumber]);
    
    secondCounter = 0;
    TCNT1 = 0;
    
    lcd.setCursor(0, 1);
    lcd.print(secondCounter);
    
    switchSong = false;
  }
  
  if(writeC){
	
    lcd.setCursor(10, 1);
    
    if(inPause)
		lcd.write(1);
    else
      	lcd.write(byte(0));
    
    writeC = false;
  }
}

void requestEvent(){
 
  if(buttonPressed){
    
    Wire.write('1');
    buttonPressed = false;
    
  }else
    Wire.write('0');
}

void previousFunction(){
 
  buttonPressed = true;
}