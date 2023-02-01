#include <Arduino.h>
#define CAYENNE_PRINT Serial     
#include <CayenneMQTTEthernet.h>
#include <Servo.h>
// ********//
// decalarations des instance de servvo moteur
Servo verti_servo;
Servo hori_servo;

char username[] = "9f4e0bf0-8137-11ed-b193-d9789b2af62b";
char password[] = "288e236b1ab2a8faa0730f036ec42e2f976f530a";
char clientID[] = "d7b34a90-9aab-11ed-b193-d9789b2af62b";

// replacer des nemro des PINs par des variables siginifique
#define ldrtopr A0    // top-right LDR
#define ldrtopl A1    // top-left LDR
#define ldrbotr A2    // bottom-right LDR
#define ldrbotl A3    // bottom-left LDR
#define manueLed 2
#define autoLed 

// les PIN qu'on va brancher les sevo motor
#define horiPin 7
#define vertiPin 9
// Sensibility Horizontal et verticale
int Hsensibility = 30;      // measurement sensitivity
int Vsensibility = 30;      // measurement sensitivity
int manualSensibility = 60; // Manual Mode sensitivity

int mode, axe, modeState, potValue, HprevpotValue, VprevpotValue = 0;
// declaration des variable pour recevoie des valeur des chaque LDR
int topl, topr, botl, botr, avgtop, avgbot, avgleft, avgright = 0;
// variable pour stocker la position acctuelle du chaque servo
int leftRightPosition, upDownPosition = 0;
// decalration des antetette des fontions utiliser
void manualMode();
void automaticMode();
void modeSwicher();
// ------------------------------------------------------------------------------------------------------------//
void setup()
{
  // pour alimentation reserver pin a HIGH 5v
  pinMode(38,OUTPUT);
  digitalWrite(38,HIGH);
  // **************//
  // initialisation des Serial et les E/S direction
  Serial.begin(115200);
  pinMode(manueLed, OUTPUT);
  pinMode(autoLed, OUTPUT);

  verti_servo.attach(vertiPin); // Servo motor up-down movement
  hori_servo.attach(horiPin);   // Servo motor right-left movement
  verti_servo.write(90);
  hori_servo.write(40);

  Cayenne.begin(username, password, clientID);
  Serial.println("initialise");
  delay(2000);
}

void loop()
{
  // pour notifier la mode actuelle et activer le mode convonable
  if (mode == 0)
  {
    Serial.println("Manual");
    manualMode();
    digitalWrite(manueLed, HIGH);
    digitalWrite(autoLed, LOW);
  }
  else
  {
    Serial.println("Automatic");
    digitalWrite(manueLed, LOW);
    digitalWrite(autoLed, HIGH);
    automaticMode();
  }
  Cayenne.loop();
}

