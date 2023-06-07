/*
 * uio_nvstorage.h
 *
 *  Created on: Jun 11, 2022
 *      Author: vitya
 */

#ifndef SRC_UIO_NVSTORAGE_H_
#define SRC_UIO_NVSTORAGE_H_

class TUioNvStorage
{
public:
  bool initialized = false;

  bool Init();
  void Erase(unsigned addr, unsigned len);
  void Write(unsigned addr, void * src, unsigned len);
  void CopyTo(unsigned addr, void * src, unsigned len);  // checks the previous contents first
  void Read(unsigned addr, void * dst, unsigned len);
};

extern TUioNvStorage g_nvstorage;

#endif /* SRC_UIO_NVSTORAGE_H_ */
