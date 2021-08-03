
// Date and time functions using a DS1307 RTC connected via I2C and Wire lib
#include "RTClib.h"

//DCF77
#include <Arduino.h>
#include "decodeurDCF77.h"


RTC_DS1307 rtc;

const uint8_t PIN_DCF77 = 3;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

int RST_PIN = 8;
int RELAY_PIN = 9;

int cmd = 0;
int myPins[] = {22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47};


void setup () {

  //reset pin 
  pinMode(RST_PIN, INPUT);
  digitalWrite(RST_PIN,LOW);
  //Relay control
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN,HIGH);

  // put your setup code here, to run once:
  for(int i = 0; i < (sizeof(myPins) / sizeof(myPins[0]));i++){
    pinMode(myPins[i], OUTPUT);    // sets the digital pin 13 as output
  }

  pinMode(PIN_DCF77, INPUT);
  Serial.begin(57600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
#ifndef ESP8266
  while (!Serial); // wait for serial port to connect. Needed for native USB
#endif

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    digitalWrite(RELAY_PIN,LOW);
    delay(2000);
    digitalWrite(RELAY_PIN,HIGH);
    delay(1000);
    digitalWrite(RST_PIN,HIGH);
    while(1);
  }
  Serial.println("RTC found");


  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    rtc.adjust(DateTime(2021, 1, 1, 0, 0, 0));

  }

  // When time needs to be re-set on a previously configured device, the
  // following line sets the RTC to the date & time this sketch was compiled
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // This line sets the RTC with an explicit date & time, for example to set
  // January 21, 2014 at 3am you would call:
  //rtc.adjust(DateTime(2021, 1, 21, 3, 0, 0));

  Serial.println("Demo decodage signal DCF77\n");
  Serial.println("Recherche pulsation...");
  bitPrint(generateInteger());
  displayTime();

}

void loop () {

  static uint8_t longueur = 0;

  bool trame_decodee = decodeurDCF77.traiterSignal(digitalRead(PIN_DCF77), millis());

  if (trame_decodee) {
    Serial.print(' ');
    Serial_printDCF77();
  }

  if (longueur > decodeurDCF77.longueur_trame_en_cours()) {
    longueur = 0;
    Serial.println();
  }

  while (longueur < decodeurDCF77.longueur_trame_en_cours()) {
    Serial.print(decodeurDCF77.bit_trame(longueur++));
    //displayTime();
  }

  bitPrint(generateInteger());

}

void displayTime() {

  Serial.println();
  DateTime now = rtc.now();
  Serial.print(now.year(), DEC);
  Serial.print('/');
  if (now.month() < 10) {
    Serial.print("0");
  }
  Serial.print(now.month(), DEC);
  Serial.print('/');
  if (now.day() < 10) {
    Serial.print("0");
  }
  Serial.print(now.day(), DEC);
  Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
  if (now.hour() < 10) {
    Serial.print("0");
  }
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  if (now.minute() < 10) {
    Serial.print("0");
  }
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  if (now.second() < 10) {
    Serial.print("0");
  }
  Serial.print(now.second(), DEC);
}

void Serial_printDCF77() {
  switch (decodeurDCF77.joursem()) {
    case 0 : Serial.println("(vide)"); return;
    case 1 : Serial.print("Lundi"); break;
    case 2 : Serial.print("Mardi"); break;
    case 3 : Serial.print("Mercredi"); break;
    case 4 : Serial.print("Jeudi"); break;
    case 5 : Serial.print("Vendredi"); break;
    case 6 : Serial.print("Samedi"); break;
    case 7 : Serial.print("Dimanche"); break;
  }
  Serial.print(' ');
  Serial_print99(decodeurDCF77.jour());
  Serial.print('/');
  Serial_print99(decodeurDCF77.mois());
  Serial.print("/20");
  Serial_print99(decodeurDCF77.annee());
  Serial.print(' ');
  Serial_print99(decodeurDCF77.heure());
  Serial.print(':');
  Serial_print99(decodeurDCF77.minute());
  Serial.print(' ');
  if (decodeurDCF77.heure_ete()) {
    Serial.print("(heure d'ete)");
  } else {
    Serial.print("(heure d'hiver)");
  }

  DateTime now = rtc.now();
  DateTime dcf77 = DateTime(decodeurDCF77.annee(), decodeurDCF77.mois(), decodeurDCF77.jour(), decodeurDCF77.heure(), decodeurDCF77.minute(), 0);

  if (abs(now.unixtime() - dcf77.unixtime() > 5)) {
    rtc.adjust(dcf77);
    Serial.println("\nNew RTC time");
  } else {
    Serial.println("\nRTC up to date");
  }

}

void Serial_print99(uint8_t nombre) {
  if (nombre < 10) Serial.print('0');
  Serial.print(nombre);
}

int generateInteger() {
  DateTime now = rtc.now();
  int hh = now.hour();
  int mm = now.minute();
  int time = 0;
  switch (mm) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
      time = time + 0;
      break;
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
      time = time + 16777216;
      break;
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
      time = time + 524288;
      break;
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
      time = time + 3145728;
      break;
    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
      time = time + 4194304;
      break;
    case 25:
    case 26:
    case 27:
    case 28:
    case 29:
      time = time + 29360128;
      break;
    case 30:
    case 31:
    case 32:
    case 33:
    case 34:
      //hh = (hh + 1) % 24;
      time = time + 34603008;
      break;
    case 35:
    case 36:
    case 37:
    case 38:
    case 39:
      hh = (hh + 1) % 24;
      time = time + 29622272;
      break;
    case 40:
    case 41:
    case 42:
    case 43:
    case 44:
      hh = (hh + 1) % 24;
      time = time + 4456448;
      break;
    case 45:
    case 46:
    case 47:
    case 48:
    case 49:
      hh = (hh + 1) % 24;
      time = time + 2359296;
      break;
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
      hh = (hh + 1) % 24;
      time = time + 786432;
      break;
    case 55:
    case 56:
    case 57:
    case 58:
    case 59:
      hh = (hh + 1) % 24;
      time = time + 17039360;
      break;
  }
  switch (hh) {
    case 0:
      time = time + 16387;
      break;
    case 1:
    case 13:
      time = time + 65603;
      break;
    case 2:
    case 14:
      time = time + 196615;
      break;
    case 3:
    case 15:
      time = time + 196627;
      break;
    case 4:
    case 16:
      time = time + 196619;
      break;
    case 5:
    case 17:
      time = time + 197635;
      break;
    case 6:
    case 18:
      time = time + 197123;
      break;
    case 7:
    case 19:
      time = time + 196739;
      break;
    case 8:
    case 20:
      time = time + 196867;
      break;
    case 9:
    case 21:
      time = time + 196643;
      break;
    case 10:
    case 22:
      time = time + 208899;
      break;
    case 11:
    case 23:
      time = time + 229379;
      break;
    case 12:
      time = time + 6147;
      break;
  }
  
  return time;

}

void bitPrint(int cmd)
{
  //Serial.print("0b");
  for (int i = 0; i < (sizeof(myPins) / sizeof(myPins[0])); i++)
  {
    if(bitRead(cmd, i) == 1){
        //Serial.print("1");
        digitalWrite(myPins[i], HIGH);
    }else{
        //Serial.print("0");
        digitalWrite(myPins[i], LOW);
    }
  }
  //Serial.println();
}
