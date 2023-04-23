/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef PICSPI_H
#define PICSPI_H

#include "mcuspi.h"

class eMcu;

class PicSpi : public McuSpi
{
    friend class PicMssp;

    public:
        PicSpi( eMcu* mcu, QString name );
        ~PicSpi();

        //virtual void initialize() override;

        virtual void setMode( spiMode_t mode ) override;
        virtual void configureA( uint8_t newSSPCON ) override;
        virtual void writeStatus( uint8_t newSSPSTAT ) override;
        virtual void writeSpiReg( uint8_t newSSPBUF ) override;
        virtual void endTransaction() override;

    protected:

        bool m_clkPol;
        bool m_clkPha;

        /*uint8_t*  m_SPCR;

        regBits_t m_SPIE;

        regBits_t m_DODR;
        regBits_t m_MSTR;
        regBits_t m_CPOL;
        regBits_t m_CPHA;*/
        //regBits_t m_SPIF;

        regBits_t m_SSPEN;
        regBits_t m_CKP;
        regBits_t m_CKE;
};

#endif
