/*
*  Wiegand26 - https://github.com/jvondrus/Wiegand26
*  by Jiri Vondrus (https://github.com/jvondrus)
*  Version 1.1.0 06-2020
*/

#include <Wiegand26.h>                            // Wiegand RFID

// Status report defines
#define       INICIALIZE      0
#define       DATA_SENDED     1
#define       CONNECTION      2
#define       LOGIC_FAULT     3
#define       RCV_TIMEOUT     4
#define       BITS_FAULTS     5
#define       PARITY_FRST     6
#define       PARITY_SCND     7

// Inicialize Wiegand26
Wiegand26 wiegand;
#define       wiegandD0       27                  // Pin for Data 0
#define       wiegandD1       26                  // Pin for Data 1

// Setup
void setup()
{

  // Serial line
  Serial.begin (115200, SERIAL_8N1);
  Serial.setTimeout (15);
  delay (1000);

  // Wiegand RFID
  wiegand.onData (wiegandData);
  wiegand.onState (wiegandState);
  wiegand.begin(wiegandD0, wiegandD1, true);
  //            Data0      Data1      false == Send state only on change
  //                                  true  == Send state on each data reading

  // Interrupt for Wiegand data pin
  attachInterrupt(digitalPinToInterrupt(wiegandD0), wiegandPinChanged, FALLING);
  attachInterrupt(digitalPinToInterrupt(wiegandD1), wiegandPinChanged, FALLING);

}


// Function for reading Wiegand data
void wiegandPinChanged () {
  wiegand.readData ();
}


// Print received Wiegand state
void wiegandState (uint8_t state) {
  if (bitRead (state, INICIALIZE)) {
    Serial.println ("Wiegand26 - Inicialized");
  }
  if (bitRead (state, DATA_SENDED)) {
    Serial.println ("Wiegand26 - Data sended");
  }
  if (bitRead (state, CONNECTION)) {
    Serial.println ("Wiegand26 - Disconnected");
  }
  if (bitRead (state, LOGIC_FAULT)) {
    Serial.println ("Wiegand26 - Wiring problem");
  }
  if (bitRead (state, RCV_TIMEOUT)) {
    Serial.println ("Wiegand26 - Time Out");
  }
  if (bitRead (state, BITS_FAULTS)) {
    Serial.println ("Wiegand26 - Buffer size fault");
  }
  if (bitRead (state, PARITY_FRST)) {
    Serial.println ("Wiegand26 - First parity fault");
  }
  if (bitRead (state, PARITY_SCND)) {
    Serial.println ("Wiegand26 - Second parity fault");
  }
  
}


// Print received Wiegand data
void wiegandData (unsigned long value) {
  
  Serial.print ("Wiegand26 - Data: 0x");
  Serial.println (value, HEX);

  Serial.print ("Wiegand26 - Data: 0b");
  Serial.println (value, BIN);

  Serial.print ("Wiegand26 - Data: ");
  Serial.println (value, DEC);

  Serial.println ();
}


// Main loop
void loop() {
  
  // Manually invoke Wiegand state
  //wiegand.readState ();
  
  delay (250);

}
