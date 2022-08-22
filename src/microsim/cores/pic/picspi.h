/***************************************************************************
 *   Copyright (C) 2021 by santiago González                               *
 *   santigoro@gmail.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.  *
 *                                                                         *
 ***************************************************************************/

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
