#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN 9
#define SDA_PIN 10

int green_led = 3;
int red_led = 4;
int buzzer = 5;

MFRC522 mfrc522(SDA_PIN, RST_PIN);

void setup() {
  pinMode(green_led, OUTPUT);
  pinMode(red_led, OUTPUT);
  pinMode(buzzer, OUTPUT);

  Serial.begin(9600);
  while (!Serial) {
    ; 
  }
  SPI.begin();
  mfrc522.PCD_Init();
}

void loop() {
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    delay(50);
    return;
  }

  MFRC522::PICC_Type card = mfrc522.PICC_GetType(mfrc522.uid.sak);

  if (card != MFRC522::PICC_TYPE_MIFARE_MINI && 
      card != MFRC522::PICC_TYPE_MIFARE_1K && 
      card != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not a type of Mifare Classic..."));
    return;
  }

  String uid = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    uid += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
    uid += String(mfrc522.uid.uidByte[i], HEX);
  }

  uid.toUpperCase();
  Serial.println(uid);
  mfrc522.PICC_HaltA();

  delay(1000);

  if (Serial.available() > 0) {
    String Cmd = Serial.readStringUntil('\n');
    if (Cmd == "GREEN") {
      digitalWrite(green_led, HIGH);
      digitalWrite(red_led, LOW);
      digitalWrite(buzzer, LOW);
      delay(1000);  
      digitalWrite(green_led, LOW);  
    } else if (Cmd == "RED") {
      digitalWrite(green_led, LOW);
      digitalWrite(red_led, HIGH);
      digitalWrite(buzzer, HIGH);
      delay(2000);  
      digitalWrite(red_led, LOW);  
      digitalWrite(buzzer, LOW);  
    } else {
      digitalWrite(green_led, LOW);
      digitalWrite(red_led, LOW);
      digitalWrite(buzzer, LOW);
    }
  }
}
