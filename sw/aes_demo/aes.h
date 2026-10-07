#ifndef AES_H
#define AES_H

#include <pulpino.h>

#define AES_BASE_ADDR       ( SOC_PERIPHERALS_BASE_ADDR + 0x8000 )

#define AES_NAME0           REG(AES_BASE_ADDR + 0x00)
#define AES_NAME1           REG(AES_BASE_ADDR + 0x04)
#define AES_VERSION         REG(AES_BASE_ADDR + 0x08)
#define AES_CTRL            REG(AES_BASE_ADDR + 0x20)
#define AES_STATUS          REG(AES_BASE_ADDR + 0x24)
#define AES_CONFIG          REG(AES_BASE_ADDR + 0x28)
#define AES_KEY(i)          REG(AES_BASE_ADDR + 0x40 + 4 * (i))
#define AES_BLOCK(i)        REG(AES_BASE_ADDR + 0x80 + 4 * (i))
#define AES_RESULT(i)       REG(AES_BASE_ADDR + 0xC0 + 4 * (i))

#define AES_CTRL_INIT       0x1
#define AES_CTRL_NEXT       0x2

#define AES_STATUS_READY    0x1
#define AES_STATUS_VALID    0x2

#define AES_CONFIG_ENCRYPT  0x1
#define AES_CONFIG_KEY256   0x2

#endif
