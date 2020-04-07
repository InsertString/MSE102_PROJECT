#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

#define RST_PIN 9
#define SS_PIN 10
#define LED 4
#define SHOCK 3
#define BUZZER 2

MFRC522 mfrc522(SS_PIN, RST_PIN);
Servo Lock;

// states of the lock
int global_state = 0;
#define STARTUP 0
#define ARMING 1
#define ARMED 2
#define ALARMING 3


// set up
void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  Lock.attach(5);
  pinMode(LED, OUTPUT);
  pinMode(SHOCK, INPUT);
  pinMode(BUZZER, OUTPUT);
  Serial.println("Put your card to the reader...");
  Serial.println();
}


// function to read the card
String read_card() {
   // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) 
  {
    return "0";
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    return "0";
  }
  //Show UID on serial monitor
  Serial.print("UID tag :");
  String content= "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522.uid.uidByte[i], HEX);
     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  Serial.print("Message : ");
  content.toUpperCase();

  return content.substring(1);
}


// main loop
void loop() {

  switch (global_state) {
    case STARTUP :
    Serial.println("UNLOCKED");
    Serial.println();
    digitalWrite(LED, LOW);
    noTone(BUZZER);
    Lock.write(90);
    delay(1000);
    global_state = ARMING;
    break;
    case ARMING :
    if (read_card() == "66 3D F5 25" && global_state == ARMING) {
      Serial.println("ARMED");
      Serial.println();
      delay(500);
      Lock.write(0);
      digitalWrite(LED, HIGH);
      tone(BUZZER, 500);
      delay(300);
      noTone(BUZZER);
      global_state = ARMED;
      delay(500);
      return;
    }
    
    break;
    case ARMED :
    if (digitalRead(SHOCK) == LOW) {
      global_state = ALARMING;
    }
    break;
    case ALARMING :
    digitalWrite(LED, LOW);
    tone(BUZZER, 500);
    Serial.println("ALERTING");
    Serial.println();
    if (read_card() == "66 3D F5 25") {
      global_state = STARTUP;
      return;
    }
    break;
  }
}
