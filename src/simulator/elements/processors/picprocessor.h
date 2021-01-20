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

        bool loadFirmware( QString file );
        void terminate();

        virtual void reset();

        void stepCpu(){ m_pPicProcessor->stepCpuClock(); }

        virtual int pc();
        virtual uint64_t cycle(){ return m_pPicProcessor->currentCycle(); }
        virtual void setFreq( double freq );

        int getRamValue( int address );

        void uartIn( int uart, uint32_t value );

        virtual QVector<int> eeprom();
        virtual void setEeprom( QVector<int> eep );

        virtual void setDevice( QString device );
        
        pic_processor* getCpu() { return m_pPicProcessor; }

    private:
        virtual int validate( int address );

        pic_processor* m_pPicProcessor;
        
        HexLoader m_hexLoader;
        
        QVector<_RCSTA*> m_rcsta;
};


#endif
