#ifndef DHT_H
#define DHT_H
#include "main.h"

#define COUNT 6

// how many timing transitions we need to keep track of. 2 * number bits + extra
#define MAXTIMINGS 85

#define DHT11 11
#define DHT22 22
#define DHT21 21
#define AM2301 21

typedef struct DHT {
    uint8_t data[5];
    uint8_t _pin, _type, _count;
    unsigned long _lastreadtime;
    bool firstreading;
} DHT;

void DHT_init(DHT* _this, uint8_t pin, uint8_t type, uint8_t count);
void DHT_begin(DHT* _this);
float DHT_readTemperature(DHT* _this, bool S);
float DHT_convertCtoF(DHT* _this, float);
float DHT_readHumidity(DHT* _this);
bool DHT_read(DHT* _this);
/** Common  interface to get temp&humi value.support all DHT device.

    @return 0 for calibrated failed,1 for succeed.
 **/
int DHT_readTempAndHumidity(DHT* _this, float* data);

#endif
