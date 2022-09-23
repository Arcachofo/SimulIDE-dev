/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef PICSPI_H
#define PICSPI_H

#include "mcuspi.h"

class eMcu;

class MAINMODULE_EXPORT PicSpi : public McuSpi
{
    friend class PicMssp;

    public:
        PicSpi( eMcu* mcu, QString name );
        ~PicSpi();

        //virtual void initialize() override;

        virtual void setMode( spiMode_t mode ) override;
        virtual void configureA( uint8_t newSPCR ) override;
        virtual void writeStatus( uint8_t newSPSR ) override;
        virtual void writeSpiReg( uint8_t newSPDR ) override;
        virtual void endTransaction() override;

    protected:

        /*uint8_t*  m_SPCR;

        regBits_t m_SPIE;
        regBits_t m_SPE;
        regBits_t m_DODR;
        regBits_t m_MSTR;
        regBits_t m_CPOL;
        regBits_t m_CPHA;*/
        //regBits_t m_SPIF;
};

#endif
