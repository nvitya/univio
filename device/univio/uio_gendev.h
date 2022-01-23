/*
 * uio_gendev.h
 *
 *  Created on: Nov 28, 2021
 *      Author: vitya
 */

#ifndef SRC_UIO_GENDEV_H_
#define SRC_UIO_GENDEV_H_

#include "uio_gendev_impl.h"

class TUioGenDev : public TUioGenDevImpl
{
public:
};

extern TUioGenDev   g_uiodev;

#endif /* SRC_UIO_GENDEV_H_ */
