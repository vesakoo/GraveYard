#include <Stepper.h> 
#define DEBUG true
/**
 * GraveYard ver 0.0.1
 * Author: Vesa Kankkunen
 * Ver: 1.0.1
 * 
 * https://miro.com/app/board/uXjVOCew02A=/
 * 
 * 
*/
const int ANALOG_GROUND_LIMIT =500;
struct 
{
  /* data */
  const int DPIN_IN1_A =10;
  const int DPIN_IN2_B = 9;
  const int DPIN_ENABLER = 8;
  const int DPIN_ALHAALLA_BTN = 15;
  //Alienin moottori
  const int SPEED = 30;
  const int STEPS_TOTAL = 1000; //steps per round tod näk
  const int STEPS_UPDOWN = 900; // nosto- ja laskumatka kotipesältä
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
  const int DPIN_ALHAALLA_BTN = 14; //same as analog 0
  const int NOSTON_KESTO = 90000;
  
  bool resetDone =false;
  bool isDown = false;
  int buttonVal = 0;
} Hissi;

struct  {
  /* data */
  const int DPIN_KETJU = 7;
  //hissi lasku alas alkaa kun saattue on kulkenut arkun kanssa tämän ajan
  // (päässyt pois hissin päältä)
  const int KESTO_ARKKUN_OTTO_HISSISTA = 4000; 
  //saattue liikkuu tämän ajan kulkien arkun kanssa talolta haudalle.
  //jos hissi ei ole laskeutunut alas,
  //saattue pysähtyy odottamaan kunnes hissi on alhaalla (hissin alakytkin laukeaa) 
  const int ODOTUSAIKA_HISSIN_LASKULLE = 13000; 
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
  const int DPIN_IR = 3;//13;
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
  #ifdef DEBUG
    debug("alienAlas()");
  #endif
  digitalWrite(Alien.DPIN_ENABLER,HIGH);
  alienStepper.setSpeed(Alien.SPEED);
  alienStepper.step(-1 * Alien.STEPS_UPDOWN);//steppien pituus alienin laskuun
  digitalWrite(Alien.DPIN_ENABLER,LOW);
}

void alienYlos(){
  #ifdef DEBUG
    debug("alienYlos()");
  #endif 
  alienStepper.setSpeed(Alien.SPEED);
  digitalWrite(Alien.DPIN_ENABLER,HIGH);
  alienStepper.step(Alien.STEPS_UPDOWN); //steppien pituus alienin nostoon (=-laskupituus)
  digitalWrite(Alien.DPIN_ENABLER,LOW);
}

void alienReset(){
  #ifdef DEBUG
    debug("alienReset()");
    debug("DPIN_IN1_A: "+(String)Alien.DPIN_IN1_A);
    debug("DPIN_IN1_B: "+(String)Alien.DPIN_IN2_B);
    debug("Alien.APIN_ALHAALLA_BTN: "+ (String)Alien.DPIN_ALHAALLA_BTN);
  #endif
  digitalWrite(Alien.DPIN_ENABLER,HIGH);
  alienStepper.setSpeed(Alien.SPEED*.2);
  while (Alien.buttonVal  ==false )
  {
    alienStepper.step(-1 * Alien.STEPLEN_IN_RESET);
    Alien.buttonVal = digitalRead(Alien.DPIN_ALHAALLA_BTN);
    delay(1);
  }
  while (Alien.buttonVal == true )
  {
    alienStepper.step(Alien.STEPLEN_IN_RESET);
    Alien.buttonVal = digitalRead(Alien.DPIN_ALHAALLA_BTN);
    delay(1);
  }
  Alien.resetDone =true;
  digitalWrite(Alien.DPIN_ENABLER,LOW);
}


void motorRampUp(int from, int to, unsigned int pin){
  for (int i = from; i < to; i++)
  {
    analogWrite(pin,i);
   delay(10);
  }
}

void motorRampDown(int from, int to, unsigned int pin){
  for (int i = from; i > to; i--)
  {
    analogWrite(pin,i);
    delay(10);
  }
  if(to == 0)
    digitalWrite(pin,LOW);
}

/**
 * Hissi funcs
 * */
