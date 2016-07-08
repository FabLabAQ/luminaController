#include <SoftTimer.h>
#include <SoftPwmTask.h>
#include <DelayRun.h>
#include <BlinkTask.h>

//#define _USE_SOFT_PWM_

#define SENSORE_A1 18
#define SENSORE_A2 19
#define SENSORE_B  20
#define SENSORE_C  21


SoftPwmTask led2(50);
SoftPwmTask led4(51);
SoftPwmTask led6(48);
SoftPwmTask led8(49);
SoftPwmTask led10(47);

byte scenario=1;

void pwm(byte pin, byte value) {
switch (pin) {
  case 1:
    analogWrite(13, value);
    break;
  case 2:
    led2.analogWrite(value);
    break;
  case 3:
    analogWrite(12, value);
    break;
  case 4:
    led4.analogWrite(value);
    break;
  case 5:
    analogWrite(11, value);
    break;
  case 6:
    led6.analogWrite(value);
    break;
  case 7:
    analogWrite(10, value);
    break;
  case 8:
    led8.analogWrite(value);
    break;
  case 9:
    analogWrite(9, value);
    break;
  case 10:
    led10.analogWrite(value);
    break;
  case 11:
    analogWrite(8, value);
    break;
  case 12:
    analogWrite(46, value);
    break;
  case 13:
    analogWrite(7, value);
    break;
  case 14:
    analogWrite(44, value);
    break;
  case 15:
    analogWrite(6, value);
    break;
  case 16:
    analogWrite(45, value);
    break;
  case 17:
    analogWrite(5, value);
    break;
  case 18:
    analogWrite(4, value);
    break;
  case 19:
    analogWrite(3, value);
    break;
  case 20:
    analogWrite(2, value);
    break;
  }  

}

#define NUMERO_PROIETTORI 8
#define MIN_PWM 1   // LIVELLO MINIMO DI LUMINOSITÀ
#define MAX_PWM 255 // LIVELLO MASSIMO DI LUMINOSITÀ
#define DOWN_SLOPE -1
#define UP_SLOPE 2
#define SPEED 5
typedef struct Faretto{
  int value;
  int slope;
  byte slopeLen;
  byte plainLen;
};
byte dir[2] = {1, -1};
Faretto arrFaretti[NUMERO_PROIETTORI];


////// SCENARIO 2 SMOOTH //////

void proiettori_smooth(Task* me) {
  for (byte i=0; i<NUMERO_PROIETTORI; i++){
    
    if (arrFaretti[i].slopeLen != 0) {
      arrFaretti[i].value += arrFaretti[i].slope;
      arrFaretti[i].slopeLen--;
      if (arrFaretti[i].value > MAX_PWM) {
        arrFaretti[i].value = MAX_PWM;  // trim lower values
      } else if (arrFaretti[i].value < MIN_PWM) {
        arrFaretti[i].value = MIN_PWM; // trim upper values
      };
    } else if (arrFaretti[i].plainLen != 0) {
      arrFaretti[i].plainLen--;
    } else {
      arrFaretti[i].slope = random(DOWN_SLOPE, UP_SLOPE); // pendenza fronti salita/discesa
      arrFaretti[i].slopeLen = random(20, 60);            // durata fronti salita/discesa
      arrFaretti[i].plainLen = random(20, 20);            // durata parte in piano
    }
//  Serial.print(String(arrFaretti[i].value) + " " + arrFaretti[i].slope + " " + arrFaretti[i].slopeLen + " " + arrFaretti[i].plainLen + " ");
//  Serial.print(String(arrFaretti[i].value) + " ");
  pwm(i*2+1, arrFaretti[i].value);
  }
  Serial.println();
}
Task tskProiettori_smooth(SPEED, proiettori_smooth);


////// SCENARIO 2 ROUGH //////

void lampadine_rough(Task* me) {
  for (byte i=0; i < NUMERO_PROIETTORI; i++){
      pwm(i*2+1, random(256));
   } 
}
Task tskLampadine_rough(80, lampadine_rough);


////// CONTROLLO MUSIC PLAYER //////
// This task "press" play button of a MP3 player using a 
// digital output connected to play/pause button of the player.
// It press play only after the music duration time. 
#define PLAY_BUTTON_PIN 24
#define MUSIC_DURATION  29 // in seconds
// -- On for 300ms off for 200ms, repeat it 1 times, sleep for MUSIC_DURATION seconds.
BlinkTask tskMusicPlayer(PLAY_BUTTON_PIN, 300, 200, 1, MUSIC_DURATION*1000);

