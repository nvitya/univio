/*
 *  file:     device.h
 *  brief:    Example UDO Slave Device main Object
 *  created:  2023-05-13
 *  authors:  nvitya
 *  license:  public domain
*/

#ifndef DEVICE_H_
#define DEVICE_H_

#include "udo.h"
#include "udoslave.h"
#include "parameterdef.h"
#include "simple_partable.h"

#define DEVICE_NAME "UDOIP PC Test"

class TDevice : public TClass // common base for bootloader and application
{
public:
	bool           outputs_active = false;
	bool           pdo_active = false;

	volatile uint16_t  irq_cycle_counter = 0;

	uint32_t       irq_period_ns = 250000;

	volatile int32_t  func_i32_1 = 0;
	volatile int32_t  func_i32_2 = 0;
	volatile int16_t  func_i16_1 = 0;
	volatile int16_t  func_i16_2 = 0;
	volatile float    func_fl_1 = 0;
	volatile float    func_fl_2 = 0;

	float          seed_sin[4] = {0.0,  0.0,   0.0,  0.0};
	float          seed_inc[4] = {0.01, 0.011, 0.02, 0.05};

	void           IrqTask(); // IRQ Context !

public:
	uint32_t       canopen_devid = 0x000000B1;

	char           udo_str_buf[64]; // for responding smaller strings, but segmented possibility

public:

	virtual        ~TDevice() { }  // to avoid compiler warning

	void           Init();
	void 					 Run(); // Run Idle Tasks

	bool           prfn_canobj_1008_1018(TUdoRequest * udorq, TParamRangeDef * prdef);

public:
	int            par_dummy_i32 = 0;
	int16_t        par_dummy_i16 = 0;
	uint8_t        par_dummy_u8 = 0;

	uint64_t       par_i_ser = 0x5511223344;

	bool           pfn_ignore(TUdoRequest * udorq, TParameterDef * pdef, void * varptr);
};

extern TDevice   g_device;

#endif /* DEVICE_H_ */