void hissiAlasWithSaattueStop(){
  #ifdef DEBUG
    debug("hissiAlasWithSaattueMonitor()");
  #endif
  long saattueTimerStart =micros();
  bool saattueWaitsForHissi = false;
  if(digitalRead(Hissi.DPIN_ALHAALLA_BTN)==false){
    motorRampUp(50,255,Hissi.DPIN_A);
  }
  while(digitalRead(Hissi.DPIN_ALHAALLA_BTN) ==false){
    long newTime = micros();
    if(newTime > saattueTimerStart + Saattue.ODOTUSAIKA_HISSIN_LASKULLE*1000 ){
      saattueSeis();
      saattueWaitsForHissi = true;
      #ifdef DEBUG
        debug("Saattue pysäytettiinn ennen hautaa koska hissi ei ole ehtinyt alas");
      #endif
    }
    delay(1);
  }
  hissiJarruta();
  //must have status && press down btn
  Hissi.isDown =true;
  if(saattueWaitsForHissi){
    #ifdef DEBUG
      debug("Hissi on laskeutunut, pysäytetyn saattueen tulee jatkaa matkaa");
    #endif
    saattueLiikuta();
  }

}

void hissiAlas(){
  #ifdef DEBUG
    debug("hissiAlas()");
  #endif
  if(digitalRead(Hissi.DPIN_ALHAALLA_BTN)==false){
    motorRampUp(50,255,Hissi.DPIN_A);
  }
  while(digitalRead(Hissi.DPIN_ALHAALLA_BTN) ==false){
    delay(1);
  }
  hissiJarruta();
  //must have status && press down btn
  Hissi.isDown =true;
}

void hissiYlos(){
  debug("hissiYlos()");
  Hissi.isDown =false;
  motorRampUp(50,255,Hissi.DPIN_B);
  delay(Hissi.NOSTON_KESTO);
  motorRampDown(250,0,Hissi.DPIN_B);
  //hissiJarruta();
}

void hissiJarruta(){
  digitalWrite(Hissi.DPIN_A,LOW);
  digitalWrite(Hissi.DPIN_B,LOW);
}

void hissiReset(){
  #ifdef DEBUG
    debug("hissiReset()");
  #endif
  hissiAlas();
  Hissi.isDown =true;
}

/**Ketju funcs*/
void saattueLiikuta(){
  #ifdef DEBUG
    debug("saattueLiikuta()");
  #endif
  digitalWrite(Saattue.DPIN_KETJU,HIGH); //NOTE! relay pin LOW == move
}

void saattueSeis(){
  #ifdef DEBUG
    debug("saattueSeis()");
  #endif
  digitalWrite(Saattue.DPIN_KETJU,LOW); //NOTE! relay pin HIGH == stop
}

/**
 * Move saattue from starting pos and stop there!! 
 */
