#include <Stepper.h>
#define DEBUG true
/**
 * GraveYard ver 0.0.1
 * Author: Vesa Kankkunen
 * Date: 2022-11-19 
 * Ver: 0.0.1
*/

struct 
{
  /* data */
  const int DPIN_IN1_A =9;
  const int DPIN_IN2_B = 10;
  const int DPIN_ENABLER = 8;
  const int APIN_ALHAALLA_BTN = 1;
  //Alienin moottori
  const int SPEED = 30;
  const int STEPS_TOTAL = 100;
  const int STEPS_UPDOWN = 80;
  const int STEPLEN_IN_RESET = 1;
  
  bool resetDone =false;
  int buttonVal = 0;
} Alien;

Stepper alienStepper(
  Alien.STEPS_TOTAL,
  Alien.DPIN_IN1_A,
  Alien.DPIN_IN2_B); 

struct {
  /* data */
  const int DPIN_IR_ARKKU_HISSISSA = 12;
  const int DPIN_A = 6;
  const int DPIN_B = 5;
  const int APIN_ALHAALLA_BTN = 0;
  const int NOSTON_KESTO = 5000;
  
  bool resetDone =false;
  bool isDown = false;
  int buttonVal = 0;
} Hissi;

struct  {
  /* data */
  const int DPIN_KETJU = 7;
  const int KESTO_ARKKUN_OTTO_HISSISTA = 2000;
  const int ODOTUSAIKA_HISSIN_LASKULLE = 2000;
  const int KESTO_HAUDALTA_ALIENILLE = 5000;
  const int KESTO_ODOTTAA_PAIKALLAAN_ALIENIA = 2000;
  const int KESTO_KATSOO_ALIENIA = 3000;
  int lastMeasuredPos =0;
  bool resetDone =false;
} Saattue;

struct {
  const int DPIN_IR =11;
  bool arkkuHautaan = false;
} Hauta;
struct 
{
  /* data */
  const int DPIN_IR = 13;
  const int DPIN_STARTPOS = 3;
} Talo;

struct 
{
  /* data */
  const int DPIN_ALOITA_BTN = 4;
  bool onkoAloitettu = false; 
} Hautajaiset;



/** Alien funcs*/

void alienAlas(){
  alienStepper.step(-1*Alien.STEPS_UPDOWN);//steppien pituus alienin laskuun
}

void alienYlos(){
  alienStepper.step(Alien.STEPS_UPDOWN); //steppien pituus alienin nostoon (=-laskupituus)
}

void alienReset(){
  debug("alienReset()");
  alienStepper.setSpeed(Alien.SPEED);
  while (Alien.buttonVal == 0)
  {
    alienStepper.step(-1 * Alien.STEPLEN_IN_RESET);
    Alien.buttonVal = analogRead(Alien.APIN_ALHAALLA_BTN);
    delay(1);
  }
  while (Alien.buttonVal > 0)
  {
    alienStepper.step(Alien.STEPLEN_IN_RESET);
    Alien.buttonVal = analogRead(Alien.APIN_ALHAALLA_BTN);
    delay(1);
  }
  Alien.resetDone =true;
}


/**
 * Hissi funcs
 * @TODO
 * */
void hissiAlas(){
  debug("hissiAlas()");
  //must have status && press down btn
  Hissi.isDown =true;
}
void hissiYlos(){
  debug("hissiYlos()");
  Hissi.isDown =false;  

}
void hissiReset(){
  debug("hissiReset()");
  Hissi.isDown =true;
}
/**Ketju funcs*/
void saattueLiikuta(){
  debug("saattueLiikuta()");
  digitalWrite(Saattue.DPIN_KETJU,HIGH);
}

void saattueSeis(){
  digitalWrite(Saattue.DPIN_KETJU,LOW);
}
void saattueReset(){
  saattueLiikuta();
  while(readIR(Talo.DPIN_STARTPOS) ==false ){
    saattueSeis();
  }

  Saattue.resetDone =true;
}

