/*
 WCharacter.h - Character utility functions for Wiring & Arduino
 Copyright (c) 2010 Hernando Barragan.  All right reserved.
 Copyright (c) 2021 by Erik Tideman (Replaced 'boolean' with 'bool')

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef Character_h
#define Character_h

#include <ctype.h>

#if defined(_WIN32)
#define ALWAYS_INLINE __forceinline
#else
#define ALWAYS_INLINE inline __attribute__((always_inline))
#endif

 // WCharacter.h prototypes
ALWAYS_INLINE bool isAlphaNumeric(int c);
ALWAYS_INLINE bool isAlpha(int c);
ALWAYS_INLINE bool isAscii(int c);
ALWAYS_INLINE bool isWhitespace(int c);
ALWAYS_INLINE bool isControl(int c);
ALWAYS_INLINE bool isDigit(int c);
ALWAYS_INLINE bool isGraph(int c);
ALWAYS_INLINE bool isLowerCase(int c);
ALWAYS_INLINE bool isPrintable(int c);
ALWAYS_INLINE bool isPunct(int c);
ALWAYS_INLINE bool isSpace(int c);
ALWAYS_INLINE bool isUpperCase(int c);
ALWAYS_INLINE bool isHexadecimalDigit(int c);
ALWAYS_INLINE int toAscii(int c);
ALWAYS_INLINE int toLowerCase(int c);
ALWAYS_INLINE int toUpperCase(int c);


// Checks for an alphanumeric character.
// It is equivalent to (isalpha(c) || isdigit(c)).
inline bool isAlphaNumeric(int c)
{
  return ( isalnum(c) == 0 ? false : true);
}


// Checks for an alphabetic character.
// It is equivalent to (isupper(c) || islower(c)).
inline bool isAlpha(int c)
{
  return ( isalpha(c) == 0 ? false : true);
}


// Checks whether c is a 7-bit unsigned char value
// that fits into the ASCII character set.
inline bool isAscii(int c)
{
  return ( isascii (c) == 0 ? false : true);
}


// Checks for a blank character, that is, a space or a tab.
inline bool isWhitespace(int c)
{
  return ( isblank (c) == 0 ? false : true);
}


// Checks for a control character.
inline bool isControl(int c)
{
  return ( iscntrl (c) == 0 ? false : true);
}


// Checks for a digit (0 through 9).
inline bool isDigit(int c)
{
  return ( isdigit (c) == 0 ? false : true);
}


// Checks for any printable character except space.
inline bool isGraph(int c)
{
  return ( isgraph (c) == 0 ? false : true);
}


// Checks for a lower-case character.
inline bool isLowerCase(int c)
{
  return (islower (c) == 0 ? false : true);
}


// Checks for any printable character including space.
inline bool isPrintable(int c)
{
  return ( isprint (c) == 0 ? false : true);
}


// Checks for any printable character which is not a space
// or an alphanumeric character.
inline bool isPunct(int c)
{
  return ( ispunct (c) == 0 ? false : true);
}


// Checks for white-space characters. For the avr-libc library,
// these are: space, formfeed ('\f'), newline ('\n'), carriage
// return ('\r'), horizontal tab ('\t'), and vertical tab ('\v').
inline bool isSpace(int c)
{
  return ( isspace (c) == 0 ? false : true);
}


// Checks for an uppercase letter.
inline bool isUpperCase(int c)
{
  return ( isupper (c) == 0 ? false : true);
}


// Checks for a hexadecimal digits, i.e. one of 0 1 2 3 4 5 6 7
// 8 9 a b c d e f A B C D E F.
inline bool isHexadecimalDigit(int c)
{
  return ( isxdigit (c) == 0 ? false : true);
}


// Converts c to a 7-bit unsigned char value that fits into the
// ASCII character set, by clearing the high-order bits.
inline int toAscii(int c)
{
  return toascii (c);
}


// Warning:
// Many people will be unhappy if you use this function.
// This function will convert accented letters into random
// characters.

// Converts the letter c to lower case, if possible.
inline int toLowerCase(int c)
{
  return tolower (c);
}


// Converts the letter c to upper case, if possible.
inline int toUpperCase(int c)
{
  return toupper (c);
}

#endif
