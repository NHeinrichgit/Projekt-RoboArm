#include <Servo.h>
#include <Math.h>

/* Steuerung Pumpe
  pinMode(4, OUTPUT);       // Setzt Pin 4 als Ausgang
  digitalWrite(4, HIGH);    // Setzt Pin 4 auf HIGH (5V)
  delay(2000);              // Wartet 2 Sekunden
  digitalWrite(4, LOW);     // Setzt Pin 4 auf LOW (0V)
*/

Servo ServoY;  // zuständig für Links/Rechts Drehung, Rumpf
Servo ServoX1;  // zuständig für Hoch Runter Drehung, Rumpf
Servo ServoX2;  // zuständig für Hoch/Runter Drehung, Ellbogen
//Servo ServoX3;  // zuständig für Links/Rechts Drehung, Kopf
Servo ServoX4;  // zuständig für Vor/Zurück Drehung, Kopf


String inputBuffer = "";
// Startposition ist die Position wo Kamera alles auf dem Blick hat
const int startPosY = 90;    // größerer Winkel -> Linksdrehung (Perspektive von Arm) 90
const int startPosX1 = 110;  // kleinerer Winkel -> Bewegung nach unten 110
const int startPosX2 = 115; // kleinerer Winkel-> Bewegung nach oben 115
//const int startPosX3 = 5;   // größerer Winkel -> Rechtsdrehung (Perspektive von Arm) 5
const int startPosX4 = 85;  // größerer Winkel ->  Bewegung nach oben, Seite Schlauch 85

// Grundposition ist die Position wo servo auf die Höhe fährt vom Shotglas (immer als erstes ausführen!)
const int grundPosY = 90;
const int grundPosX1 = 80;
const int grundPosX2 = 140;
const int grundPosX3 = 145;

const float grundPosWinkelX = 53; // tatsächlicher Winkel X1 zum Boden wenn grundPosX1 = 80

const float mitteCamY =11.25;
const float mitteCamX = 8.25;
const int winkelSchlauchZuKamera = 8;   //8° Müssen beide Armteile sich ca bewegen um den Abstand von der Kamera zum Schlauch zu bereinigen
const float faktorY= 28.44;  //Bildpixel Faktor zum Umrechnen 480p auf 16,5cm -> 480/16,5= 28,44
const float faktorX= 29.09; //Bildpixel Faktor zum Umrechnen 640p auf 22,5cm -> 640/22,5= 29,09
const float laengeY = 22.5;
const float laengeX = 16.5;
const int laengeArm = 12; // 12cm beide Armteile gleich lang
const int servoDelay = 400;

int x1;
int x2;
int x3;

void korrekturPositionKamerazuSchlauch();
void processdata();

void setup() {
  Serial.begin(9600);
  ServoY.attach(12);
  ServoX1.attach(11);
  ServoX2.attach(10);
//  ServoX3.attach(9);
  ServoX4.attach(8);

  ServoY.write(startPosY);
  delay(servoDelay);
  ServoX2.write(startPosX2);
  delay(servoDelay);
  ServoX1.write(startPosX1);
//  delay(300);
//  ServoX3.write(startPosX3);
  delay(servoDelay);
  ServoX4.write(startPosX4);
  delay(servoDelay);
  ServoY.detach();
  ServoX1.detach();
  ServoX2.detach();
//  ServoX3.detach();
  ServoX4.detach();
  Serial.println("Bereit. Sende Winkel als: y,x (z.B. 45,90)");
}

