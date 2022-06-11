/*
 * uio_nvstorage.cpp
 *
 *  Created on: Jun 11, 2022
 *      Author: vitya
 */

#include <uio_nvstorage.h>
#include "board_pins.h"
#include "hwintflash.h"

TUioNvStorage g_nvstorage;

#if HAS_SPI_FLASH

  #include "spiflash_updater.h"

  #define SPIFL_UPDATER_BUFSIZE  256  // allocated on the stack !

#endif

bool TUioNvStorage::Init()
{
  initialized = false;

  #if HAS_SPI_FLASH
    if (!spiflash.initialized)
    {
      return false;
    }
  #else
    if (!hwintflash.initialized)
    {
      if (!hwintflash.Init())
      {
        return false;
      }
    }
  #endif

  initialized = true;
  return true;
}

void TUioNvStorage::Erase(unsigned addr, unsigned len)
{
  if (!initialized)
  {
    return;
  }

  #if HAS_SPI_FLASH
    spiflash.StartEraseMem(addr, len);
    spiflash.WaitForComplete();
  #else
    hwintflash.StartEraseMem(addr, len);
    hwintflash.WaitForComplete();
  #endif
}

void TUioNvStorage::Write(unsigned addr, void * src, unsigned len)
{
  if (!initialized)
  {
    return;
  }

  #if HAS_SPI_FLASH
    spiflash.StartWriteMem(addr, src, len);
    spiflash.WaitForComplete();
  #else
    hwintflash.StartWriteMem(addr, src, len);
    hwintflash.WaitForComplete();
  #endif
}

void TUioNvStorage::CopyTo(unsigned addr, void * src, unsigned len)
{
  if (!initialized)
  {
    return;
  }

  #if HAS_SPI_FLASH

    uint8_t   localbuf[SPIFL_UPDATER_BUFSIZE] __attribute__((aligned(8)));

    // Using the flash writer to first compare the flash contents:
    TSpiFlashUpdater  flashupdater(&spiflash, localbuf, sizeof(localbuf));
    flashupdater.UpdateFlash(addr, (uint8_t *)src, len);

  #else
    hwintflash.StartCopyMem(addr, src, len);
    hwintflash.WaitForComplete();
  #endif
}

void TUioNvStorage::Read(unsigned addr, void * dst, unsigned len)
{
  if (!initialized)
  {
    return;
  }

  #if HAS_SPI_FLASH
    spiflash.StartReadMem(addr,  dst,  len);
    spiflash.WaitForComplete();
  #else
    memcpy(dst, (void *)addr, len);
  #endif
}
