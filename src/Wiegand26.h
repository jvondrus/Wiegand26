/*
*  Wiegand26
*  by Jiri Vondrus (https://github.com/jvondrus)
*  Version 2.0.0 07-2020
*/

// Define
#ifndef Wiegand26_h
#define Wiegand26_h

// Import
#include <stdint.h>

// Setings
#define MAX_BITS      26
#define KEY_BITS      8
#define KEY_SEND      11
#define TIMEOUT       20
#define TIMEOUTKEY    5000

// Parity
#define EVEN          false
#define ODD           true

// States
#define INICIALIZE    0
#define DATA_SENDED   1
#define CONNECTION    2
#define LOGIC_FAULT   3
#define RCV_TIMEOUT   4
#define BITS_FAULTS   5
#define PARITY_FRST   6
#define PARITY_SCND   7


// Class
class Wiegand26
{

  // Public part
  public:

    // Inicialize
    Wiegand26 () { }
    Wiegand26 (uint8_t pinData0, uint8_t pinData1, bool status=true, bool swapData=false) {
      begin (pinData0, pinData1);
    }
    void begin (uint8_t pinData0, uint8_t pinData1, bool status=true, bool swapData=false);

    // Reading data
    void readData (void);

    // Reading state
    void readState (void);

    // Emit key
    void onKey (void (*func) (uint8_t key)) {
      function_key = (callback_key)func;
    }

    // Emit code
    void onCode (void (*func) (unsigned long code)) {
      function_code = (callback_code)func;
    }

    // Emit data
    void onData (void (*func) (unsigned long data)) {
      function_data = (callback_data)func;
    }

    // Emit State
    void onState (void (*func) (uint8_t state)) {
      function_state = (callback_state)func;
    }

    // Type definition
    typedef void (*callback_key) (uint8_t key);
    typedef void (*callback_code) (unsigned long code);
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
    uint8_t _keyCnt;
    bool _bitData[MAX_BITS];
    uint8_t _key;
    unsigned long _code;
    unsigned long _data;

    // State
    bool _first;
    uint8_t _state;
    uint8_t _stateOld;
    bool _sendState;
    bool _swapData;

    // Reset
    void reset (void);

    // Timeout
    void timeout (void);

    // Emit Key data
    void emitKey (void);

    // Emint data
    void emitData (void);

    // Callback functions
    Wiegand26::callback_key function_key;
    Wiegand26::callback_code function_code;
    Wiegand26::callback_data function_data;
    Wiegand26::callback_state function_state;

};

#endif