/**
 * Hautajaiset-sekvenssi
 * hissi ylös
 * saattue liikkkeelle && hissin lasku alas
 * saattue pysähtyy, jos hissi ei kuittaa olevansa alhaalla ennen kuin saattue on haudalla
 * arkku pudotetaan hautaan, saattue jatkaa mataa
 * hetken kuluttua alien nostetaan ylös 
 * saattue pysähtyy
 * alienia pidetään hetki ylhäällä
 * alien laksetaan alas
 * saattue jatkaa matkaansa kunnes
 * saapuu talolle
 * kierros päättyy
*/
void hautajaiset (){
  hissiYlos();
  delay(Hissi.NOSTON_KESTO); 
  saattueLiikuta();
  delay(Saattue.KESTO_ARKKUN_OTTO_HISSISTA);
  hissiAlas();
  delay(Saattue.ODOTUSAIKA_HISSIN_LASKULLE);
  if(Hissi.isDown ==false){
    saattueSeis();
    do
    {
      delay(1);
    } while (Hissi.isDown);
    saattueLiikuta();
  }
  while (Hauta.arkkuHautaan = readIR(Hauta.DPIN_IR) )//!!!!!! tapahtuma!!!
  {
    delay(1);
  }
  delay(Saattue.KESTO_HAUDALTA_ALIENILLE);
  alienYlos();
  saattueSeis();
  delay(Saattue.KESTO_KATSOO_ALIENIA);
  alienAlas();
  delay(Saattue.KESTO_KATSOO_ALIENIA);
  //saattueSeis();
  //delay(Saattue.KESTO_ODOTTAA_PAIKALLAAN_ALIENIA);
  //alienYlos();
  //delay(Saattue.KESTO_KATSOO_ALIENIA);
  //alienAlas()
  // put your main code here, to run repeatedly:
  saattueLiikuta();
  while (readIR(Talo.DPIN_IR))
  {
    delay(1);
  }
  saattueSeis();
}


/**generig irda read*/
bool readIR(int pin){
  return digitalRead(pin)== HIGH;
}

/** UTILS*/
void debug(String msg){
  if(DEBUG)
    Serial.println(msg);
}

////// Arduino 'main' /////////

void setup() {
  // put your setup code here, to run once:
  if(DEBUG){
    Serial.begin(9600);
  }

  pinMode(Alien.DPIN_IN1_A,OUTPUT);
  pinMode(Alien.DPIN_IN2_B,OUTPUT);
  pinMode(Alien.DPIN_ENABLER,OUTPUT);

  pinMode(Hissi.DPIN_A,OUTPUT);
  pinMode(Hissi.DPIN_B,OUTPUT);
  pinMode(Hissi.DPIN_IR_ARKKU_HISSISSA,INPUT);

  pinMode(Hauta.DPIN_IR,INPUT);
  
  pinMode(Talo.DPIN_IR, INPUT);
  pinMode(Talo.DPIN_STARTPOS,INPUT);
  

  pinMode(Hautajaiset.DPIN_ALOITA_BTN,INPUT);
  
  alienReset();
  hissiReset();
  saattueReset();
  

}

void loop() {
  debug("loop()");
  //Älä aloita hautajaisia ennen kuin nappia on painettu
  while (Hautajaiset.onkoAloitettu ==false)
  {
    Hautajaiset.onkoAloitettu = digitalRead(Hautajaiset.DPIN_ALOITA_BTN) == HIGH;
    delay(1);
  }
  //älä aloita hautajaisia ennen kuin Arkku on hississä (alussa hissi alhaalla)
  while(readIR(Hissi.DPIN_IR_ARKKU_HISSISSA) ==false){
    delay(1);
  }
  //älä aloita hautajaisia jos saattue ei ole aloituskohdassa
  while (readIR(Talo.DPIN_STARTPOS) ==false)
  {
    delay(1);
  }
  hautajaiset();
  //hautajaiset ovat päättyneet (taloon)
  Hautajaiset.onkoAloitettu =false;

  
  //wait untill start btn
  //wait untill arkku hississa
  

}
