/*
*  Wiegand26
*  by Jiri Vondrus (https://github.com/jvondrus)
*  Version 2.0.0 07-2020
*/

// Import
#include "Wiegand26.h"
#include "Arduino.h"



// Set bit in buffer
inline void setBit (bool* bits, uint8_t bit, bool value) {
  bits[MAX_BITS - bit - 1] = value;
}



// Read bit from buffer
inline bool readBit (bool* bits, uint8_t bit) {
  return bool (bits[MAX_BITS - bit - 1]);
}



// Align key (buffer >> value)
inline uint8_t alignKey (bool* bits) {
  uint8_t keyA = 0x00;
  uint8_t keyB = 0x0F;
  uint8_t o = MAX_BITS - KEY_BITS;
  uint8_t p = MAX_BITS - ( KEY_BITS / 2 );
  for (uint8_t i = o; i < MAX_BITS; i++) {
    if (i < p) {
      if (bits[i]) {
        bitSet (keyA, i-o);
      }
    } else {
      if (bits[i]) {
        bitClear (keyB, i-p);
      }
    }
  }
  if (keyA != keyB) {
    keyA = 0xFF;
  }
  return keyA;
}



// Align data (buffer >> value)
inline unsigned long alignData (bool* bits, bool swap) {
  unsigned long data = 0;
  for (uint8_t i = 1; i < MAX_BITS-1; i++) {
    if (bits[i]) {
      bitSet (data, (i - 1));
    }
  }
  if (swap) {
    data = (data & 0x0000FF) << 16 | (data & 0x00FF00) | (data & 0xFF0000) >> 16;
  }
  return data;
}



// Reset
void Wiegand26::reset () {
  // Reset buffer
  for (uint8_t i = 0; i < MAX_BITS; i++) {
    _bitData[i] = false;
  }

  // Reset state
  _state = 0;
  _bitCnt = 0;
  _keyCnt = 0;
  _timestamp = millis();
  _elapsed = 0;

  // Read Pin State
  _stateData0 = !digitalRead (_pinData0);
  _stateData1 = !digitalRead (_pinData1);

  // Check state
  bitSet (_state, INICIALIZE);
  if (_stateData0 && _stateData1) {
    bitSet (_state, CONNECTION);
  }

  // Send state
  if (_first) {
    if (function_state) {
      _first = false;
      _stateOld = _state;
      function_state (_state);
    }
  }
}



// Emint Key / Code
void Wiegand26::emitKey () {

  // Send key
  _key = alignKey (_bitData);
  if (_key <= 9) {
    _code = _code * 10 + _key;
    if (function_key) {
      function_key (_key);
      bitSet (_state, DATA_SENDED);
    }

  // Send Code
  } else if (_key == KEY_SEND) {
    if (_code && _code <= 0xFFFFFF && function_code) {
      function_code (_code);
      bitSet (_state, DATA_SENDED);
    }
    _code = 0;
  }

  // Reset and Send state
  if (_key <= 9 || _key == KEY_SEND) {
    if (function_state) {
      if (_state != _stateOld || _sendState) {
        _stateOld = _state;
        function_state (_state);
        _state = 0;
      }
    }
    reset ();
  }

}



// Emint data
void Wiegand26::emitData () {

  // Buffer size error
  if (_bitCnt != MAX_BITS) {
    bitSet (_state, BITS_FAULTS);
  }

  // Calc parity
  bool parityFirst = false;
  bool paritySecond = false;
  for (uint8_t i = 0; i < (MAX_BITS+1)/2; i++) {
    parityFirst = (parityFirst != readBit (_bitData, i));
  }
  for (uint8_t i = MAX_BITS/2; i < MAX_BITS; i++) {
    paritySecond = (paritySecond != readBit (_bitData, i));
  }

  // Check parity
  if (parityFirst != EVEN) {
    bitSet (_state, PARITY_FRST);
  }
  if (paritySecond != ODD) {
    bitSet (_state, PARITY_SCND);
  }

  // Send data
  if (parityFirst == EVEN && paritySecond == ODD) {
    if (function_data) {
      _data = alignData (_bitData, _swapData);
      function_data (_data);
      bitSet (_state, DATA_SENDED);
      _code = 0;
    }
  }

  // Send state
  if (function_state) {
    if (_state != _stateOld || _sendState) {
      _stateOld = _state;
      function_state (_state);
      _state = 0;
    }
  }
}



// Check buffer
void Wiegand26::readState () {
  // Send state
  if (function_state) {
    _stateOld = _state;
    function_state (_state);
  }
}



// Timeout
void Wiegand26::timeout () {
  // Check timeout
  _elapsed = millis() - _timestamp;

  // Code
  if (_elapsed > TIMEOUTKEY) {
    _code = 0;
  }

  // Send
  if (_elapsed > TIMEOUT) {
    if (_bitCnt) {
      bitSet (_state, RCV_TIMEOUT);
      emitData ();
    }
    reset ();
  }
}



// Reading pin
void Wiegand26::readData () {
  // Read Pin State
  _stateData0 = !digitalRead (_pinData0);
  _stateData1 = !digitalRead (_pinData1);

  // Check for timemout
  timeout ();

  // Update timestamp
  _timestamp = millis();

  // Read Logical 0
  if (_stateData0 && !_stateData1) {
    setBit (_bitData, _bitCnt, false);
  // Read Logical 1
  } else if (!_stateData0 && _stateData1) {
    setBit (_bitData, _bitCnt, true);
  } else {
    bitSet (_state, LOGIC_FAULT);
  }
  _bitCnt++;
  _keyCnt++;

  // Key done
  if (_keyCnt == KEY_BITS) {
    emitKey ();
  }

  // Reading done
  if (_bitCnt >= MAX_BITS) {
    emitData ();
    reset ();
  }

}



// Begin
void Wiegand26::begin (uint8_t pinData0, uint8_t pinData1, bool state, bool swapData) {
  // Save pin
  _pinData0 = pinData0;
  _pinData1 = pinData1;

  // Set pin mode
  pinMode (_pinData0, INPUT);
  pinMode (_pinData1, INPUT);

  // Inicialize
  _sendState = state;
  _swapData = swapData;
  _first = true;
  _stateOld = 0;
  _state = 0;
  _code = 0;
  reset ();
}

