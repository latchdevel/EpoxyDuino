/*
 * Copyright (c) 2019 Brian T. Park
 *
 * Parts derived from the Arduino SDK
 * Copyright (c) 2005-2013 Arduino Team
 *
 * Parts inspired by [Entering raw
 * mode](https://viewsourcecode.org/snaptoken/kilo/02.enteringRawMode.html).
 *
 * Parts inspired by [ESP8266 Host
 * Emulation](https://github.com/esp8266/Arduino/tree/master/tests/host).
 *
 * The 'Serial' object sends output to STDOUT, and receives input from STDIN in
 * 'raw' mode. The main() loop checks the STDIN and if it finds a character,
 * inserts it into the Serial buffer.
 */

#ifdef EPOXY_DUINO

#include "Arduino.h"
#include <inttypes.h>
#include <signal.h> // SIGINT
#include <stdlib.h> // exit()
#include <stdio.h> // perror()
#if defined(_WIN32)
#include <Windows.h>
#else
#include <unistd.h> // isatty(), STDIN_FILENO, STDOUT_FILENO
#include <fcntl.h>
#include <termios.h>
#endif

// -----------------------------------------------------------------------
// Unix compatibility. Put STDIN into raw mode and hook it into the 'Serial'
// object. Trap Ctrl-C and perform appropriate clean up.
// -----------------------------------------------------------------------

#if defined(_WIN32)
static HANDLE hStdin = INVALID_HANDLE_VALUE;
static HANDLE hStdout = INVALID_HANDLE_VALUE;
static DWORD orig_stdin_mode = 0; 
static DWORD orig_stdout_mode = 0;
#else
static struct termios orig_termios;
static int orig_stdin_flags;
#endif
static bool inRawMode = false;
static bool inNonBlockingMode = false;

static void die(const char* s) {
  perror(s);
  exit(1);
}

static void disableRawMode() {
  if (inRawMode) {
#if defined(_WIN32)
    if (!SetConsoleMode(hStdout, orig_stdout_mode)) {
      perror("disableRawMode(): SetConsoleMode(hStdout) failure");
    }
    if (!SetConsoleMode(hStdin, orig_stdin_mode)) {
      perror("disableRawMode(): SetConsoleMode(hStdin) failure");
#else
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) {
      perror("disableRawMode(): tcsetattr() failure");
#endif
      inRawMode = false; // prevent exit(1) from being called twice
    }
  }

  if (inNonBlockingMode) {
#if defined(_WIN32)
    inNonBlockingMode = false;
#else
    if (fcntl(STDIN_FILENO, F_SETFL, orig_stdin_flags) == -1) {
      inNonBlockingMode = false; // prevent exit(1) from being called twice
      perror("enableRawMode(): fcntl() failure");
    }
#endif
  }
}

#if defined(_WIN32)
BOOL WINAPI ConsoleHandler(DWORD signal) {
  if (signal == CTRL_C_EVENT || signal == CTRL_CLOSE_EVENT) {
    disableRawMode();
	  return FALSE;  // OS trap Ctrl-C
  }
  return TRUE;
}
#endif

static void enableRawMode() {
#if defined(_WIN32)
  hStdin = GetStdHandle(STD_INPUT_HANDLE);
  hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
  DWORD mode;

  // Enter raw mode only if STD_INPUT_HANDLE is a terminal. If the input is a file or a
  // pipe (or even /dev/null), then raw mode does not make sense.
  //
  // In addition, enter raw mode only if STD_OUTPUT_HANDLE is also a terminal. If the
  // output is a file or a pipe, it could be piping to a pager, like the less(1)
  // program which wants to handle keyboard inputs in raw mode by itself.
  if (GetConsoleMode(hStdin, &mode) && GetConsoleMode(hStdout, &mode)) {

    if (!GetConsoleMode(hStdin, &orig_stdin_mode)) {
      die("enableRawMode(): GetConsoleMode(hStdin) failure");
    }

    DWORD rawInMode = orig_stdin_mode;
    rawInMode &= ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT);

    if (!SetConsoleMode(hStdin, rawInMode)) {
      die("enableRawMode(): SetConsoleMode(hStdin) failure");
    }

    if (!GetConsoleMode(hStdout, &orig_stdout_mode)) {
      die("enableRawMode(): GetConsoleMode(hStdout) failure");
    }

    DWORD rawOutMode = orig_stdout_mode;
    rawOutMode |= ENABLE_PROCESSED_OUTPUT;
#ifdef ENABLE_VIRTUAL_TERMINAL_PROCESSING
    rawOutMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
#endif
    if (!SetConsoleMode(hStdout, rawOutMode)) {
      die("enableRawMode(): SetConsoleMode(hStdout) failure");
    }

    inRawMode = true;
    inNonBlockingMode = true;
  }
