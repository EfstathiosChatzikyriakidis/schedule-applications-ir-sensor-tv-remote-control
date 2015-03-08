/*
 *  This file defines / implements EEPROM generic template functions.
 *
 *  Copyright (C) 2010  Efstathios Chatzikyriakidis (contact@efxa.org)
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

// header defining the interface of the source.
#ifndef _EEPROM_GENERIC_H
#define _EEPROM_GENERIC_H

// include Arduino basic header.
#include <Arduino.h>

// write to EEPROM a data structure.
template <class T> int
EEPROMGenericWrite (int addr, const T & value) {
  const byte *p = (const byte *) (const void *) &value;

  int i;
  for (int i = 0; i < sizeof (value); i++)
    EEPROM.write (addr++, *p++);

  return i;
}

// read from EEPROM a data structure.
template <class T> int
EEPROMGenericRead (int addr, T & value) {
  byte *p = (byte *) (void *) &value;

  int i;
  for (i = 0; i < sizeof (value); i++)
    *p++ = EEPROM.read (addr++);

  return i;
}

#endif // _EEPROM_GENERIC_H
