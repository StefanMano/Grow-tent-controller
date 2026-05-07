


#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,16,2);

const byte AC = 2, cald = 3, apa = 4, aer = 5;
byte contor = 0, val_timer_apa = 0;
unsigned int timer = 0,contor_aer = 0,timer_fae_aux=0, timer_fae = 0, ac_fae = 0;


void setup()
{
  Serial.begin(9600);
  
  lcd.init();       //Start the LC communication
  lcd.backlight();  //Turn on backlight for LCD
  
  DDRD = (0<<AC)|(0<<cald)|(1<<apa)|(1<<aer); 			//seteaza AC si caldura ca input, apa si aer ca output
  PORTD = 0x00; 										//toate outputurile sunt 0 si pull up e oprit
  SREG |= (1<<SREG_I);									//permite intreruperi
  EICRA = (0<<ISC11)|(1<<ISC10)|(0<<ISC01)|(1<<ISC00);	//se presupune ca toate sistemele de ac/cald sunt oprite si se va porni prima intrerupere cand porneste unul
  EIMSK = (1<<INT0)|(1<<INT1);							//foloseste ambii pini de intreruperi extrene
  
  TCCR0A = (0<<WGM00)|(0<<WGM01); //seteaza mod CTC
  TCCR0B = (0<<WGM02)|(1<<CS02)|(0<<CS01)|(0<<CS00); // mod CTC si prescalar 64(e posibil sa trebuiasca 1024 daca nu am inteles bien fisa si merge la 16 MHz sau altceva la 8MHz)
  OCR0A = 0xFA ;
  TIMSK0 = 1<<OCIE0A; 
  SREG |= (1<<SREG_I);									//permite intreruperi

}

void loop()
{
  //Serial.print(timer);
  //Serial.print("; ");
  //Serial.println(val_timer_apa+ac_fae);
  
  if(val_timer_apa !=0){
    if(ac_fae+val_timer_apa<=timer){
    	val_timer_apa=0;
      	PORTD &= ~(1<<apa);
    }
  }
}


//contor secunde
ISR (TIMER0_COMPA_vect){
  SREG &= ~(1<<SREG_I);
  ++contor;
  if(contor == 244){
  	contor = 0;
    ++timer;
  }
    
  SREG |= (1<<SREG_I);	 
}

//schimbare stare AC
ISR(INT0_vect){
  SREG &= ~(1<<SREG_I);
  if(PIND & (1<<AC)){		//cand e low (s-a oprit AC)  
    PORTD |= 1<<apa;
    timer_fae_aux = timer;
    ++contor_aer;
  }
  else{						//cand e high (s-a pornit AC)
    val_timer_apa = 15;
    timer_fae = timer-timer_fae_aux;
    timer_fae_aux = 0;
   	ac_fae = timer;
  }
  SREG |= (1<<SREG_I);	
}


//schimbare stare cald
ISR(INT1_vect){  
  SREG &= ~(1<<SREG_I);
  if(PIND & (1<<cald)){		//cand e low (s-a oprit aerul cald)
    PORTD |= 1<<apa;
    timer_fae_aux = timer;
    ++contor_aer;
  }
  else{						//cand e high (s-a pornit aerul cald)
    val_timer_apa = 15;
    timer_fae = timer-timer_fae_aux;
    timer_fae_aux = 0;
   	ac_fae = timer;
  }
  SREG |= (1<<SREG_I);
}	