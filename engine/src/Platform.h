
#ifndef ENGINE_PLATFORM_H
#define ENGINE_PLATFORM_H

class IPlatformEventHandler;

#include <string>

/*
  common definitions for platform specific code.
  implemented in android/platform.cpp
  or ios/platform.m
*/
namespace Platform
{
    void setHandler(IPlatformEventHandler * handler);

    /**
     * Time in seconds, with sub-millisecond precision, since some unspecified point in the past.
     * Not a valid wall clock time, use for timing intervals only
     */
    double getTime();

    void readFile(const std::string& path, std::string * out);

    int width();
    int height();

}

#endif
