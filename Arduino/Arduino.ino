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


// Mantém o estado atual dos pinos (HIGH ou LOW)
int pinsStatus[PINS_COUNT];




////////////////////////////////////////////////////////////////////////////////////////
void setup()
{
  Serial.begin(9600);
  //Serial.begin(115200);


  /////////////// Configurar pinos ///////////////
  
  // Coloca os pinos que estão ligados os relês como saída
  for(int i = 0; i < PINS_COUNT; i++) {
    pinsStatus[i] = LOW;
    
    int pinNumber = FIRST_PIN + i;
    pinMode(pinNumber, OUTPUT);
    digitalWrite(pinNumber, pinsStatus[i]);
  }


  // Arduino Pin Configuration
  pinMode(redLed,   OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(blueLed,  OUTPUT);
  pinMode(wipeB,    INPUT_PULLUP);   // Enable pin's pull up resistor
  pinMode(relay,    OUTPUT);

  // Be careful how relay circuit behave on while resetting or power-cycling your Arduino
  digitalWrite(relay, HIGH);        // Make sure door is locked
  digitalWrite(redLed, LED_OFF);    // Make sure led is off
  digitalWrite(greenLed, LED_OFF);  // Make sure led is off
  digitalWrite(blueLed, LED_OFF);   // Make sure led is off



  // Protocol Configuration
  //Serial.begin(9600);    // Initialize serial communications with PC
  SPI.begin();           // MFRC522 Hardware uses SPI protocol
  mfrc522.PCD_Init();    // Initialize MFRC522 Hardware



  //Serial.println(F("Access Control Example v0.1"));   // For debugging purposes
  //ShowReaderDetails();  // Show details of PCD - MFRC522 Card Reader details




  /////////////// Logica para restaurar EEPROM do Arduino, aciona se continuar pressionado botao 'wipeB' na hora de inicializar ///////////////
  
  // Wipe Code - If the Button (wipeB) Pressed while setup run (powered on) it wipes EEPROM
  if(digitalRead(wipeB) == LOW) {       // when button pressed pin should get low, button connected to ground
    digitalWrite(redLed, LED_ON);       // Red Led stays on to inform user we are going to wipe
    
    //Serial.println(F("Wipe Button Pressed"));
    //Serial.println(F("You have 10 seconds to Cancel"));
    //Serial.println(F("This will be remove all records and cannot be undone"));
    
    bool buttonState = monitorWipeButton(10000);              // Give user enough time to cancel operation
    
    if(buttonState == true && digitalRead(wipeB) == LOW) {    // If button still be pressed, wipe EEPROM
      //Serial.println(F("Starting Wiping EEPROM"));
      
      for (uint16_t x = 0; x < EEPROM.length(); x = x + 1) {   // Loop end of EEPROM address
        if(EEPROM.read(x) == 0) {                             // If EEPROM address 0
          // do nothing, already clear, 
          // go to the next address in order 
          // to save time and reduce writes to EEPROM
        }
        else {
          EEPROM.write(x, 0);  // if not write 0 to clear, it takes 3.3mS
        }
      }
      //Serial.println(F("EEPROM Successfully Wiped"));
      digitalWrite(redLed, LED_OFF);  // visualize a successful wipe
      delay(200);
      digitalWrite(redLed, LED_ON);
      delay(200);
      digitalWrite(redLed, LED_OFF);
      delay(200);
      digitalWrite(redLed, LED_ON);
      delay(200);
      digitalWrite(redLed, LED_OFF);
    }
    else {
      //Serial.println(F("Wiping Cancelled")); // Show some feedback that the wipe button did not pressed for 15 seconds
      digitalWrite(redLed, LED_OFF);
    }
  }




  /////////////// Logica para registrar master card ///////////////
  
  // Check if master card defined, if not let user choose a master card
  // This also useful to just redefine the Master Card
  // You can keep other EEPROM records just write other than 143 to EEPROM address 1
  // EEPROM address 1 should hold magical number which is '143'
  if(EEPROM.read(1) != 143) {
    //Serial.println(F("No Master Card Defined"));
    //Serial.println(F("Scan A PICC to Define as Master Card"));
    uint8_t successRead;
    do {
      successRead = getID();         // sets successRead to 1 when we get read from reader otherwise 0
      digitalWrite(blueLed, LED_ON);         // Visualize Master Card need to be defined
      delay(200);
      digitalWrite(blueLed, LED_OFF);
      delay(200);
    } while(!successRead);                   // Program will not go further while you not get a successful read
    
    for (uint8_t j = 0; j < 4; j++) {        // Loop 4 times
      EEPROM.write(2 + j, readCard[j]);      // Write scanned PICC's UID to EEPROM, start from address 3
    }
    EEPROM.write(1, 143);                    // Write to EEPROM we defined Master Card.
    //Serial.println(F("Master Card Defined"));
  }



  /*
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
  */


  
  cycleLeds();    // Everything ready lets give user some feedback by cycling leds
}






////////////////////////////////////////////////////////////////////////////////////////
void loop()
{
  /////////////// Sializacao ///////////////
  
  if(programMode) {
    cycleLeds();            // Program Mode cycles through Red Green Blue waiting to read a new card
  }
  else {
    normalModeOn();         // Normal mode, blue Power LED is on, all others are off
  }




  /////////////// Logiaca pra Limpar EEPROM ///////////////
  
  // When device is in use if wipe button pressed for 10 seconds initialize Master Card wiping
  if(digitalRead(wipeB) == LOW) {       // Check if button is pressed
      
    // Visualize normal operation is iterrupted by pressing wipe button Red is like more Warning to user
    digitalWrite(redLed, LED_ON);       // Make sure led is off
    digitalWrite(greenLed, LED_OFF);    // Make sure led is off
    digitalWrite(blueLed, LED_OFF);     // Make sure led is off
      
    // Give some feedback
    //Serial.println(F("Wipe Button Pressed"));
    //Serial.println(F("Master Card will be Erased! in 10 seconds"));
      
    bool buttonState = monitorWipeButton(10000);              // Give user enough time to cancel operation
    
    if(buttonState == true && digitalRead(wipeB) == LOW) {    // If button still be pressed, wipe EEPROM
      EEPROM.write(1, 0);   // Reset Magic Number.
      //Serial.println(F("Master Card Erased from device"));
      //Serial.println(F("Please reset to re-program Master Card"));
      while(1);
    }
    //Serial.println(F("Master Card Erase Cancelled"));
  }




  /////////////// Logica pra Leitura do cartao ///////////////
  uint8_t successRead = getID();  // Variable integer to keep if we have Successful Read from Reader

  /////////////// se nao tiver nenhuma requisicao e nem cartao lido pula p proximo ciclo ///////////////
  if(!Serial.available() && successRead == false) {
    return;
  }

  


  /////////////// tratamento sobre requisicao do servidor ///////////////
  
  // Verifica se há um novo cliente
  if(Serial.available()) {
    //Faz a leitura da requisição
    char* request = readRequest();

    //Se a requisição não for para o favicon
    if(strstr(request, "favicon") == NULL) {
      //Executamos a ação com o valor passado na requisição
      //execute(getAction(request), getValue(request));

      /*------------------------------------------------------------------------------------*/
      String action = getAction(request);
      //String value = getValue(request);

      //if(action == "on" || action == "off") {
      if(action == "on") {
        // Os relês são numerados a partir do 1, max o array começa do 0
        // então tiramos 1
        //int index = value.toInt() - 1;
        
        // O número do pino será o índice mais o número do pino onde os relês
        // começam. Os relês devem estar em sequência a partir do pino inicial (FIRST_PIN)
        //int pinNumber = FIRST_PIN + index;
        
        //int status = action == "on" ? HIGH : LOW;
        
        //digitalWrite(pinNumber, status);
        //pinsStatus[index] = status;

        granted(300);   // Open the door lock for 300 ms
      }
      /*------------------------------------------------------------------------------------*/

      //Envia a resposta ao cliente
      sendResponse();
    }
    else {
      Serial.print(
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Length: 0\r\n"
        "Connection: close\r\n"
        "\r\n"
      );
    }
    Serial.write(127);
  }




  /////////////// tratamento sobre cartao lido pelo RFID ///////////////
  
  if(successRead == true) {
    ///////////////////////// modo de registro /////////////////////////
    if(programMode) {
      if(isMaster(readCard)) {            //When in program mode check First If master card scanned again to exit program mode
        //Serial.println(F("Master Card Scanned"));
        //Serial.println(F("Exiting Program Mode"));
        //Serial.println(F("-----------------------------"));
        programMode = false;
        return;
      }
      else {
        if(findID(readCard)) {            // If scanned card is known delete it
          //Serial.println(F("I know this PICC, removing..."));
          deleteID(readCard);
          //Serial.println("-----------------------------");
          //Serial.println(F("Scan a PICC to ADD or REMOVE to EEPROM"));
        }
        else {                            // If scanned card is not known add it
          //Serial.println(F("I do not know this PICC, adding..."));
          writeID(readCard);
          //Serial.println(F("-----------------------------"));
          //Serial.println(F("Scan a PICC to ADD or REMOVE to EEPROM"));
        }
      }
    }
    ///////////////////////// nao esta em modo de registro /////////////////////////
    else {
      if(isMaster(readCard)) {            // If scanned card's ID matches Master Card's ID - enter program mode
        programMode = true;
        //Serial.println(F("Hello Master - Entered Program Mode"));
        uint8_t count = EEPROM.read(0);   // Read the first Byte of EEPROM that
        
        //Serial.print(F("I have "));       // stores the number of ID's in EEPROM
        //Serial.print(count);
        //Serial.print(F(" record(s) on EEPROM"));
        //Serial.println("");
        //Serial.println(F("Scan a PICC to ADD or REMOVE to EEPROM"));
        //Serial.println(F("Scan Master Card again to Exit Program Mode"));
        //Serial.println(F("-----------------------------"));
      }
      else {
        if(findID(readCard)) {            // If not, see if the card is in the EEPROM
          //Serial.println(F("Welcome, You shall pass"));
          granted(300);                   // Open the door lock for 300 ms
        }
        else {                            // If not, show that the ID was not valid
          //Serial.println(F("You shall not pass"));
          denied();
        }
      }
    }
  }
}
