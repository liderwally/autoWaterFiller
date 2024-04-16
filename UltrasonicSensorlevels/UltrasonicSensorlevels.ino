
#include <LiquidCrystal.h>
#pragma GCC push_options
#pragma GCC optimize ("Os")
#pragma GCC pop_options





//constants.....

const int tankheight = 150; // height of a tank
const int contrast = 100; // display contrast
const int hfg= 100; //height of a tank From Ground 
const float seoffset = 20; //sensor Offset distance
const int tankradius = 20; //radius of the tank
const int maxdepth = tankheight*9/10;  //the depth of water at which motor will stop fill
const int mindepth = tankheight/10;  //the depth of water at which motor will start again
const int buzzer = 13;

// Entry Points and Interrupt Handlers
long var_duration;
float var_lenthincm;
float lastValue;
String lastWord;
int realdepth,mappedDepth;
volatile int flow_frequency; // Measures flow sensor pulses
unsigned int l_hour = 0; // Calculated litres/hour
unsigned char flowsensor = 2; // Sensor Input
unsigned long currentTime;
unsigned long cloopTime;

byte container[8]={
    B10001,
    B10001,
    B10001,
    B10001,
    B10001,
    B10001,
    B10001,
    B11111
    };
byte container10[8]={
    B10001,
    B10001,
    B10001,
    B10001,
    B10001,
    B11001,
    B11111,
    B11111
    };


byte container25[8]={
    B10001,
    B10001,
    B10001,
    B10001,
    B10011,
    B11111,
    B11111,
    B11111
    };
    
byte container50[8]={
    B10001,
    B10001,
    B10001,
    B11001,
    B11111,
    B11111,
    B11111,
    B11111
    };
    
byte container65[8]={
    B10001,
    B10001,
    B10001,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111
    };
    
byte container75[8]={
    B10001,
    B10001,
    B11101,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111
    };
     

byte container99[8]={
    B10001,
    B10001,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111
    };
byte container100[8]={
    B10001,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111
    };


void flow () // Interrupt function
{
   flow_frequency++;
}

LiquidCrystal lcd(6,7,8,9,10,11,12);

void printResult(int a) {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("MOTOR:");
  lcd.setCursor(6,0);
  if(bitRead(PORTC,1)> 0){lcd.print("ON"); }else{lcd.print("OFF");}  //printing motor state
  lcd.setCursor(11, 0);
  lcd.print(map(a,0,tankheight,0,100));
  lcd.setCursor(14, 0);
  lcd.print("%");
  lcd.setCursor(15, 0);
  int ind = map(a,0,tankheight,0,7);
  lcd.write(byte(ind));
  lcd.setCursor(0,1);
  lcd.print(l_hour, DEC);
  lcd.setCursor(3,1);
  lcd.print("L/hr");
}

void startfilling() {

  lastWord = "startfill";
  digitalWrite(A1, HIGH);
  Serial.println(lastWord);
}

void stopfilling() {
  lastWord = "stopfill";
  digitalWrite(A1, LOW);
  Serial.println(lastWord);
  }
  
void updateDisplay(){
  
};

void setup () { 
 lcd.begin(16,2); //initialize the lcd 
 lcd.createChar(0,container);
 lcd.createChar(1,container10);
 lcd.createChar(2,container25);
 lcd.createChar(3,container50);
 lcd.createChar(4,container65);
 lcd.createChar(5,container75);
 lcd.createChar(6,container99);
 lcd.createChar(7,container100);
 
 pinMode(3, INPUT); // echo for ultrasonic sensor
 pinMode(2, INPUT);// flow rate
 pinMode(5, OUTPUT); /// display contrast
 pinMode(4, OUTPUT); // trigger ultrasonic sensor
 pinMode(13, OUTPUT); //buzzer
 pinMode(A1, OUTPUT);  // motor
 analogWrite(5, contrast); //setting contrast
 attachInterrupt(digitalPinToInterrupt(2),flow, RISING);
 lcd.setCursor(1,1);
 lcd.print("HI THERE!");
 
 Serial.begin(9600,0x06);
 
if(tankradius < 0.13165*(seoffset + tankheight)){
   Serial.println(" Please increase radius or reduce offset distance ");
 }

 }
void loop () { 
  // put your main code here, to run repeatedly:
  digitalWrite(4, true);
  delay(10);
  digitalWrite(4, false);
  var_duration = pulseIn(3, HIGH, 1000000);
  var_lenthincm = var_duration * 0.034 / 2;
  
  realdepth = seoffset + tankheight - var_lenthincm ;
  //Serial.println(realdepth);
  mappedDepth = constrain( realdepth, 0 , tankheight );
  //Serial.println(mappedDepth);
  int percentdepth = map(mappedDepth, 0, tankheight, 0, 100);
  //Serial.println(percentdepth);
  
  if (mappedDepth >  mindepth && mappedDepth < maxdepth) {
    if (lastValue != realdepth) {
      Serial.println("The depth is now :");
      Serial.print(realdepth);
      Serial.println("\nwhich is equal to :");
      Serial.print(percentdepth);
      Serial.print("%\n");   // shows water level in %

    }
  } else if (mappedDepth > maxdepth) {
    if (lastWord != "TankFull") {
      Serial.println("Water is full.");
      stopfilling();
      lastWord = "TankFull"; // if tank is full
      }
  } else if (mappedDepth < mindepth ) {
    if (lastWord != "TankEmpty") {
      Serial.println("Danger, tank is empty.");
      startfilling();
      lastWord = "TankEmpty"; //if tank is empty
    }
  } else {
    if (lastWord != "notank") {
      Serial.println("Please, put a tank");
      stopfilling();
      lastWord = "noTank";
    }
  }
     currentTime = millis();
   // Every second, calculate and print litres/hour
   if(currentTime >= (cloopTime + 1000))
   {
      cloopTime = currentTime; // Updates cloopTime
      // Pulse frequency (Hz) = 7.5Q, Q is flow rate in L/min.
      l_hour = (flow_frequency * 60 / 7.5); // (Pulse frequency x 60 min) / 7.5Q = flowrate in L/hour
      flow_frequency = 0; // Reset Counter
      Serial.println(l_hour); 
      Serial.print(" L/hour");
      Serial.println("");
   }
   if(l_hour <= 100 && bitRead(PORTC,1)> 0){
   digitalWrite(buzzer,HIGH);
   lcd.clear();
   lcd.setCursor(0, 1);
   lcd.print("leakage detected!");
   delay(1000);
   }
   else{
   digitalWrite(buzzer,LOW);
   }

delay(500);

printResult(mappedDepth);
}

