#ifndef STATIC_CONTENT_H
#define STATIC_CONTENT_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

AsyncWebHandler* registerStaticFiles(AsyncWebServer* webServer);

#endif