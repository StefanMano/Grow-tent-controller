#include <Arduino.h>
#include <SensirionI2CScd4x.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,20,4);
SensirionI2CScd4x scd4x;
#include <avr/interrupt.h>

#define PIN0 0
#define PIN1 1
#define PIN2 2
#define PIN3 3
#define PIN4 4
#define PIN5 5
#define PIN6 6
#define PIN7 7

uint16_t setCO2=1000;
float setTemp=16.0f;
float setRH=85.0f;
volatile bool menu_interrupt = false;
int16_t i=1002;
volatile bool setting = false;
ISR(INT0_vect)
{
  if(menu_interrupt){
    menu_interrupt = false;
    
  }
  else{
    menu_interrupt = true;
    i=1002;
   setting = false;
  }
}
/*
void printSerialNumber(uint16_t serial0, uint16_t serial1, uint16_t serial2) {
    Serial.print("Serial: 0x");
    printUint16Hex(serial0);
    printUint16Hex(serial1);
    printUint16Hex(serial2);
    Serial.println();
}
void printUint16Hex(uint16_t value) {
    Serial.print(value < 4096 ? "0" : "");
    Serial.print(value < 256 ? "0" : "");
    Serial.print(value < 16 ? "0" : "");
    Serial.print(value, HEX);
}
*/
void lcdprintmenu(bool setting, int16_t i){
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("Set CO2  = ");
  lcd.setCursor(11,1);
  lcd.print(setCO2);
  lcd.setCursor(16,1);
  lcd.print("ppm");
  
  lcd.setCursor(0,0);
  lcd.print("Set Temp = ");
  lcd.setCursor(12,0);
  lcd.print(setTemp);
  lcd.setCursor(16,0);
  lcd.print(" C");

  lcd.setCursor(0,2);
  lcd.print("Set Umid = ");
  lcd.setCursor(12,2);
  lcd.print(setRH);
  lcd.setCursor(16,2);
  lcd.print("%");

  lcd.setCursor(0,3);
  if (setting == false)
    lcd.print("Select V  Inchide ->");
  else
    lcd.print("Inapoi V  Inchide ->");

  lcd.setCursor(19,i%3);
  lcd.print("*");
}

void printlcd(uint16_t co2, float temperature, float humidity){
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("CO2 = ");
  lcd.setCursor(6,1);
  lcd.print(co2);
  lcd.setCursor(11,1);
  lcd.print("ppm");

  lcd.setCursor(0,0);
  lcd.print("Temperatura = ");
  lcd.setCursor(14,0);
  lcd.print(temperature);
  lcd.setCursor(18,0);
  lcd.print("C");

  lcd.setCursor(0,2);
  lcd.print("Umiditate = ");
  lcd.setCursor(12,2);
  lcd.print(humidity);
  lcd.setCursor(16,2);
  lcd.print("%");

  lcd.setCursor(0,3);
  lcd.print("Seteaza parametrii >");
}
void handleinterrupt(){

  
  lcdprintmenu(setting, i);

    //sa aiba butoanele de pe pin 4,5 desene de up/down
  if((PIND & (1<<PIN5)) == 0)
    i--;
  if((PIND & (1<<PIN4)) == 0)
    i++;
  delay(100);
  lcdprintmenu(setting, i);
  if((PIND & (1<<PIN3)) == 0)
  {
    setting = true;
    delay(1000);
    switch (i%3){
        case 1:
          lcdprintmenu(setting, i);
          while((PIND & (1<<PIN3)) == (1<<PIN3) && menu_interrupt){
            
            if((PIND & (1<<PIN5)) == 0){
                setCO2 += 10;
                lcdprintmenu(setting, i);
                delay(100);
            } 
            if((PIND & (1<<PIN4)) == 0){
                setCO2 -= 10;
                lcdprintmenu(setting, i);
                delay(100);
            }
          }
          break;

        case 0:
          lcdprintmenu(setting, i);
          while((PIND & (1<<PIN3)) == (1<<PIN3) && menu_interrupt){
            if((PIND & (1<<PIN5)) == 0){
              setTemp += 0.1;
              lcdprintmenu(setting, i);
              delay(100);
            }
            if((PIND & (1<<PIN4)) == 0){
              setTemp -= 0.1;
              lcdprintmenu(setting, i);
              delay(100);
            }
          }
          break;


        case 2:
          lcdprintmenu(setting, i);
          while((PIND & (1<<PIN3)) == (1<<PIN3) && menu_interrupt){
            if((PIND & (1<<PIN5)) == 0){
              setRH += 1;
              lcdprintmenu(setting, i);
              delay(100);
            }
            if((PIND & (1<<PIN4)) == 0){
              setRH -= 1;
              lcdprintmenu(setting, i);
              delay(100);
            }
          }
          break;

    }
    else
      setting = false; //merge sau fute mai incolo?

  }

    

    
}