void loop() {
  while (Serial.available()) {
    /* char c = Serial.read();

    // Eingabe beenden mit Zeilenumbruch
    if (c == '\n' || c == '\r') {
      processInput(inputBuffer);
      inputBuffer = "";
    } else {
      inputBuffer += c; 
      
    }*/
    processdata();
  }
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
    float y;
    float x;
    int yMov;
    int xMov;
    int xMov1;
    int xMov2;
    int xMov4;
    float winkelY;
    float winkelX;
    x = laengeX - (val2 / faktorX);
    if(val1/faktorY > mitteCamY){
      y = (val1 / faktorY) - mitteCamY;
      winkelY = atan(y / x) * (180.0 / PI);
      yMov= startPosY - ceil(winkelY);
    }
    else {
      y =  mitteCamY - (val1 / faktorY);
      winkelY = atan(y / x) * (180.0 / PI);
      yMov= startPosY + ceil(winkelY);
    }
    float hypStrecke = sqrt(y*y + x*x);
    winkelX = acos((hypStrecke*hypStrecke+laengeArm*laengeArm - laengeArm*laengeArm)/(2*laengeArm*hypStrecke)) * (180.0 / PI);

    if (winkelX > grundPosWinkelX){
      xMov = ceil(winkelX) - grundPosWinkelX;
      xMov1 = grundPosX1 + xMov;
      xMov2 = grundPosX2 + 2 * xMov;
      xMov4 = grundPosX3 + xMov;
    }
    else {
      xMov = grundPosWinkelX - ceil(winkelX);
      xMov1 = grundPosX1 - xMov;
      xMov2 = grundPosX2 - 2 * xMov;
      xMov4 = grundPosX3 - xMov;
    }
  /*  
  Serial.print(" Grad | Servo Y: ");
  Serial.print(yMov);
  Serial.println();  // sendet nur den Zeilenumbruch (CR+LF)
  Serial.print(winkelY); 
  Serial.println();  // sendet nur den Zeilenumbruch (CR+LF) 
  Serial.print(y);
  Serial.print(" Grad | Servo X: ");
  Serial.print(xMov);
  Serial.println();  // sendet nur den Zeilenumbruch (CR+LF) 
  Serial.print(winkelX);
  Serial.println();  // sendet nur den Zeilenumbruch (CR+LF) 
  Serial.print(x);
  Serial.print(" Grad | Servo X1: ");
  Serial.print(xMov1);
  Serial.print(" Grad | Servo X2: ");
  Serial.print(xMov2);
  Serial.print(" Grad | Servo X4: ");
  Serial.print(xMov4);
  delay(2000); // Wartezeit für Bewegung
  */
  
    // Servos aktivieren, bewegen, dann wieder detach
    ServoY.attach(12);
    ServoX1.attach(11);
    ServoX2.attach(10);
    ServoX4.attach(8);

    ServoY.write(yMov);
    delay(2000); // Wartezeit für Bewegung
    ServoX1.write(xMov1);
    delay(2000); // Wartezeit für Bewegung
    ServoX2.write(xMov2);
    delay(2000); // Wartezeit für Bewegung
    ServoX4.write(xMov4);
    delay(2000); // Wartezeit für Bewegung
  
    ServoY.detach();
    ServoX1.detach();
    ServoX2.detach();
    ServoX4.detach();
    delay(10000);
    ServoY.attach(12);
    ServoX1.attach(11);
    ServoX2.attach(10);
//  ServoX3.attach(9);
    ServoX4.attach(8);

    ServoY.write(startPosY);
    delay(servoDelay);
    ServoX2.write(startPosX2);
    delay(servoDelay);
    ServoX1.write(startPosX1);
  //  delay(300);
  //  ServoX3.write(startPosX3);
    delay(servoDelay);
    ServoX4.write(startPosX4);
    delay(servoDelay);
    ServoY.detach();
    ServoX1.detach();
    ServoX2.detach();
  //  ServoX3.detach();
    ServoX4.detach();
/*
    Serial.print("Servo Y: ");
    Serial.print(y);
    Serial.print(" Grad | Servo X: ");
    Serial.print(x);
    Serial.println(" Grad gesetzt.");>
*/
  }
}
void korrekturPositionKamerazuSchlauch(){
x1=x1+winkelSchlauchZuKamera;
x2=x2+2*winkelSchlauchZuKamera; // 2*, da X1 nachkorrigiert muss
x3=x3+winkelSchlauchZuKamera;
}

/*
void processInput(String data) {
  data.trim(); // Whitespace entfernen

  int commaIndex = data.indexOf(',');
  if (commaIndex == -1) {
    Serial.println("Fehler: Format ist y,x (z.B. 45,90)");
    return;
  }

  String yStr = data.substring(0, commaIndex);
  String xStr = data.substring(commaIndex + 1);

  int y = yStr.toInt();
  int x = xStr.toInt();

  if (y < 0 || y > 180 || x < 0 || x > 180) {
    Serial.println("Fehler: Winkel muessen zwischen 0 und 180 liegen.");
    return;
  }
  ServoY.attach(12);
  ServoX1.attach(11);
  // Servos aktivieren, bewegen, dann wieder detach
  ServoY.write(y);
  delay(500); // Wartezeit für Bewegung
  ServoX1.write(x);
  delay(500); // Wartezeit für Bewegung

  ServoY.detach();
  ServoX1.detach();

  Serial.print("Servo Y: ");
  Serial.print(y);
  Serial.print(" Grad | Servo X: ");
  Serial.print(x);
  Serial.println(" Grad gesetzt.");
}*/