void automaticMode()
{
  // Lire les valeur des 4 LDRs
  topr = analogRead(ldrtopr); // top right LDR
  topl = analogRead(ldrtopl); // top left LDR
  botr = analogRead(ldrbotr); // bot right LDR
  botl = analogRead(ldrbotl); // bot left LDR

  // calculer les moyenne les 4 coté (right left top bottom)
  int avgtop = (topr + topl) / 2;   // average of top LDRs
  int avgbot = (botr + botl) / 2;   // average of bottom LDRs
  int avgleft = (topl + botl) / 2;  // average of left LDRs
  int avgright = (topr + botr) / 2; // average of right LDRs

  Serial.println("-------------- Get avr --------------");
  Serial.print("average of top LDRs : ");
  Serial.println(avgtop);
  Serial.print("average of bottom LDRs : ");
  Serial.println(avgbot);
  Serial.print("average of left LDRs :");
  Serial.println(avgleft);
  Serial.print("average of right LDRs :");
  Serial.println(avgright);

  // calculer le different entre (top et bottom) et entre (right et left)
  int Vdifferent = avgtop - avgbot;
  int Hdifferent = avgright - avgleft;

  Serial.println("----------------- Get Differrent -----------------");
  Serial.print("different LDRs top and LDRs bot : ");
  Serial.println(Vdifferent);
  Serial.print("different LDRs right and LDRs left : ");
  Serial.println(Hdifferent);

  // commande de moteur horizontal ******//
  // commande les moteur : si la valeur absulue différent entre top et bottom > de la sensibilite
  if (abs(Hdifferent) >= Hsensibility)
  {
    leftRightPosition = hori_servo.read();
    Serial.print("left right Position : ");
    Serial.println(leftRightPosition);
    if (Hdifferent > 0)
    {
      if (leftRightPosition < 180)
      {
        hori_servo.write(leftRightPosition + 2);
      }
    }
    if (Hdifferent < 0)
    {
      if (leftRightPosition > 0)
      {
        hori_servo.write(leftRightPosition - 2);
      }
    }
    Serial.println("--------- horizontal Servo Control -------------");
  }

  // commande de moteur vertical ******//
  // commande les moteur : si la valeur absulue différent entre right et left > de la sensibilite
  if (abs(Vdifferent) >= Vsensibility)
  {
    upDownPosition = verti_servo.read();
    Serial.print("up down Position : ");
    Serial.println(upDownPosition);
    if (Vdifferent > 0)
    {
      if (upDownPosition < 180)
      {
        verti_servo.write(upDownPosition + 2);
      }
    }
    if (Vdifferent < 0)
    {
      if (upDownPosition > 0)
      {
        verti_servo.write(upDownPosition - 2);
      }
    }
    Serial.println("--------- vertical Servo Control -------------");
  }

  // Sens data to cayenne 
    // lde top right
    Cayenne.virtualWrite(0, topr);
    // lde top left
    Cayenne.virtualWrite(1, topl);
    // lde bottom right
    Cayenne.virtualWrite(2, botr);
    // lde bottom  left
    Cayenne.virtualWrite(3, botl);
    // Servo horizontal
    Cayenne.virtualWrite(4, leftRightPosition);
    // Servo vertical
    Cayenne.virtualWrite(5, upDownPosition);

}

void manualMode()
{
  Cayenne.loop();
}

// CAYENNE_OUT(0) {
//   // lire la valeur de ldr top right
//   int value=analogRead(ldrtopr); 
//   Cayenne.virtualWrite(0, value);
// }
// CAYENNE_OUT(1) {
//   // lire la valeur de ldr top left
//   int value=analogRead(ldrtopl); 
//   Cayenne.virtualWrite(1, value);
// }
// CAYENNE_OUT(2) {
//   // lire la valeur de ldr bottom right
//   int value=analogRead(ldrbotr); 
//   Cayenne.virtualWrite(2, value);
// }
// CAYENNE_OUT(3) {
//   // lire la valeur de ldr bottom left
//   int value=analogRead(ldrbotl); 
//   Cayenne.virtualWrite(3, value);
// }
// CAYENNE_OUT(4) {
//   // lire la valeur de servo vertical
//   int value=verti_servo.read();
//   Cayenne.virtualWrite(4, value);
// }
// CAYENNE_OUT(5) {
//   // lire la valeur de servo horizontal
//   int value=hori_servo.read();
//   Cayenne.virtualWrite(5, value);
// }

// ***************
CAYENNE_IN(6) {
  // commander le servo vertical
  CAYENNE_LOG("Channel %d, pin %d, value %d", 6, vertiPin, value);
  int value = getValue.asInt();
  verti_servo.write(value);
}
CAYENNE_IN(7) {
  // commnader le servo horizontal
  CAYENNE_LOG("Channel %d, pin %d, value %d", 7, horiPin, value);
  int value = getValue.asInt();
  hori_servo.write(value);
}
CAYENNE_IN(8) {
  // switch mode
  mode = getValue.asInt();
}

// void modeSwicher()
// {
  
//   modeState = digitalRead(modeButton);
//   if (modeState == 0)
//   {
//     //     condition   True False
//     mode = (mode == 0) ? 1 : 0;
//   }
// }