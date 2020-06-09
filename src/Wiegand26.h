/*
*  Wiegand26
*  by Jiri Vondrus (https://github.com/jvondrus)
*  Version 1.1.0 06-2020
*/

// Define
#ifndef Wiegand26_h
#define Wiegand26_h

// Import
#include <stdint.h>

// Setings
#define MAX_BITS      26


// Class
class Wiegand26
{

  // Public part
  public:

    // Inicialize
    Wiegand26 () { }
    Wiegand26 (uint8_t pinData0, uint8_t pinData1, bool status=true) {
      begin (pinData0, pinData1);
    }
    void begin (uint8_t pinData0, uint8_t pinData1, bool status=true);

    // Reading data
    void readData (void);

    // Reading state
    void readState (void);

    // Emit data
    void onData (void (*func) (unsigned long data)) {
      funcion_data = (callback_data)func;
    }

    // Emit State
    void onState (void (*func) (uint8_t state)) {
      funcion_state = (callback_state)func;
    }

    // Type definition
    typedef void (*callback_data) (unsigned long data);
    typedef void (*callback_state) (uint8_t state);


  // Private part
  private:

    // Timestamps
    unsigned long _timestamp;
    unsigned long _elapsed;

    // Pin
    uint8_t _pinData0;
    uint8_t _pinData1;
    bool _stateData0;
    bool _stateData1;
    bool _stateLed;

    // Buffer
    uint8_t _bitCnt;
    bool _bitData[MAX_BITS];
    unsigned long _data;

    // State
    bool _first;
    bool _doneOk;
    uint8_t _state;
    uint8_t _stateOld;
    bool _sendState;

    // Reset
    void reset (void);

    // Timeout
    void timeout (void);

    // Emint data
    void emit (void);

    // Callback functions
    Wiegand26::callback_data funcion_data;
    Wiegand26::callback_state funcion_state;

};

#endif