/*
 *  file:     uio_device.h
 *  brief:    UNIVIO device final instance, project specific
 *  version:  1.00
 *  date:     2022-02-13
 *  authors:  nvitya
*/

#ifndef _UIO_DEVICE_H
#define _UIO_DEVICE_H

#include "uio_gendev_impl.h"

class TUioDevice : public TUioGenDevImpl
{
public:
};

extern TUioDevice   g_uiodev;

#endif
