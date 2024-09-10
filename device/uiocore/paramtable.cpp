/* -----------------------------------------------------------------------------
 * This file is a part of the UNIVIO project: https://github.com/nvitya/univio
 * Copyright (c) 2023 Viktor Nagy, nvitya
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
 *  file:     paramtable.cpp
 *  brief:    UnivIO Device Parameter Table
 *  created:  2023-06-07
 *  authors:  nvitya
*/

#include "string.h"
#include "paramtable.h"
#include "uio_device.h"
#include "simple_scope.h"

// at some improper definitions (like missing TClass base) the tables were moved to .data (initialized RW data)
// and thus took twice so much space.  So we force these tables to .rodata with the following macro:
//#define PARAMTABLE_DEF   __attribute__((section(".rodata"))) const TParameterDef
#define PARAMTABLE_DEF   const TParameterDef
// if you get "session type conflict", then maybe some classes with callbacks are not based on the TClass

#define PAR_TABLE(astart, atab)    {astart, astart + (sizeof(atab) / sizeof(TParameterDef)) - 1, &atab[0], nullptr, nullptr}

#define PTENTRY_EMPTY         0, nullptr, nullptr, nullptr
#define PTENTRY_CONST(aval)   PAR_INT32_CONST,  (void *)(aval), nullptr, nullptr
#define PTENTRY_CONST8(aval)  PAR_UINT8_CONST,  (void *)(aval), nullptr, nullptr


/*****************************************************************************************************************
                                             PARAMETER TABLES
******************************************************************************************************************/
// range tables must come before the main

/*****************************************************************************************************************
                                       THE MAIN RANGE TABLE
******************************************************************************************************************/

const TParamRangeDef  param_range_table[] =
{
  {0x0100, 0x017F, nullptr, &g_uiodev, PParRangeMethod(&TUioDevice::prfn_0100_DevId) }, // some RO device identifiactions
  {0x0180, 0x019F, nullptr, &g_uiodev, PParRangeMethod(&TUioDevice::prfn_0180_DevConf) },

  // pin configuration
  {0x01FF, 0x01FF, nullptr, &g_uiodev, PParRangeMethod(&TUioDevice::prfn_PinCfgReset) },
  {0x0200, 0x02FF, nullptr, &g_uiodev, PParRangeMethod(&TUioDevice::prfn_PinConfig) },

  // output default values
  {0x0300, 0x0300, nullptr, &g_uiodev, PParRangeMethod(&TUioDevice::prfn_DefValue_DigOut) },
  {0x0320, 0x033F, nullptr, &g_uiodev, PParRangeMethod(&TUioDevice::prfn_DefValue_AnaOut) },
  {0x0340, 0x035F, nullptr, &g_uiodev, PParRangeMethod(&TUioDevice::prfn_DefValue_PwmDuty) },
  {0x0360, 0x037F, nullptr, &g_uiodev, PParRangeMethod(&TUioDevice::prfn_DefValue_LedBlp) },
  {0x0700, 0x071F, nullptr, &g_uiodev, PParRangeMethod(&TUioDevice::prfn_DefValue_PwmFreq) },

  // configuration info
  {0x0E00, 0x0EFF, nullptr, &g_uiodev, PParRangeMethod(&TUioDevice::prfn_ConfigInfo) },
  // Non-Volatile Data
  {0x0F00, 0x0FFF, nullptr, &g_uiodev, PParRangeMethod(&TUioDevice::prfn_NvData) },

  // IO Access
  {0x1000, 0x1001, nullptr, &g_uiodev, PParRangeMethod(&TUioDevice::prfn_DigOutSetClr) },
  {0x1010, 0x1010, nullptr, &g_uiodev, PParRangeMethod(&TUioDevice::prfn_DigOutDirect) },
  {0x1100, 0x1100, nullptr, &g_uiodev, PParRangeMethod(&TUioDevice::prfn_DigInValues) },
  {0x1200, 0x121F, nullptr, &g_uiodev, PParRangeMethod(&TUioDevice::prfn_AnaInValues) },
  {0x1300, 0x13FF, nullptr, &g_uiodev, PParRangeMethod(&TUioDevice::prfn_AnaOutCtrl) },
  {0x1400, 0x14FF, nullptr, &g_uiodev, PParRangeMethod(&TUioDevice::prfn_PwmControl) },
  {0x1500, 0x15FF, nullptr, &g_uiodev, PParRangeMethod(&TUioDevice::prfn_LedBlpCtrl) },

	// SPI, I2C
  {0x1600, 0x161F, nullptr, &g_spictrl[0], PParRangeMethod(&TUioSpiCtrl::prfn_SpiControl) },
	#if UIO_SPI_COUNT > 1
	  {0x1620, 0x163F, nullptr, &g_spictrl[1], PParRangeMethod(&TUioSpiCtrl::prfn_SpiControl) },
	#endif

  #if UIO_SPIFLASH_COUNT
    {0x1680, 0x168F, nullptr, &g_spiflash_ctrl, PParRangeMethod(&TUioSpiFlashCtrl::prfn_SpiFlashControl) },
  #endif

  {0x1700, 0x171F, nullptr, &g_i2cctrl[0], PParRangeMethod(&TUioI2cCtrl::prfn_I2cControl) },
	#if UIO_I2C_COUNT > 1
	  {0x1720, 0x173F, nullptr, &g_i2cctrl[1], PParRangeMethod(&TUioI2cCtrl::prfn_I2cControl) },
	#endif

  // MPRAM
  {0xC000, 0xC000, nullptr, &g_uiodev, PParRangeMethod(&TUioDevice::prfn_Mpram) },

	//PAR_TABLE(0x0100, pt_0100),   // main application parameters
	//PAR_TABLE(0x5000, pt_5000_scope), // scope parameters

	// close the list
	{0, 0, nullptr, nullptr, nullptr}
};

/*****************************************************************************************************************
******************************************************************************************************************/