#else
  // Enter raw mode only if STDIN is a terminal. If the input is a file or a
  // pipe (or even /dev/null), then raw mode does not make sense.
  //
  // In addition, enter raw mode only if STDOUT is also a terminal. If the
  // output is a file or a pipe, it could be piping to a pager, like the less(1)
  // program which wants to handle keyboard inputs in raw mode by itself.
  //
  // I believe this fixes https://github.com/bxparks/EpoxyDuino/issues/2 and
  // https://github.com/bxparks/EpoxyDuino/issues/25 finally.
  if (isatty(STDOUT_FILENO) && isatty(STDIN_FILENO)) {

    // Save the original config.
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
      die("enableRawMode(): tcgetattr() failure");
    }

    // The 'Enter' key in raw mode is ^M (\r, CR). But internally, we want this
    // to be ^J (\n, NL), so ICRNL and INLCR causes the ^M to become a \n.
    struct termios raw = orig_termios;
    raw.c_iflag &= ~(/*ICRNL | INLCR |*/ INPCK | ISTRIP | IXON);

    // Set the output into cooked mode, to handle NL and CR properly.
    // Print.println() sends CR-NL (\r\n). But some code will send just \n. The
    // ONLCR translates \n into \r\n. So '\r\n' will become \r\r\n, which is
    // just fine.
    raw.c_oflag |= (OPOST | ONLCR);
    raw.c_cflag |= (CS8);

    // Enable ISIG to allow Ctrl-C to kill the program.
    raw.c_lflag &= ~(ECHO | /*ISIG |*/ ICANON | IEXTEN);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
      die("enableRawMode(): tcsetattr() failure");
    }

    inRawMode = true;
  }

  // Always set input into non-blocking mode so that yield() does not block when
  // it reads one character, emulating a read from the Serial port.
  orig_stdin_flags = fcntl(STDIN_FILENO, F_GETFL, 0);
  if (fcntl(STDIN_FILENO, F_SETFL, orig_stdin_flags | O_NONBLOCK) == -1) {
    die("enableRawMode(): fcntl() failure");
  }
  inNonBlockingMode = true;
#endif
}

// -----------------------------------------------------------------------
// Main loop. User code will provide setup() and loop().
// -----------------------------------------------------------------------

extern "C" {

int epoxy_argc;

const char* const* epoxy_argv;

static int epoxyduino_main(int argc, char** argv) {
  epoxy_argc = argc;
  epoxy_argv = argv;

#if defined(_WIN32)
  SetConsoleCtrlHandler(ConsoleHandler, TRUE);
#else
  atexit(disableRawMode);
#endif
  enableRawMode();

  setup();
  while (true) {
    loop();
    yield();
  }
}

void enableTerminalEcho() {
#if defined(_WIN32)
  HANDLE hStdinLocal = GetStdHandle(STD_INPUT_HANDLE);
  DWORD mode;
  if (!GetConsoleMode(hStdinLocal, &mode)) {
    return;
  }
  mode |= ENABLE_ECHO_INPUT;
  if (!SetConsoleMode(hStdinLocal, mode)) {
    return;
  }
#else
  struct termios term;
  if (tcgetattr(STDIN_FILENO, &term) == -1) {
    return;
  }
  term.c_lflag |= ECHO;
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &term) == -1) {
    return;
  }
#endif
}

// Weak reference so that the calling code can provide its own main().
#if defined(_WIN32)
#pragma comment(linker, "/alternatename:main=weak_main")
int weak_main(int argc, char** argv);
#else
int main(int argc, char** argv)
__attribute__((weak));
#endif

int main(int argc, char** argv) {
  return epoxyduino_main(argc, argv);
}

}

#endif // #ifdef EPOXY_DUINO
