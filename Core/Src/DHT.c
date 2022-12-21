#include <math.h>
#include <string.h>
#include "DHT.h"
#include "wiring_time.h"
#include "usart.h"
//#define NAN 0
#define TIMEOUT 25
#ifdef DEBUG
    #define DEBUG_PRINT(val) HAL_UART_Transmit(&huart2, val, sizeof(val), TIMEOUT)
#else
    #define DEBUG_PRINT(val)
#endif

void pinMode(uint8_t pin, uint32_t mode) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = pin;
    GPIO_InitStruct.Mode = mode;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void DHT_init(DHT* _this, uint8_t pin, uint8_t type, uint8_t count) {
    _this->_pin = pin;
    _this->_type = type;
    _this->_count = count;
    _this->firstreading = true;
}

void DHT_begin(DHT* _this) {

    // set up the pins!
    pinMode(_this->_pin, GPIO_MODE_INPUT);
    _this->_lastreadtime = 0;
    delayMicroseconds(50);
    DEBUG_PRINT("DHT initialized\n");

}

/** Common  interface to get temp&humi value.support all DHT device.

    @return 0 for calibrated failed,1 for succeed.
 **/
int DHT_readTempAndHumidity(DHT* _this, float* data) {
    data[0] = DHT_readHumidity(_this);
    data[1] = DHT_readTemperature(_this, false);
    if (isnan(data[0]) || isnan(data[1])) {
        return -1;
    }
    return 0;
}

//boolean S == Scale.  True == Farenheit; False == Celcius
float DHT_readTemperature(DHT* _this, bool S) {
    float f;
    switch (_this->_type) {
        case DHT11:
            f = _this->data[2];
            if(_this->data[3]%128<10){
                f += _this->data[3]%128/10.0f;
            }else if(_this->data[3]%128<100){
                f += _this->data[3]%128/100.0f;
            }else{
                f += _this->data[3]%128/1000.0f;
            }
            if(_this->data[3]>=128){ // The left-most digit indicate the negative sign.
                f = -f;
            }
            if (S) {
                f = DHT_convertCtoF(_this, f);
            }

            return f;
        case DHT22:
        case DHT21:
            f = _this->data[2] & 0x7F;
            f *= 256;
            f += _this->data[3];
            f /= 10;
            if (_this->data[2] & 0x80) {
                f *= -1;
            }
            if (S) {
                f = DHT_convertCtoF(_this, f);
            }

            return f;
    }
}

float DHT_convertCtoF(DHT* _this, float c) {
    return c * 9 / 5 + 32;
}

float DHT_readHumidity(DHT* _this) {
    float f;
    switch (_this->_type) {
        case DHT11:
            f = _this->data[0];
            return f;
        case DHT22:
        case DHT21:
            f = _this->data[0];
            f *= 256;
            f += _this->data[1];
            f /= 10;
            return f;
    }
}


bool DHT_read(DHT* _this) {
	pinMode(_this->_pin, GPIO_MODE_OUTPUT_PP);  // set the pin as output
	  HAL_GPIO_WritePin(GPIOA, _this->_pin, 0);   // pull the pin low

	  delayMicroseconds(18000);   // wait for 18ms

	  HAL_GPIO_WritePin(GPIOA, _this->_pin, 1);   // pull the pin high
	  delayMicroseconds(30);   // wait for 30us
	  pinMode(_this->_pin, GPIO_MODE_INPUT);    // set as input
    uint8_t Response = 0;
    delayMicroseconds(40);
  if (!(HAL_GPIO_ReadPin(GPIOA, _this->_pin)))
  {
    delayMicroseconds(80);
    if ((HAL_GPIO_ReadPin(GPIOA, _this->_pin))) Response = 1;
    else Response = -1;
  }
  while ((HAL_GPIO_ReadPin(GPIOA, _this->_pin)));   // wait for the pin to go low
    uint8_t data[5];
    for (size_t l = 0; l < 5; l++)
    {
        uint8_t i,j;
        for (j=0;j<8;j++)
        {
            while (!(HAL_GPIO_ReadPin (GPIOA, _this->_pin)));   // wait for the pin to go high
            delayMicroseconds (40);   // wait for 40 us
            if (!(HAL_GPIO_ReadPin (GPIOA, _this->_pin)))   // if the pin is low
            {
                i&= ~(1<<(7-j));   // write 0
            }
            else i|= (1<<(7-j));  // if the pin is high, write 1
            while ((HAL_GPIO_ReadPin (GPIOA, _this->_pin)));  // wait for the pin to go low
        }
        _this->data[l] = i;
    }
    // check we read 40 bits and that the checksum matches
    if ((_this->data[4] == ((_this->data[0] + _this->data[1] + _this->data[2] + _this->data[3]) & 0xFF))) {
        return true;
    }


    return false;

}
