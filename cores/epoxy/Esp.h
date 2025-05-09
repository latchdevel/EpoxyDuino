#ifndef EPOXY_DUINO_ESP_H
#define EPOXY_DUINO_ESP_H

#if defined(EPOXY_CORE_ESP8266)

#if defined(_WIN32)
#include <Windows.h>
#else
#include <sys/time.h>
#endif
#include "Stream.h"

class EspClass
{
  public:
    EspClass() {
      gettimeofday(&start, NULL);
    }

    void reset() {}

    void restart() {}

    // Very ugly approximation, this is freeStack
    unsigned long getFreeHeap() {
      int i;
      static int* h=40000+&i;
      return h-&i;
    }

    uint32_t getCpuFreqMHZ() { return 80; }

    uint32_t getChipId() { return 0; }

    uint32_t getCycleCount() {
      struct timeval now;
      gettimeofday(&now, NULL);
      return getCpuFreqMHZ()
          * ((now.tv_sec-start.tv_sec)*1000000+(now.tv_usec-start.tv_usec));
    }

  private:
    struct timeval start;
#if defined(_WIN32)
    void gettimeofday(struct timeval *tv, void* tz) {
      FILETIME ft;
      unsigned __int64 tmpres = 0;
      static const unsigned __int64 DELTA_EPOCH_IN_MICROSECS = 11644473600000000UL;

      GetSystemTimeAsFileTime(&ft);

      tmpres |= ft.dwHighDateTime;
      tmpres <<= 32;
      tmpres |= ft.dwLowDateTime;
      tmpres /= 1;
      tmpres -= DELTA_EPOCH_IN_MICROSECS;

      tv->tv_sec = static_cast<long>(tmpres / 1000000UL);
      tv->tv_usec = static_cast<long>(tmpres % 1000000UL);
    }
#endif
};

class Serial_ : public Stream
{
};

extern EspClass ESP;

#endif
#endif
