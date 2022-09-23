/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef MCUSPI_H
#define MCUSPI_H

#include "mcuprescaled.h"
#include "spimodule.h"

class eMcu;
class McuPin;

class MAINMODULE_EXPORT McuSpi : public McuPrescaled, public SpiModule
{
        friend class McuCreator;

    public:
        McuSpi( eMcu* mcu, QString name );
        ~McuSpi();

        //virtual void initialize() override;

        //virtual void setMode( spiMode_t mode ) override;
        virtual void writeStatus( uint8_t val ){;}
        virtual void writeSpiReg( uint8_t val ){;}

    protected:
        uint8_t* m_dataReg;
        uint8_t* m_statReg;
};

#endif
