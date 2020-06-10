/*
*  Wiegand26
*  by Jiri Vondrus (https://github.com/jvondrus)
*  Version 1.1.0 06-2020
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



// Align data (buffer >> value)
inline unsigned long alignData (bool* bits) {
  unsigned long data = 0;
  for (uint8_t i = 1; i < MAX_BITS-1; i++) {
    if (bits[i]) {
      bitSet (data, (i - 1));
    }
  }
  data = (data & 0x0000FF) << 16 | (data & 0x00FF00) | (data & 0xFF0000) >> 16;
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
    if (funcion_state) {
      _first = false;
      _stateOld = _state;
      funcion_state (_state);
    }
  }
}



// Emint data
void Wiegand26::emit () {

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
    if (funcion_data) {
      _data = alignData (_bitData);
      funcion_data (_data);
      bitSet (_state, DATA_SENDED);
    }
  }

  // Send state
  if (funcion_state) {
    if (_state != _stateOld || _sendState) {
      _stateOld = _state;
      funcion_state (_state);
      _state = 0;
    }
  }
}



// Check buffer
void Wiegand26::readState () {
  // Send state
  if (funcion_state) {
    _stateOld = _state;
    funcion_state (_state);
  }
}



// Timeout
void Wiegand26::timeout () {
  // Check timeout
  _elapsed = millis() - _timestamp;

  // Send
  if (_elapsed > TIMEOUT) {
    if (_bitCnt) {
      bitSet (_state, RCV_TIMEOUT);
      emit ();
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

  // Reading done
  if (_bitCnt >= MAX_BITS) {
    emit ();
    reset ();
  }

}



// Begin
void Wiegand26::begin (uint8_t pinData0, uint8_t pinData1, bool state) {
  // Save pin
  _pinData0 = pinData0;
  _pinData1 = pinData1;

  // Set pin mode
  pinMode (_pinData0, INPUT);
  pinMode (_pinData1, INPUT);

  // Inicialize
  _sendState = state;
  _first = true;
  _stateOld = 0;
  _state = 0;
  reset ();
}