////// TASK PER IL RITORNO A TEMPO ALLO SCENARIO 1 //////
boolean scenario1(Task* task){ scenario = 1; };
DelayRun tskScenario1(0, scenario1);


////// MAIN CONTROLLER //////

void mainController(Task* me) {
  switch (scenario) {
    case 0:
      break;
    case 1:                                    // SCENARIO 1
      SoftTimer.remove(&tskProiettori_smooth); // Ferma effetto "smooth" su proiettori
      lampadine(0);                            // Lampadine OFF
      proiettori(128);                         // Proiettori 50%
      enableSensors();                         // Abilita i sensori di cambio scenario
      tskMusicPlayer.stop();                   // Stop music loop
      scenario = 0;                            // Nessuna azione del controller
      break;
    case 2:                                    // SCENARIO 2
      disableSensors();                        // Disabilita i sensori di cambio scenario
      tskMusicPlayer.start();                  // Start music loop
      SoftTimer.add(&tskProiettori_smooth);    // Avvia effetto "smooth" su proiettori
      lampadine(0);                            // Lampadine OFF
      tskScenario1.delayMs = 30000;            // Durata scenario 2 in millisecondi
      tskScenario1.startDelayed();
      break;
    case 3:                                    // SCENARIO 3
      disableSensors();                        // Disabilita i sensori di cambio scenario
      tskMusicPlayer.stop();                   // Stop music loop
      SoftTimer.add(&tskProiettori_smooth);    // Avvia effetto "smooth" su proiettori
      lampadine(50);                           // Lampadine 20%
      tskScenario1.delayMs = 5000;             // Durata scenario 3 in millisecondi
      tskScenario1.startDelayed();
      break;
    case 4:                                    // SCENARIO 4
      disableSensors();                        // Disabilita i sensori di cambio scenario
      tskMusicPlayer.stop();                   // Stop music loop
      SoftTimer.remove(&tskProiettori_smooth); // Ferma effetto "smooth" su proiettori
      proiettori(12);                          // Proiettori 5%
      lampadine(255);                          // Lampadine 100%
      tskScenario1.delayMs = 5000;             // Durata scenario 4 in millisecondi
      tskScenario1.startDelayed();
      break;
  }
}
Task tskMainController(300, mainController);

#define NUMERO_LAMPADINE 10
void lampadine(byte v) {
  for (byte i=0; i < NUMERO_LAMPADINE; i++){
      pwm(i*2+2, v);
  }
  pwm(17, v);
  pwm(19, v);
}

void proiettori(byte v) {
  for (byte i=0; i < NUMERO_PROIETTORI; i++){
      pwm(i*2+1, v);
   } 
}

void ISR_scenario2(){ scenario = 2; }
void ISR_scenario3(){ scenario = 3; }
void ISR_scenario4(){ scenario = 4; }

void enableSensors(){
  attachInterrupt( digitalPinToInterrupt(SENSORE_A1), ISR_scenario2, CHANGE );
  attachInterrupt( digitalPinToInterrupt(SENSORE_A2), ISR_scenario2, CHANGE );
  attachInterrupt( digitalPinToInterrupt(SENSORE_B),  ISR_scenario3, CHANGE );
  attachInterrupt( digitalPinToInterrupt(SENSORE_C),  ISR_scenario4, CHANGE );
}

void disableSensors(){
  detachInterrupt( digitalPinToInterrupt(SENSORE_A1));
  detachInterrupt( digitalPinToInterrupt(SENSORE_A2));
  detachInterrupt( digitalPinToInterrupt(SENSORE_B));
  detachInterrupt( digitalPinToInterrupt(SENSORE_C));
}

void setup() {
//  Serial.begin(115200);

pinMode(PLAY_BUTTON_PIN, OUTPUT);

pinMode(SENSORE_A1, INPUT_PULLUP);
pinMode(SENSORE_A2, INPUT_PULLUP);
pinMode(SENSORE_B,  INPUT_PULLUP);
pinMode(SENSORE_C,  INPUT_PULLUP);

#if defined (_USE_SOFT_PWM_)
  SoftTimer.add(&led2);
  SoftTimer.add(&led4);
  SoftTimer.add(&led6);
  SoftTimer.add(&led8);
  SoftTimer.add(&led10);
#endif

  SoftTimer.add(&tskMainController);
}

