#include "wiring_time.h"

void delay(uint32_t ms)
{
  if (ms != 0) {
    uint32_t start = HAL_GetTick();
    do {
    } while (HAL_GetTick() - start < ms);
  }
}
