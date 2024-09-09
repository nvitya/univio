/*
 * uio_gendev_version.h
 *
 *  Created on: Jun 8, 2023
 *      Author: vitya
 */

#ifndef SRC_UIO_GENDEV_VERSION_H_
#define SRC_UIO_GENDEV_VERSION_H_


#define UIO_VERSION_MAJOR       3
#define UIO_VERSION_MINOR       0
#define UIO_VERSION_INCREMENT   0

#define UIO_VERSION_INTEGER  ((UIO_VERSION_MAJOR << 24) | (UIO_VERSION_MINOR << 16) | UIO_VERSION_INCREMENT)

/* VERSION LOG

3.0.0
  - Multi SPI, UART, I2C support
  - CAN support
2.1.1
  - SPI Flash Write Accelerator fixes
2.1.0
  - SPI Flash Write Accelerator added
2.0.2
  - SG473-48 I2C pin functions moved to safe I2C pins
  - SG473-48: opendrain added to I2C pin config
2.0.1
  - Zero-padding strings like manufacturer name

2.0.0
  - Rework using UDO protocol

*/

#endif /* SRC_UIO_GENDEV_VERSION_H_ */