/*
 * Copyright (c) 2019 Brian T. Park
 * MIT License
 */

#include <stdio.h>
#include "StdioSerial.h"

size_t StdioSerial::write(uint8_t c) {
#if defined(_WIN32)
  DWORD bytesWritten;
  BOOL writeStatus = WriteFile(outputFd, &c, 1, &bytesWritten, NULL);
  size_t status = writeStatus & (bytesWritten > 0);
#else
  ssize_t status = ::write(outputFd, &c, 1);
#endif
  return (status <= 0) ? 0 : 1;
}

int StdioSerial::read() {
  int ch = peek();
  bufch = -1;
  return ch;
}

int StdioSerial::peek() {
  if (bufch == -1) {
    // 'c' must be unsigned to avoid ambiguity with -1 in-band error condition
    unsigned char c;
#if defined(_WIN32)
    DWORD bytesRead;
    BOOL readStatus = ReadFile(GetStdHandle(STD_INPUT_HANDLE), &c, 1, &bytesRead, NULL);
    int status = readStatus & (bytesRead > 0);
#else
    ssize_t status = ::read(STDIN_FILENO, &c, 1);
#endif
    bufch = (status <= 0) ? -1 : c;
  }
  return bufch;
}

int StdioSerial::available() {
  int ch = peek();
  return (int) (ch != -1);
}

StdioSerial Serial;
