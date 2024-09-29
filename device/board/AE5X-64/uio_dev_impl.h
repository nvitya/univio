/*
 * uio_gendev_impl.h (ATSAME5X)
 */

#ifndef UIO_GENDEV_H_
#define UIO_GENDEV_H_

#include <uio_dev_base.h>

class TUioDevImpl : public TUioDevBase
{
private:
  typedef TUioDevBase  super;

public:

  virtual bool      InitBoard();
  virtual bool      PinFuncAvailable(TPinCfg * pcf);

  virtual void      SetupAdc(TPinCfg * pcf);
  virtual void      SetupDac(TPinCfg * pcf);
  virtual void      SetupPwm(TPinCfg * pcf);
  virtual void      SetupSpi(TPinCfg * pcf);
  virtual void      SetupI2c(TPinCfg * pcf);
  virtual void      SetupUart(TPinCfg * pcf);
  virtual void      SetupClockOut(TPinCfg * pcf);

  virtual bool      LoadBuiltinConfig(uint8_t anum);
};

#endif /* UIO_GENDEV_H_ */