void saattueReset(){
  #ifdef DEBUG
    debug("saattueReset()");
  #endif
  saattueLiikuta();
  if(readIR(Talo.DPIN_STARTPOS) ==false){
    while(readIR(Talo.DPIN_STARTPOS) ==false ){
      delay(1);
    }
  }
  saattueSeis();


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
  #ifdef DEBUG
    debug("hautajaiset()");
  #endif
  hissiYlos();
  //delay(Hissi.NOSTON_KESTO); 
  saattueLiikuta();
  delay(Saattue.KESTO_ARKKUN_OTTO_HISSISTA);
  hissiAlasWithSaattueStop();
  /*hissiAlas();
  delay(Saattue.ODOTUSAIKA_HISSIN_LASKULLE);
  if(Hissi.isDown ==false){
    #ifdef DEBUG
    debug("saattueSeis koska Hissi.isdown =" + (String)Hissi.isDown);
    #endif
    saattueSeis();
    do
    {
      delay(1);
    } while (Hissi.isDown);
    #ifdef DEBUG
      debug("Hissi on laskeutunut, pysäytetty saattue tulee jatkaa matkaa");
    #endif
    saattueLiikuta();
  }*/

  #ifdef DEBUG
    debug("odotellaan arkun putoamista hautaan...");
  #endif 
  while (Hauta.arkkuHautaan ==false )//!!!!!! tapahtuma!!!
  {
    //hauta ir voi jäädä huomaamatta!!!!!
    //varmistetaan lukemalla onko arkku jo hississä
    Hauta.arkkuHautaan = readIR(Hauta.DPIN_IR) || readIR(Hissi.DPIN_IR_ARKKU_HISSISSA);
    delay(1);
  }
  #ifdef DEBUG
    debug("arkku putosi hautaan, käynnistetään viive Alienin nostamiseksi...");
  #endif
  delay(Saattue.KESTO_HAUDALTA_ALIENILLE);
  alienYlos();
  saattueSeis();
  #ifdef DEBUG
    debug("käynnistetään viive alienin katsomiseksi");
  #endif
  delay(Saattue.KESTO_KATSOO_ALIENIA);
  alienAlas();
  #ifdef DEBUG
    debug("ootellaan hetki ja jatketaan matkaa talolle.");
  #endif
  delay(Saattue.KESTO_KATSOO_ALIENIA);
  //saattueSeis();
  //delay(Saattue.KESTO_ODOTTAA_PAIKALLAAN_ALIENIA);
  //alienYlos();
  //delay(Saattue.KESTO_KATSOO_ALIENIA);
  //alienAlas()
  // put your main code here, to run repeatedly:
  saattueLiikuta();
  while (readIR(Talo.DPIN_IR) == false)
  {
    delay(1);
  }
  #ifdef DEBUG
    debug("talon ir ilmoitti saattueen saapuneen taloon");
  #endif
  saattueSeis();
}


/**generig irda read*/
bool readIR(int pin){
  return digitalRead(pin)== LOW;
}

/** UTILS*/

  #ifdef DEBUG
void debug(String msg){
    Serial.println(msg);
}
  #endif

////// Arduino 'main' /////////

void setup() {
  // put your setup code here, to run once:
  #ifdef DEBUG
    Serial.begin(19200);
    debug("\n======== setup() ohjelma käynnistyy");
  #endif
  

  pinMode(Alien.DPIN_IN1_A,OUTPUT);
  pinMode(Alien.DPIN_IN2_B,OUTPUT);
  pinMode(Alien.DPIN_ENABLER,OUTPUT);
  pinMode(Alien.DPIN_ALHAALLA_BTN,INPUT);

  pinMode(Hissi.DPIN_A,OUTPUT);
  pinMode(Hissi.DPIN_B,OUTPUT);
  pinMode(Hissi.DPIN_IR_ARKKU_HISSISSA,INPUT);
  pinMode(Hissi.DPIN_ALHAALLA_BTN,INPUT);

  pinMode(Hauta.DPIN_IR,INPUT);
  
  pinMode(Talo.DPIN_IR, INPUT);
  pinMode(Talo.DPIN_STARTPOS,INPUT);
  

  pinMode(Hautajaiset.DPIN_ALOITA_BTN,INPUT);
  
  alienReset();
  hissiReset();
  saattueReset();
  

}

void loop() {
  #ifdef DEBUG
    debug("loop(), odota start-nappia");
  #endif
  
  //Älä aloita hautajaisia ennen kuin nappia on painettu
  debug("Hautajaiset.onkoAloitettu ==false");
  while (Hautajaiset.onkoAloitettu ==false)
  { 
    Hautajaiset.onkoAloitettu = digitalRead(Hautajaiset.DPIN_ALOITA_BTN) == HIGH;
    delay(1);
  }
  #ifdef DEBUG
    debug("DPIN_ALOITA_BTN=" +(String)Hautajaiset.onkoAloitettu);
    debug("odota onko arkku hississä -IR" );
  #endif
  //älä aloita hautajaisia ennen kuin Arkku on hississä (alussa hissi alhaalla)
  debug("Hissi.DPIN_IR_ARKKU_HISSISSA) ==false");
  while(readIR(Hissi.DPIN_IR_ARKKU_HISSISSA) ==false){
    delay(1);
  }
  #ifdef DEBUG
    debug("odota onko saattue aloistuspisteessä Talo.DPIN_STARTPOS" );
  #endif
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
