#include <Servo.h>
#include <Math.h>

//Servo Benennungen
Servo ServoY;                         // zuständig für Links/Rechts Drehung, Rumpf
Servo ServoX1;                        // zuständig für Hoch Runter Drehung, Rumpf
Servo ServoX2;                        // zuständig für Hoch/Runter Drehung, Ellbogen
//Servo ServoX3;                      // zuständig für Links/Rechts Drehung, Kopf
Servo ServoX4;                        // zuständig für Vor/Zurück Drehung, Kopf

const int servoDelay = 500;           //500ms Wartezeit zwischen einzelne Servobewegungen

// Startposition ist die Position wo Kamera alles auf dem Blick hat
const int startPosY = 90;             // größerer Winkel -> Linksdrehung (Perspektive von Arm) 90
const int startPosX1 = 110;           // kleinerer Winkel -> Bewegung nach unten 110
const int startPosX2 = 115;           // kleinerer Winkel-> Bewegung nach oben 115
//const int startPosX3 = 5;           // größerer Winkel -> Rechtsdrehung (Perspektive von Arm) 5
const int startPosX4 = 85;            // größerer Winkel ->  Bewegung nach oben, Seite Schlauch 85

// Grundposition ist die Position in der servo auf die Höhe vom Shotglas fährt mitte des Bildes
const int grundPosY = 90;             //90
const int grundPosX1 = 80;            //80
const int grundPosX2 = 140;           //140
const int grundPosX3 = 145;           //145

//Werte zum Berechnen der Winkel (Kinematik)
const float grundPosWinkelX = 53;     // tatsächlicher Winkel X1 zum Boden wenn grundPosX1 = 80
const float laengeY = 22.5;           //Bildbreite Y-Richtung
const float laengeX = 16.5;           //Bildbreite X-Richtung
const float mitteCamY =11.25;
const float mitteCamX = 8.25;
const float faktorY= 28.44;           //Bildpixel Faktor zum Umrechnen Pixel auf cm, 480p auf 16,5cm -> 480/16,5= 28,44
const float faktorX= 29.09;           //Bildpixel Faktor zum Umrechnen Pixel auf cm, 640p auf 22,5cm -> 640/22,5= 29,09
const int laengeArm = 12;             //beide Armteile gleich lang
const float laengeX1ZuBildkante = 6;  // ca. 6cm X1 Servo zur Bildkante
const int winkelSchlauchZuKamera = 8; //8° Müssen beide Armteile sich ca bewegen um den Abstand von der Kamera zum Schlauch zu bereinigen ca 2.5cm


void attachServos();                        //Servos->PIN: Y->12, X1->11, >X2->10, X3->9, X4->8
void bewegeZuStartPosition();               
void detachServos();
void processdata();                         //Funktion zum Verarbeiten der Daten über UART(serielle Schnittstelle)
void berechneWinkel(short val1, short val2);//Berechnung Winkel fuer Servos
void korrekturPositionKamerazuSchlauch();   //Funktion zum nachkorrigieren Abstand Kamera und Schlauch
void pumpeAktivieren();                     //Aktivieren der Pumpe die mit PIN 4 gesteuert wird


void setup() {
  Serial.begin(9600);
  attachServos();
  bewegeZuStartPosition();
  detachServos();
}

void loop() {                               //main
  while (Serial.available()) {
    processdata();
  }
}


void attachServos(){
  ServoY.attach(12);
  ServoX1.attach(11);
  ServoX2.attach(10);
//  ServoX3.attach(9);
  ServoX4.attach(8);
}

void bewegeZuStartPosition(){
  ServoY.write(startPosY);
  delay(servoDelay);
  ServoX2.write(startPosX2);
  delay(servoDelay);
  ServoX1.write(startPosX1);
//  delay(servoDelay);
//  ServoX3.write(startPosX3);
  delay(servoDelay);
  ServoX4.write(startPosX4);
  delay(servoDelay);
  delay(servoDelay);
}

void detachServos(){
  ServoY.detach();
  ServoX1.detach();
  ServoX2.detach();
//  ServoX3.detach();
  ServoX4.detach();
}

void processdata()
{
  int bytes = Serial.available();
  if(bytes==4)
  {
    short val1;
    short val2;
    val1 = Serial.read() | (Serial.read()<<8);
    val2 = Serial.read() | (Serial.read()<<8);
    berechneWinkel(val1, val2);
    //pumpeAktivieren();
    bewegeZuStartPosition();
    detachServos();
    //HIER MUSS NOCH EINE RÜCKMELDUNG REIN, DAMIT DIE KI WIEDER NÄCHSTE POSITION ABSCHICKT, MIT NIKLAS ABSPRECHEN
  }
}

