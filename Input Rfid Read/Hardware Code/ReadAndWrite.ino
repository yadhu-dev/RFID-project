#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN         9
#define SS_PIN          10

int led = 3;
int buzzer = 8;


//////////////////////////////////////////////////////////////////////////////////
/////////////////////////// Creating Object //////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance


//////////////////////////////////////////////////////////////////////////////////
/////////////////////////// set up ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(9600);  // Initialize serial communications
  pinMode(led, OUTPUT);
  pinMode(buzzer, OUTPUT);
  SPI.begin();        // Initialize SPI bus
  mfrc522.PCD_Init(); // Initialize MFRC522
  Serial.println("Setup complete. Scan your ID card.");
  ///////////////////////////////////////////////////////////////////////////////
  start();                ////////// start indiaction buuzer/////////////////////
  ///////////////////////////////////////////////////////////////////////////////
}


//////////////////////////////////////////////////////////////////////////////////
/////////////////////////// loop /////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
void loop() {
  if (Serial.available() > 0) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "Read") {
      Read();
    } else if (cmd == "Write") {
      Write();
    }
  }
}



//////////////////////////////////////////////////////////////////////////////////
/////////////////////////// Read function ////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
void Read() {
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;  // Using default key value
  }

  byte sector = 1;
  byte blockAddr = 4;
  byte buffer[18];
  byte bufferSize = sizeof(buffer);

  // Authenticate using key A
  MFRC522::StatusCode status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockAddr, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // Read data from the block
  status = mfrc522.MIFARE_Read(blockAddr, buffer, &bufferSize);
  if (status == MFRC522::STATUS_OK) {
    String rollNo = "";
    for (uint8_t i = 0; i < 16; i++) {
      if (buffer[i] >= 32 && buffer[i] <= 126 && buffer[i] != '#') {
        rollNo += (char)buffer[i];
      }
    }
    Serial.println(rollNo);
    digitalWrite(led, HIGH);
    digitalWrite(buzzer, HIGH);
    delay(50);
    digitalWrite(led, LOW);
    digitalWrite(buzzer, LOW);
  } else {
    Serial.print(F("MIFARE_Read() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
  }

  // Halt PICC
  mfrc522.PICC_HaltA();
  // Stop encryption on PCD
  mfrc522.PCD_StopCrypto1();
}



//////////////////////////////////////////////////////////////////////////////////
/////////////////////////// Write function ////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
void Write() {
  Serial.println("Place your ID card on the scanner....");

  while (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    delay(50);
  }

  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);

  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
      piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
      piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }

  String uid = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    uid += String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    uid += String(mfrc522.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();

  Serial.println("UID: " + uid);

  while (true) {
    if (Serial.available() > 0) {
      String cmd = Serial.readStringUntil('\n');
      cmd.trim();

      if (cmd == "get_data") {
        while (Serial.available() == 0) {
          // Wait for the data
        }

        String data_hex = Serial.readStringUntil('\n');
        data_hex.trim();

        byte dataBlock[16];
        for (int i = 0; i < 16; i++) {
          String hexPair = data_hex.substring(i * 2, i * 2 + 2);
          dataBlock[i] = strtol(hexPair.c_str(), NULL, 16); // Convert hex string to byte
        }

        byte sector = 1;
        byte block = 0;

        MFRC522::MIFARE_Key key;
        for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

        if (writeBlock(sector, block, dataBlock, key)) {
          Serial.println(F("Data written to block 4 successfully"));
          digitalWrite(led, HIGH);
          digitalWrite(buzzer, HIGH);
          delay(50);
          digitalWrite(led, LOW);
          digitalWrite(buzzer, LOW);
          delay(50);
          digitalWrite(led, HIGH);
          digitalWrite(buzzer, HIGH);
          delay(100);
          digitalWrite(led, LOW);
          digitalWrite(buzzer, LOW);
          delay(200);
          digitalWrite(led, HIGH);
          digitalWrite(buzzer, HIGH);
          delay(1000);
          digitalWrite(led, LOW);
          digitalWrite(buzzer, LOW);
        } else {
          Serial.println(F("Failed to write to block 4"));
        }

        mfrc522.PICC_HaltA(); 
        mfrc522.PCD_StopCrypto1();
        break;
      }
    }
  }
}

bool writeBlock(byte sector, byte block, byte *dataBlock, MFRC522::MIFARE_Key key) {
  byte blockAddr = sector * 4 + block; // Calculate the absolute block address

  // Authenticate the block
  MFRC522::StatusCode status;
  status = (MFRC522::StatusCode)mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockAddr, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return false;
  }

  // Write data to the block
  status = (MFRC522::StatusCode)mfrc522.MIFARE_Write(blockAddr, dataBlock, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return false;
  }
  return true;
}
void start(){
  /*digitalWrite(buzzer,HIGH);
  digitalWrite(led,HIGH);
  delay(50);
  digitalWrite(buzzer,LOW);
  digitalWrite(led,LOW);
  delay(100);                //set1
  digitalWrite(buzzer,HIGH);
  digitalWrite(led,HIGH);
  delay(50);
  digitalWrite(buzzer,LOW);
  digitalWrite(led,LOW);
  delay(50);             //set2
  digitalWrite(buzzer,HIGH);
  digitalWrite(led,HIGH);
  delay(50);
  digitalWrite(buzzer,LOW);
  digitalWrite(led,LOW);
  delay(100);            //set3
  digitalWrite(buzzer,HIGH);
  digitalWrite(led,HIGH);
  delay(800);              //long
  digitalWrite(buzzer,LOW);
  digitalWrite(led,LOW);
  delay(50);               //set4
  digitalWrite(buzzer,HIGH);
  digitalWrite(led,HIGH);
  delay(100);
  digitalWrite(buzzer,LOW);
  digitalWrite(led,LOW);
  delay(50); */

  digitalWrite(led,HIGH);
  digitalWrite(buzzer,HIGH);
  delay(1000);
  digitalWrite(led,LOW);
  digitalWrite(buzzer,LOW);
  
}
