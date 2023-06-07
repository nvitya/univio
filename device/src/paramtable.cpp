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

PARAMTABLE_DEF pt_2000_app[] =  // 0x2000
{
/* +00*/{ PAR_INT32_RO,  (void *)&g_device.irq_period_ns,      nullptr, nullptr },
/* +01*/{ PAR_UINT16_RO, (void *)&g_device.irq_cycle_counter,  nullptr, nullptr },
/* +02*/{ PAR_INT32_RO,  (void *)&g_device.func_i32_1,         nullptr, nullptr },
/* +03*/{ PAR_INT32_RO,  (void *)&g_device.func_i32_2,         nullptr, nullptr },
/* +04*/{ PAR_INT16_RO,  (void *)&g_device.func_i16_1,         nullptr, nullptr },
/* +05*/{ PAR_INT16_RO,  (void *)&g_device.func_i16_2,         nullptr, nullptr },
/* +06*/{ PAR_FLOAT_RO,  (void *)&g_device.func_fl_1,          nullptr, nullptr },
/* +07*/{ PAR_FLOAT_RO,  (void *)&g_device.func_fl_2,          nullptr, nullptr },
/* +08*/{ PAR_INT32_RW,  (void *)&g_device.par_dummy_i32,      nullptr, nullptr },
/* +09*/{ PAR_INT16_RW,  (void *)&g_device.par_dummy_i16,      nullptr, nullptr },

///* +0A*/{ PTENTRY_SPECIALFUNC(0, &g_device, PWdParMethod(&TDevice::pfn_big_data)) }, //
};


/*****************************************************************************************************************
                                       THE MAIN RANGE TABLE
******************************************************************************************************************/

const TParamRangeDef  param_range_table[] =
{
	{0x1000, 0x1018, nullptr, &g_device, PParRangeMethod(&TDevice::prfn_canobj_1008_1018) }, // some RO device identifiactions

	PAR_TABLE(0x2000, pt_2000_app),   // main application parameters
	PAR_TABLE(0x5000, pt_5000_scope), // scope parameters

	// close the list
	{0, 0, nullptr, nullptr, nullptr}
};

/*****************************************************************************************************************
******************************************************************************************************************/

