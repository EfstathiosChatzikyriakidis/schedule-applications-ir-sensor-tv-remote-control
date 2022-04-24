/*
 *  Schedule Applications IR Sensor & TV Remote Control.
 *
 *  Copyright (C) 2010  Efstathios Chatzikyriakidis (stathis.chatzikyriakidis@gmail.com)
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

// include some external libraries' headers.
#include <Streaming.h>
#include <IRremote.h>
#include <EEPROM.h>

// include some local libraries' headers.
#include "eeprom_generic.h"

const uint8_t buttonPin = 2; // the pin number of the button.
const uint8_t ledPin = 13;   // the pin number of the status led.
const uint8_t irRecvPin = 3; // the pin number of the IR receiver.

IRrecv irrecv (irRecvPin); // create IR receiver.
decode_results results;    // create IR decoder.

// serial data rate: bits per second.
const uint32_t SERIAL_BAUD_RATE = 9600;

// define bounce time for remote keys.
const uint32_t REMOTE_KEY_DURATION = 1000;

// define bounce time for buttons.
const uint32_t BUTTON_KEY_DURATION = 200;

// ms count to debounce pressed remote keys.
uint32_t remoteKeyBounceTime = 0;

// ms count to debounce pressed buttons.
uint32_t buttonKeyBounceTime = 0;

// data type definition for a remote key.
typedef struct rKeyT {
  uint32_t code; // the code of the key.
} rKeyT;

// array for the known remote keys.
rKeyT remoteKeys[] = {
  { 0 }, // 0.
  { 0 }, // 1.
  { 0 }, // 2.
  { 0 }, // 3.
  { 0 }, // 4.
  { 0 }, // 5.
  { 0 }, // 6.
  { 0 }, // 7.
  { 0 }, // 8.
  { 0 }  // 9.
};

// maximum number of remote keys.
const int8_t REMOTE_KEYS =
  (int8_t) (sizeof (remoteKeys) / sizeof (const rKeyT));

// check if a remote key exists.
int8_t
remoteKeyExists (decode_results * rs) {
  // get the type of the remote key.
  int16_t kType = rs->decode_type;

  // if the remote key type is not unknown.
  if (kType != UNKNOWN) {
    // get the value of the remote key.
    uint32_t kValue = rs->value;

    // if the type of the remote key is RC5/6.
    if (kType == RC5 || kType == RC6) {
      // get the length (bits) of the remote key.
      int16_t kLength = rs->bits;

      // calculate the toggle value of the remote key.
      uint32_t tValue = kValue ^ (1 << (kLength - 1));

      // searching the remote keys array.
      for (int8_t i = 0; i < REMOTE_KEYS; i++) {
        // if the remote key or the toggle value of it exist.
        if (remoteKeys[i].code == kValue || remoteKeys[i].code == tValue)
          // return the index of it.
          return i;
      }
    }
    else
      // searching the remote keys array.
      for (int8_t i = 0; i < REMOTE_KEYS; i++)
        // if the remote key exists.
        if (remoteKeys[i].code == kValue)
          // return the index of it.
          return i;
  }

  // remote key does not exist.
  return -1;
}

// try to handle a remote key.
void
handleRemoteKey (decode_results * rs) {
  // if a remote key is pressed.
  if (irrecv.decode (rs)) {
    // get the type of the remote key.
    int16_t kType = rs->decode_type;

    // if the remote key type is not unknown.
    if (kType != UNKNOWN) {
      // check if the remote key exists.
      int8_t index = remoteKeyExists (rs);

      // if the remote key exists and there is an acceptable timeslot.
      if (index >= 0 && (millis () - remoteKeyBounceTime > REMOTE_KEY_DURATION)) {
        // print the index of the remote key.
        Serial << index << endl;

        // set whatever bounce time in ms is appropriate.
        remoteKeyBounceTime = millis ();
      }
    }

    // now accept the next remote key.
    irrecv.resume ();
  }
}

// try to learn new remote keys.
void
learnRemoteKeys (decode_results * rs) {
  int8_t i = 0; // remote keys array index.

  // if the button is pressed.
  if (digitalRead (buttonPin)) {
    // while the are unlearned remote keys.
    while (i < REMOTE_KEYS) {
      // if remote key is pressed.
      if (irrecv.decode (rs)) {
        // get the type of the remote key.
        int16_t kType = rs->decode_type;

        // if the remote key type is not unknown.
        if (kType != UNKNOWN) {
          // get the value of the remote key.
          uint32_t kValue = rs->value;

          // light the status led.
          digitalWrite (ledPin, HIGH);

          // store the remote key.
          remoteKeys[i++].code = kValue;

          // wait some time.
          delay (REMOTE_KEY_DURATION);

          // dark the led.
          digitalWrite (ledPin, LOW);
        }

        // now accept the next remote key.
        irrecv.resume ();
      }
    }

    // save the remote keys to EEPROM.
    saveRemoteKeys ();
  }
}

// load remote keys from EEPROM.
void
loadRemoteKeys () {
  EEPROMGenericRead (0, remoteKeys);
}

// save remote keys to EEPROM.
void
saveRemoteKeys () {
  EEPROMGenericWrite (0, remoteKeys);
}

// startup point entry (runs once).
void
setup () {
  // set the serial baud rate.
  Serial.begin (SERIAL_BAUD_RATE);

  // set the status pin as output.
  pinMode (ledPin, OUTPUT);

  // set the button pin as input.
  pinMode (buttonPin, INPUT);

  // start the IR receiver.
  irrecv.enableIRIn ();

  // load remote keys from EEPROM.
  loadRemoteKeys ();
}

// loop the main sketch.
void
loop () {
  // try to learn new remote keys.
  learnRemoteKeys (&results);

  // try to handle a remote key.
  handleRemoteKey (&results);
}
