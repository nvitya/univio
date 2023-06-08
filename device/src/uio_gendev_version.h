/*
 * uio_gendev_version.h
 *
 *  Created on: Jun 8, 2023
 *      Author: vitya
 */

#ifndef SRC_UIO_GENDEV_VERSION_H_
#define SRC_UIO_GENDEV_VERSION_H_


#define UIO_VERSION_MAJOR       2
#define UIO_VERSION_MINOR       0
#define UIO_VERSION_INCREMENT   0

#define UIO_VERSION_INTEGER  ((UIO_VERSION_MAJOR << 24) | (UIO_VERSION_MINOR << 16) | UIO_VERSION_INCREMENT)

/* VERSION LOG

2.0.0
  - Rework using UDO protocol

*/

#endif /* SRC_UIO_GENDEV_VERSION_H_ */