void berechneWinkel(short val1, short val2){
  float y;
  float x;
  int yMov;
  int xMov;
  int xMov1;
  int xMov2;
  int xMov4;
  float winkelY;
  float winkelX;
  float hypStrecke;

  x = laengeX + laengeX1ZuBildkante - (val2 / faktorX);       //Länge X, vom Roboterarm zum Ziel
  if(val1/faktorY >= mitteCamY){                              //Länge Y, Wenn Objekt Rechts vom Roboterarm
    y = (val1 / faktorY) - mitteCamY;
    winkelY = atan(y / x) * (180.0 / PI);
    yMov= startPosY - ceil(winkelY);
  }
  else {                                                      //Länge Y, Wenn Objekt Links vom Roboterarm
    y =  mitteCamY - (val1 / faktorY);
    winkelY = atan(y / x) * (180.0 / PI);
    yMov= startPosY + ceil(winkelY);
  }

  hypStrecke = sqrt(y*y + x*x);                               //Länge der Gesamtstrecke (Hypothenose)
  winkelX = acos((hypStrecke*hypStrecke+laengeArm*laengeArm - laengeArm*laengeArm)/(2*laengeArm*hypStrecke)) * (180.0 / PI);

  if (winkelX >= grundPosWinkelX){                            //Winkelberechnung X1,X2 und X4, wenn Objekt zwischen Bildmitte und Roboterarm
    xMov = ceil(winkelX) - grundPosWinkelX;
    xMov1 = grundPosX1 + xMov;
    xMov2 = grundPosX2 + 2 * xMov;
    xMov4 = grundPosX3 + xMov;
  }
  else {                                                      //Winkelberechnung X1,X2 und X4, wenn Objekt nicht zwischen Bildmitte und Roboterarm
    xMov = grundPosWinkelX - ceil(winkelX);
    xMov1 = grundPosX1 - xMov;
    xMov2 = grundPosX2 - 2 * xMov;
    xMov4 = grundPosX3 - xMov;
  }
    
  /*  
  //Testcode über UART (serielle Schnittstelle) zum überprüfen der Werte
  Serial.print(" Grad | Servo Y: ");
  Serial.print(yMov);
  Serial.println();  // sendet nur den Zeilenumbruch (CR+LF)
  Serial.print(winkelY); 
  Serial.println();  // sendet nur den Zeilenumbruch (CR+LF) 
  Serial.print(y);
  Serial.println();/  / sendet nur den Zeilenumbruch (CR+LF) 
  Serial.print(" Grad | Servo X: ");
  Serial.print(xMov);
  Serial.println();  // sendet nur den Zeilenumbruch (CR+LF) 
  Serial.print(winkelX);
  Serial.println();/  / sendet nur den Zeilenumbruch (CR+LF) 
  Serial.print(x);
  Serial.println();/  / sendet nur den Zeilenumbruch (CR+LF) 
  Serial.print(" Grad | Servo X1: ");
  Serial.print(xMov1);
  Serial.print(" Grad | Servo X2: ");
  Serial.print(xMov2);
  Serial.print(" Grad | Servo X4: ");
  Serial.print(xMov4);
  delay(2000);
  */
  
  /*
  !!!!!
!!!HIER NOCH RECHNUNG SCHREIBEN ZUR KORRIGIERUNG DES ABSTANDS VON SCHLAUCH ZUR KAMERA!!!!!!!
  !!!!!
  */

  //Servos bewegen sich zur Zielposition
  attachServos();
  ServoY.write(yMov);
  delay(servoDelay);
  ServoX1.write(xMov1);
  delay(servoDelay);
  ServoX2.write(xMov2);
  delay(servoDelay);
  ServoX4.write(xMov4);
  delay(3000);
}

void pumpeAktivieren(){
  pinMode(4, OUTPUT);       // Setzt Pin 4 als Ausgang
  digitalWrite(4, HIGH);    // Setzt Pin 4 auf HIGH (5V)
  delay(2000);              // Wartet 2 Sekunden
  digitalWrite(4, LOW);     // Setzt Pin 4 auf LOW (0V)
  delay(2000);              // Wartet 2 Sekunden
}