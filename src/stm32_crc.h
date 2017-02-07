

#ifndef STM32_CRC_H__
#define STM32_CRC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void stm32crc_init();
uint32_t stm32crc_crc32(uint8_t *src, size_t len);


#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* STM32_CRC_H__ */

