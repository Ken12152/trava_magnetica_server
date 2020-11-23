#include <EEPROM.h>     // We are going to read and write PICC's UIDs from/to EEPROM
#include <SPI.h>        // RC522 Module uses SPI protocol
#include <MFRC522.h>    // Library for Mifare RC522 Devices

#include "config.h"


// initialize programming mode to false
bool programMode = false;  


byte storedCard[4];   // Stores an ID read from EEPROM
byte readCard[4];     // Stores scanned ID read from RFID Module
byte masterCard[4];   // Stores master card's ID read from EEPROM


// Create MFRC522 instance.
MFRC522 mfrc522(SS_PIN, RST_PIN);




////////////////////////////////////////////////////////////////////////////////////////
void setup()
{
  Serial.begin(9600);
  //Serial.begin(115200);


  // Protocol Configuration
  //Serial.begin(9600);    // Initialize serial communications with PC
  SPI.begin();           // MFRC522 Hardware uses SPI protocol
  mfrc522.PCD_Init();    // Initialize MFRC522 Hardware



  //Serial.println(F("Access Control Example v0.1"));   // For debugging purposes
  ShowReaderDetails();  // Show details of PCD - MFRC522 Card Reader details




  /////////////// Logica para registrar master card ///////////////
  
  // Check if master card defined, if not let user choose a master card
  // This also useful to just redefine the Master Card
  // You can keep other EEPROM records just write other than 143 to EEPROM address 1
  // EEPROM address 1 should hold magical number which is '143'
  if(EEPROM.read(1) != 143) {
    Serial.println(F("No Master Card Defined"));
    Serial.println(F("Scan A PICC to Define as Master Card"));
    uint8_t successRead;
    do {
      successRead = getID();         // sets successRead to 1 when we get read from reader otherwise 0
    } while(!successRead);                   // Program will not go further while you not get a successful read
    
    for (uint8_t j = 0; j < 4; j++) {        // Loop 4 times
      EEPROM.write(2 + j, readCard[j]);      // Write scanned PICC's UID to EEPROM, start from address 3
    }
    EEPROM.write(1, 143);                    // Write to EEPROM we defined Master Card.
    Serial.println(F("Master Card Defined"));
  }



  Serial.println(F("-------------------"));
  Serial.println(F("Master Card's UID"));
  for (uint8_t i = 0; i < 4; i++) {          // Read Master Card's UID from EEPROM
    masterCard[i] = EEPROM.read(2 + i);      // Write it to masterCard
    Serial.print(masterCard[i], HEX);
  }
  Serial.println("");
  Serial.println(F("-------------------"));
  Serial.println(F("Everything is ready"));
  Serial.println(F("Waiting PICCs to be scanned"));
}






////////////////////////////////////////////////////////////////////////////////////////
void loop()
{
  /////////////// Logica pra Leitura do cartao ///////////////
  uint8_t successRead = getID();  // Variable integer to keep if we have Successful Read from Reader


  /////////////// tratamento sobre cartao lido pelo RFID ///////////////
  
  if(successRead == true) {
    ///////////////////////// modo de registro /////////////////////////
    if(programMode) {
      if(isMaster(readCard)) {            //When in program mode check First If master card scanned again to exit program mode
        Serial.println(F("Master Card Scanned"));
        Serial.println(F("Exiting Program Mode"));
        Serial.println(F("-----------------------------"));
        programMode = false;
        return;
      }
      else {
        if(findID(readCard)) {            // If scanned card is known delete it
          Serial.println(F("I know this PICC, removing..."));
          deleteID(readCard);
          Serial.println("-----------------------------");
          Serial.println(F("Scan a PICC to ADD or REMOVE to EEPROM"));
        }
        else {                            // If scanned card is not known add it
          Serial.println(F("I do not know this PICC, adding..."));
          writeID(readCard);
          Serial.println(F("-----------------------------"));
          Serial.println(F("Scan a PICC to ADD or REMOVE to EEPROM"));
        }
      }
    }
    ///////////////////////// nao esta em modo de registro /////////////////////////
    else {
      if(isMaster(readCard)) {            // If scanned card's ID matches Master Card's ID - enter program mode
        programMode = true;
        Serial.println(F("Hello Master - Entered Program Mode"));
        uint8_t count = EEPROM.read(0);   // Read the first Byte of EEPROM that
        
        Serial.print(F("I have "));       // stores the number of ID's in EEPROM
        Serial.print(count);
        Serial.print(F(" record(s) on EEPROM"));
        Serial.println("");
        Serial.println(F("Scan a PICC to ADD or REMOVE to EEPROM"));
        Serial.println(F("Scan Master Card again to Exit Program Mode"));
        Serial.println(F("-----------------------------"));
      }
      else {
        if(findID(readCard)) {            // If not, see if the card is in the EEPROM
          Serial.println(F("Welcome, You shall pass"));
          Serial.println(F("-----------------------------"));
          //granted(300);                   // Open the door lock for 300 ms
        }
        else {                            // If not, show that the ID was not valid
          Serial.println(F("You shall not pass"));
          Serial.println(F("-----------------------------"));
          //denied();
        }
      }
    }
  }
}
