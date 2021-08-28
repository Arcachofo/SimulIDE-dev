/***************************************************************************
 *   Copyright (C) 2012 by santiago González                               *
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

  
#ifndef PICPROCESSOR_H
#define PICPROCESSOR_H

#include "baseprocessor.h"

#include "pic-processor.h"
#include "registers.h"
#include "hexutils.h"

class _RCSTA;

class PicProcessor : public BaseProcessor
{
    Q_OBJECT
    public:
        PicProcessor( McuComponent* parent );
        ~PicProcessor();

        virtual bool setDevice( QString device ) override;
        virtual bool loadFirmware( QString file ) override;
        virtual void reset() override;
        virtual void stepCpu() override { m_pPicProcessor->stepCpuClock(); }

        virtual int pc() override;
        virtual uint64_t cycle() override { return m_pPicProcessor->currentCycle(); }
        virtual void setFreq( double freq ) override;

        virtual uint8_t getRamValue( int address ) override;
        virtual void    setRamValue( int address, uint8_t value ) override;
        virtual uint16_t getFlashValue( int address ) override;
        virtual void setFlashValue( int address, uint16_t value ) override;
        virtual uint8_t getRomValue( int address ) override;
        virtual void    setRomValue( int address, uint8_t value ) override;

        virtual QVector<int>* eeprom() override;

        virtual void uartIn( int uart, uint32_t value ) override;

        pic_processor* getCpu() { return m_pPicProcessor; }

    private:
        pic_processor* m_pPicProcessor;
        HexLoader m_hexLoader;
        QVector<_RCSTA*> m_rcsta;
};


#endif
