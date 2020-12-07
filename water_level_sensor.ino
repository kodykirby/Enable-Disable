#include <Arduino.h>
#include <LiquidCrystal.h>
#include <Wire.h>

volatile unsigned char *myUCSR0A = (unsigned char *) 0x00C0;
volatile unsigned char *myUCSR0B = (unsigned char *) 0x00C1;
volatile unsigned char *myUCSR0C = (unsigned char *) 0x00C2;
volatile unsigned char *myUBRR0 = (unsigned char *) 0x00C4;
volatile unsigned char *myUDR0 = (unsigned char *) 0x00C6;

volatile unsigned char *port_f = (unsigned char *) 0x31;
volatile unsigned char *ddr_f = (unsigned char *) 0x30;
volatile unsigned char *port_b = (unsigned char *) 0x25;
volatile unsigned char *ddr_b = (unsigned char *) 0x24;

volatile unsigned char *myADMUX = (unsigned char *) 0x7C;
volatile unsigned char *myADCSRB = (unsigned char *) 0x7B;
volatile unsigned char *myADCSRA = (unsigned char *) 0x7A;
volatile unsigned char *myADC_DATA = (unsigned char *) 0x78;

volatile unsigned char *myTCCR1A = (unsigned char *) 0x80;
volatile unsigned char *myTCCR1B = (unsigned char *) 0x81;
volatile unsigned char *myTCNT1 = (unsigned char *) 0x84;
volatile unsigned char *myOCR1A = (unsigned char *) 0x88;
volatile unsigned char *myTIMSK1 = (unsigned char *) 0x6F;

int reading = 0;
const int rs = 11, en = 10, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

#define sensorPin 0
#define RDA 0x80

ISR(TIMER1_COMPA_vect){
  *port_b |= 0x80; //turn on adc0
  delay(10);
  reading = analogRead(0);
  if(reading < 200){
    lcd.print("ERROR");
    *port_f |= 0b00010000; //turns on red led to indicate err
  }
  *port_b &= 0x7F; //turn off adc0
}

void setup() {
  //port setup, can change once implemented into proj
  *ddr_f &= 0x00; //port f is all inputs
  *ddr_b |= 0xFF; //port b is all outputs
  *port_b &= 0x00; //all pins in port b are set low
  
  //uart and adc setup
  U0init(9600); //set up the uart
  adc_init; //set up the adc

  //timer setup here
  TCCR1A = 0; // clears TCCR1A register
  TCCR1B = 0; // clears TCCR1B register
  TCNT1  = 0; //sets counter value to 0
 
  OCR1A = 15624; //sets for 1hz increments
  TCCR1B |= 0b00001000; //turns on ctc mode
  TCCR1B |= 0b00000101; //sets prescaler of 1024
  TIMSK1 |= 0b00000010; //enables timer compare interrupt

  //enables interrupts
  sei();

  //lcd setup
  lcd.begin(16,2);
}

void loop() {
  // put your main code here, to run repeatedly:
  
}

void adc_init(){
  *myADCSRA |= 0b10000000;
  *myADCSRA &= 0b11011111;
  *myADCSRA &= 0b11110111;
  *myADCSRA &= 0b11111000;

  *myADCSRB &= 0b11110111;
  *myADCSRB &= 0b11111000;

  *myADMUX &= 0b01111111;
  *myADMUX |= 0b01000000;
  *myADMUX &= 0b11011111;
  *myADMUX &= 0b11011111;
  *myADMUX &= 0b11100000;
}

unsigned int adc_read(unsigned char adc_channel){
  *myADMUX &= 0xE0;
  *myADCSRB &= 0xFF;

  *myADMUX |= adc_channel;
  
  *myADCSRA |= 0x40;
   while((*myADCSRA & 0x40) != 0);

  return *myADC_DATA;
}

void U0init(unsigned long U0baud){
  unsigned long FCPU = 16000000;
  unsigned int tbaud;
  tbaud = (FCPU / 16 / U0baud -1);
  *myUCSR0A = 0x20;
  *myUCSR0B = 0x18;
  *myUCSR0C = 0x06;
  *myUBRR0  = tbaud;
}
