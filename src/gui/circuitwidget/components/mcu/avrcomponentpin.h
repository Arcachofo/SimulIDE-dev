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

#ifndef AVRCOMPONENTPIN_H
#define AVRCOMPONENTPIN_H

#include "mcucomponentpin.h"

//simavr includes
#include "sim_avr.h"
#include "sim_irq.h"
#include "sim_io.h"
#include "avr_adc.h"
#include "avr_acomp.h"
#include "avr_ioport.h"
#include "avr_timer.h"


class AVRComponentPin : public McuComponentPin
{
    Q_OBJECT
    public:
        AVRComponentPin( McuComponent *mcu, QString id, QString type, QString label, int pos, int xpos, int ypos, int angle );
        ~AVRComponentPin();

        virtual void initialize() override;
        //virtual void stamp() override;
        virtual void voltChanged() override;

        virtual void attachPin( avr_t* AvrProcessor );
        virtual void setState( bool state ) override;

        virtual void setImp( double imp ) override;

        void enableSPI( uint32_t value );

        void adcread();

        static void ddr_hook( struct avr_irq_t* irq, uint32_t value, void* param )
        {
            Q_UNUSED(irq);
            // get the pointer out of param and asign it to AVRComponentPin*
            AVRComponentPin* ptrAVRComponentPin = reinterpret_cast<AVRComponentPin*> (param);

            ptrAVRComponentPin->setDirection( value>0 );
        }
        static void port_hook( struct avr_irq_t* irq, uint32_t value, void* param )
        {
            AVRComponentPin* ptrAVRComponentPin = reinterpret_cast<AVRComponentPin*> (param);
            ptrAVRComponentPin->setOutState( value>0 );
        }
        static void port_reg_hook( struct avr_irq_t* irq, uint32_t value, void* param )
        {
            Q_UNUSED(irq);
            // get the pointer out of param and asign it to AVRComponentPin*
            AVRComponentPin* ptrAVRComponentPin = reinterpret_cast<AVRComponentPin*> (param);
            ptrAVRComponentPin->setState( value>0 );
        }
        static void pwm_pin_hook( struct avr_irq_t* irq, uint32_t value, void* param )
        {
            Q_UNUSED(irq);
            // get the pointer out of param and asign it to AVRComponentPin*
            AVRComponentPin* ptrAVRComponentPin = reinterpret_cast<AVRComponentPin*> (param);
            ptrAVRComponentPin->enableIO( (value==0) );
        }
        static void pwm_out_hook( struct avr_irq_t* irq, uint32_t value, void* param )
        {
            Q_UNUSED(irq);
            // get the pointer out of param and asign it to AVRComponentPin*
            AVRComponentPin* ptrAVRComponentPin = reinterpret_cast<AVRComponentPin*> (param);
            ptrAVRComponentPin->m_enableIO = true;
            ptrAVRComponentPin->setState( value>0 );
            ptrAVRComponentPin->m_enableIO = false;
        }
        static void spiEn_hook( struct avr_irq_t* irq, uint32_t value, void* param )
        {
            AVRComponentPin* ptrAVRComponentPin = reinterpret_cast<AVRComponentPin*> (param);
            ptrAVRComponentPin->enableSPI( value );
        }

    protected:
        int  m_channelAdc;
        int  m_channelAin;

        QString m_spiPin;

 static QString m_lastid;
 static uint64_t m_lastCycle;
        //from simavr
        avr_t*     m_avrProcessor;
        avr_irq_t* m_PortChangeIrq;
        avr_irq_t* m_PortRegChangeIrq;
        avr_irq_t* m_DdrRegChangeIrq;
        avr_irq_t* m_Write_stat_irq;
        avr_irq_t* m_Write_adc_irq;
        avr_irq_t* m_Write_acomp_irq;
        avr_irq_t* m_Spi_Enable_Irq;
};

#endif
