/* -----------------------------------------------------------------------------
 * This file is a part of the UNIVIO project: https://github.com/nvitya/univio
 * Copyright (c) 2022 Viktor Nagy, nvitya
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software. Permission is granted to anyone to use this
 * software for any purpose, including commercial applications, and to alter
 * it and redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software in
 *    a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 * --------------------------------------------------------------------------- */
/*
 *  file:     uio_common.h
 *  brief:    UNIVIO Common Definitions
 *  date:     2024-09-10
 *  authors:  nvitya
*/

#ifndef UIOCORE_UIO_COMMON_H_
#define UIOCORE_UIO_COMMON_H_

#include "platform.h"  // includes board.h

#ifndef UIO_UART_COUNT
  #define UIO_UART_COUNT  0
#endif

#ifndef UIO_I2C_COUNT
  #define UIO_I2C_COUNT   0
#endif

#ifndef UIO_SPI_COUNT
  #define UIO_SPI_COUNT   0
#endif

#ifndef UIO_CAN_COUNT
  #define UIO_CAN_COUNT   0
#endif

#ifndef UIO_SPIFLASH_COUNT
  #define UIO_SPIFLASH_COUNT  0
#endif

#endif /* UIOCORE_UIO_COMMON_H_ */
