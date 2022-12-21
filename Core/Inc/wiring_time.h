#ifndef _WIRING_TIME_H_
#define _WIRING_TIME_H_

#include "main.h"
#include "tim.h"

/**
 * \brief Pauses the program for the amount of time (in milliseconds) specified as parameter.
 * (There are 1000 milliseconds in a second.)
 *
 * \param ms the number of milliseconds to pause (uint32_t)
 */
extern void delay(uint32_t ms) ;

/**
 * \brief Pauses the program for the amount of time (in microseconds) specified as parameter.
 *
 * \param us the number of microseconds to pause (uint32_t)
 */
static inline void delayMicroseconds(uint32_t) __attribute__((always_inline, unused));
static inline void delayMicroseconds(uint32_t us)
{
  __HAL_TIM_SET_COUNTER(&htim21,0);  // set the counter value a 0
  while (__HAL_TIM_GET_COUNTER(&htim21) < us);  // wait for the counter to reach the us input in the parameter
}

#endif /* _WIRING_TIME_H_ */
