/*
 * ambient.h - Library for sending data to Ambient
 * Created by Takehiko Shimojima, April 21, 2016
 */
#ifndef Ambient_h
#define Ambient_h

#include "Arduino.h"
#include <ESP8266WiFi.h>

#define AMBIENT_WRITEKEY_SIZE 18
#define AMBIENT_MAX_RETRY 5
#define AMBIENT_DATA_SIZE 24
#define AMBIENT_NUM_PARAMS 11

class Ambient
{
public:

    Ambient(void);

    void begin(unsigned int channelId, const char * writeKey, WiFiClient * c, int dev = 0);
    bool set(int field, char * data);
    bool clear(int field);

    bool send();

private:

    WiFiClient * client;
    unsigned int channelId;
    char writeKey[AMBIENT_WRITEKEY_SIZE];
    int dev;
    char host[18];
    int port;

    struct {
        int set;
        char item[AMBIENT_DATA_SIZE];
    } data[AMBIENT_NUM_PARAMS];
};

#endif // Ambient_h
