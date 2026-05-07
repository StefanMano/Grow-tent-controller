#include <EEPROM.h>
void setup() {
int EEPROM_CO2 = 0;
int EEPROM_umid = 8;
int EEPROM_temp = 16;
EEPROM.put(EEPROM_CO2,700);
EEPROM.put(EEPROM_umid,91.0);
EEPROM.put(EEPROM_temp,17.5);

}

void loop() {
  // put your main code here, to run repeatedly:

}
