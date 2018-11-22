/*
 * ambient.h - Library for sending data to Ambient
 * Created by Takehiko Shimojima, April 21, 2016
 */
#include "Ambient.h"

//#define _DEBUG 1

#if AMBIENT_DEBUG
#define DBG(...) { Serial.print(__VA_ARGS__); }
#define ERR(...) { Serial.print(__VA_ARGS__); }
#else
#define DBG(...)
#define ERR(...)
#endif /* AMBIENT_DBG */

const char* AMBIENT_HOST = "54.65.206.59";
int AMBIENT_PORT = 80;
const char* AMBIENT_HOST_DEV = "192.168.0.8";
int AMBIENT_PORT_DEV = 4567;

const char * ambient_keys[] = {"\"d1\":\"", "\"d2\":\"", "\"d3\":\"", "\"d4\":\"", "\"d5\":\"", "\"d6\":\"", "\"d7\":\"", "\"d8\":\"", "\"lat\":\"", "\"lng\":\"", "\"created\":\""};

Ambient::Ambient() {
}

void
Ambient::begin(unsigned int channelId, const char * writeKey, WiFiClient * c, int dev) {
    this->channelId = channelId;

    if (sizeof(writeKey) > AMBIENT_WRITEKEY_SIZE) {
        ERR("writeKey length > AMBIENT_WRITEKEY_SIZE");
    }
    strcpy(this->writeKey, writeKey);

    if(NULL == c) {
        ERR("Socket Pointer is NULL, open a socket.");
    }
    this->client = c;
    this->dev = dev;
    if (dev) {
        strcpy(this->host, AMBIENT_HOST_DEV);
        this->port = AMBIENT_PORT_DEV;
    } else {
        strcpy(this->host, AMBIENT_HOST);
        this->port = AMBIENT_PORT;
    }
    for (int i = 0; i < AMBIENT_NUM_PARAMS; i++) {
        this->data[i].set = false;
    }
}

bool
Ambient::set(int field, char * data) {
    --field;
    if (field < 0 || field >= AMBIENT_NUM_PARAMS) {
        return false;
    }
    if (strlen(data) > AMBIENT_DATA_SIZE) {
        return false;
    }
    this->data[field].set = true;
    strcpy(this->data[field].item, data);

    return true;
}

bool
Ambient::clear(int field) {
    --field;
    if (field < 0 || field >= AMBIENT_NUM_PARAMS) {
        return false;
    }
    this->data[field].set = false;

    return true;
}

bool
Ambient::send() {

    int retry;
    for (retry = 0; retry < AMBIENT_MAX_RETRY; retry++) {
        int ret;
        ret = this->client->connect(this->host, this->port);
        if (ret) {
            break ;
        }
    }
    if(retry == AMBIENT_MAX_RETRY) {
        ERR("Could not connect socket to host\r\n");
        return false;
    }

    char str[360] = {0};
    char header[54] = {0};
    char host[32] = {0};
    char contentLen[28] = {0};
    const char *contentType = "Content-Type: application/json\r\n\r\n";
    char body[192] = {0};
    char inChar;

    strcat(body, "{\"writeKey\":\"");
    strcat(body, this->writeKey);
    strcat(body, "\",");

    for (int i = 0; i < AMBIENT_NUM_PARAMS; i++) {
        if (this->data[i].set) {
            strcat(body, ambient_keys[i]);
            strcat(body, this->data[i].item);
            strcat(body, "\",");
        }
    }
    body[strlen(body) - 1] = '\0';

    strcat(body, "}\r\n");

    sprintf(header, "POST /api/v2/channels/%d/data HTTP/1.1\r\n", this->channelId);
    if (this->port == 80) {
        sprintf(host, "Host: %s\r\n", this->host);
    } else {
        sprintf(host, "Host: %s:%d\r\n", this->host, this->port);
    }
    sprintf(contentLen, "Content-Length: %d\r\n", strlen(body));
    sprintf(str, "%s%s%s%s%s", header, host, contentLen, contentType, body);

    DBG("sending: ");
    DBG(strlen(str));
    DBG(" bytes\n");
    DBG(str);

    int ret;
    ret = this->client->print(str);
    delay(30);
    DBG(ret);
    DBG(" bytes sent\n\n");
    if (ret == 0) {
        ERR("send failed\n");
        return false;
    }

    while (this->client->available()) {
      inChar = this->client->read();
#if AMBIENT_DEBUG
      Serial.write(inChar);
#endif
    }

    this->client->stop();

    for (int i = 0; i < AMBIENT_NUM_PARAMS; i++) {
        this->data[i].set = false;
    }

    return true;
}