void setup() {

  SREG |= 1<<SREG_I;
  //HI-LO INTERRUPT ON PIN 0
  EICRA = (0<<ISC11)|(0<<ISC10)|(1<<ISC01)|(0<<ISC00);
  EIMSK = (0<<INT1)|(1<<INT0);
  EIFR = (0<<INTF1)|(0<<INTF0);
  DDRD = 0x00;
  PORTD = 0xFF;//tre schimbat pe pinii care au butoane doar

  //outputs
  DDRB = 0xFF;
  PORTB = 0x00;
  // put your setup code here, to run once:
  Serial.begin(57600);       //Start serial com with the BT module (RX and TX pins   
  lcd.init();       //Start the LC communication
  lcd.backlight();  //Turn on backlight for LCD
  //init of lcd screen

  Wire.begin();

  uint16_t error;
  char errorMessage[256];

  scd4x.begin(Wire);

  // stop potentially previously started measurement
  error = scd4x.stopPeriodicMeasurement();
  if (error) {
      Serial.print("Error trying to execute stopPeriodicMeasurement(): ");
      errorToString(error, errorMessage, 256);
      Serial.println(errorMessage);
  }

  /*uint16_t serial0;
  uint16_t serial1;
  uint16_t serial2;
  error = scd4x.getSerialNumber(serial0, serial1, serial2);
  if (error) {
      Serial.print("Error trying to execute getSerialNumber(): ");
      errorToString(error, errorMessage, 256);
      Serial.println(errorMessage);
  } else {
      printSerialNumber(serial0, serial1, serial2);
  }
*/
  // Start Measurement
  error = scd4x.startPeriodicMeasurement();
  if (error) {
      Serial.print("Error trying to execute startPeriodicMeasurement(): ");
      errorToString(error, errorMessage, 256);
      Serial.println(errorMessage);
  }

  //Serial.println("Waiting for first measurement... (5 sec)");
}

void loop() {
  
if (menu_interrupt == false){
  uint16_t error;
  char errorMessage[256];

  delay(100);

  // Read Measurement & afisare
  uint16_t co2 = 0;
  float temperature = 0.0f;
  float humidity = 0.0f;
  bool isDataReady = false;

  error = scd4x.getDataReadyFlag(isDataReady);
  if (error) {
      Serial.print("Error trying to execute getDataReadyFlag(): ");
      errorToString(error, errorMessage, 256);
      Serial.println(errorMessage);
      return;
  }
  if (!isDataReady) {
      return;
  }



  error = scd4x.readMeasurement(co2, temperature, humidity);
  if (error) {
      Serial.print("Error trying to execute readMeasurement(): ");
      errorToString(error, errorMessage, 256);
      Serial.println(errorMessage);
  } else if (co2 == 0) {
      Serial.println("Invalid sample detected, skipping.");
  } 
  else {
    printlcd(co2,temperature,humidity);
    //FAE
    if(co2>setCO2+200)
      PORTB|=1<<PIN0;
    else if(co2<setCO2-100)
      PORTB &= ~(1<<PIN0);
    // temp daca merge aerul
    if((PINB & (1<<PIN0)) == (1<<PIN0)){
      //cald
      if(temperature<setTemp-1)
        PORTB|=1<<PIN1;
      else if(temperature>setTemp+1)
        PORTB &= ~(1<<PIN1);
      
      //rece
      if(temperature>setTemp+2)
        PORTB|=1<<PIN2;
      else if(temperature<setTemp)
        PORTB &= ~(1<<PIN2);
      }
      //daca nu merge oprim tot 
    else{
      PORTB &= ~(1<<PIN1);
      PORTB &= ~(1<<PIN2);
    }

    if(humidity<setRH-10)
      PORTB|=(1<<PIN3);
    else if (humidity>setRH+5)
      PORTB &= ~(1<<PIN3);
  }
  }
  else{
    handleinterrupt();
  }
}
