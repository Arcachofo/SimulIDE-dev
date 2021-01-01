#ifndef PIE_H
#define PIE_H

class PIR;

#include "registers.h"

//---------------------------------------------------------
// PIE Peripheral Interrupt Enable register base class 
// for PIE1 & PIE2

class PIE : public  SfrReg
{
    public:
      PIE(Processor *pCpu, const char *pName );

      void put(uint new_value);
      void setPir(PIR *pPir);

    protected:
      PIR *pir;
};

#endif /* PIE_H */
