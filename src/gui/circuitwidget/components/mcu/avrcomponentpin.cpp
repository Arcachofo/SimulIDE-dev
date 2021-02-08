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

#include "avrcomponentpin.h"
#include "baseprocessor.h"
#include "simulator.h"

uint64_t AVRComponentPin::m_lastCycle = 0;
QString  AVRComponentPin::m_lastid = "";

AVRComponentPin::AVRComponentPin( McuComponent* mcu, QString id, QString type, QString label, int pos, int xpos, int ypos, int angle )
               : McuComponentPin( mcu, id, type, label, pos, xpos, ypos, angle )
{
    m_channelAdc = -1;
    m_channelAin = -1;

    m_avrProcessor = 0l;
    m_PortRegChangeIrq = 0l;
    m_DdrRegChangeIrq = 0l;
    m_Write_stat_irq = 0l;
    m_Write_adc_irq = 0l;
    m_Write_acomp_irq = 0l;
}
AVRComponentPin::~AVRComponentPin(){}

void AVRComponentPin::attachPin( avr_t*  AvrProcessor )
{
    m_avrProcessor = AvrProcessor;

    if( m_id.startsWith("P") )
    {
        m_pinType = 1;
        m_port = m_id.at(1).toLatin1();
        m_pinN = m_id.mid(2,1).toInt();
        
        // PORTX Register change irq
        QString portName = "PORT";
        portName.append( m_id.at(1) );

        int portAddr = m_processor->getRegAddress( portName );
        if( portAddr < 0 )
        {
            qDebug()<<"PORT Register for"<<AvrProcessor->mmcu<<"Not found\nAVRComponentPin::attach"<<portName<<m_pinN;
            return;
        }
        m_PortRegChangeIrq = avr_iomem_getirq( AvrProcessor, portAddr, &m_port, m_pinN );
        avr_irq_register_notify( m_PortRegChangeIrq, port_reg_hook, this );
        
        // DDRX Register change irq
        QString ddrName = "DDR";
        ddrName.append( m_id.at(1) );
        int ddrAddr = m_processor->getRegAddress( ddrName );
        if( ddrAddr < 0 )
        {
            qDebug()<<"DDR Register for"<<AvrProcessor->mmcu<<"Not found\nAVRComponentPin::attach"<<ddrName<<m_pinN;
            return;
        }
        m_DdrRegChangeIrq = avr_iomem_getirq( AvrProcessor, ddrAddr, " ", m_pinN );
        avr_irq_register_notify( m_DdrRegChangeIrq, ddr_hook, this );
        
        m_Write_stat_irq = avr_io_getirq( AvrProcessor, AVR_IOCTL_IOPORT_GETIRQ(m_port), m_pinN );
        
        QStringList types = m_type.split( "," );
        for( QString ty : types )
        {
            ty = ty.toLower();
            if( ty.startsWith("adc") )
            {
                m_channelAdc = ty.remove( "adc" ).toInt();
                m_Write_adc_irq   = avr_io_getirq( m_avrProcessor, AVR_IOCTL_ADC_GETIRQ, m_channelAdc );
                m_Write_acomp_irq = avr_io_getirq( m_avrProcessor, AVR_IOCTL_ACOMP_GETIRQ, ACOMP_IRQ_ADC0+m_channelAdc );
            }
            else if( ty.startsWith("ain") )
            {
                m_channelAin = ty.remove( "ain" ).toInt();
                m_Write_acomp_irq = avr_io_getirq( m_avrProcessor, AVR_IOCTL_ACOMP_GETIRQ, m_channelAin );
            }
            else if( ty.startsWith("oc") )
            {
                QString com = ty.remove( "oc" );
                QString ch = com.right(1);
                int channel = 0;
                if     ( ch == "b" ) channel = 1;
                else if( ch == "c" ) channel = 2;

                QByteArray ba = com.left(1).toUtf8();
                char timer = ba[0];

                avr_irq_t* pwm_pin = avr_io_getirq( m_avrProcessor, AVR_IOCTL_TIMER_GETIRQ(timer), channel ); //TIMER_IRQ_OUT_PWM0
                avr_irq_register_notify( pwm_pin, pwm_pin_hook, this );

                avr_irq_t* pwm_out = avr_io_getirq( m_avrProcessor, AVR_IOCTL_TIMER_GETIRQ(timer), TIMER_IRQ_OUT_COMP+channel ); //TIMER_IRQ_OUT_PWM0
                avr_irq_register_notify( pwm_out, pwm_out_hook, this );
            }
        }
    }
    else if( m_type == "reset" ) 
    {
        m_pinType = 21;
    }
    else if( m_type == "vcc"  ) 
    {
        m_pinType = 22;
    }
    else if( m_type == "avcc" ) 
    {
        m_pinType = 23;
    }
    else if( m_type == "aref" )
    {
         m_pinType = 24;
    }
    m_attached = true;
    
    initialize();
}

void AVRComponentPin::initialize()
{
    if( m_pinType == 1 )
    {
        if( m_Write_stat_irq )  m_Write_stat_irq->flags  |= IRQ_FLAG_INIT;
        if( m_Write_adc_irq )   m_Write_adc_irq->flags   |= IRQ_FLAG_INIT;
        if( m_Write_acomp_irq ) m_Write_acomp_irq->flags |= IRQ_FLAG_INIT;

        m_PortRegChangeIrq->flags |= IRQ_FLAG_INIT;
        m_DdrRegChangeIrq->flags  |= IRQ_FLAG_INIT;
    }
    McuComponentPin::initialize();
}

void AVRComponentPin::stamp()
{
    if( m_pinType > 20 )
    {
        if( m_ePin[0]->isConnected() && m_attached )
            m_ePin[0]->getEnode()->voltChangedCallback( this );
    }
    McuComponentPin::stamp();
}

void AVRComponentPin::voltChanged()
{
    double volt = m_ePin[0]->getVolt();

    if( m_pinType == 1 )                    // Is an IO Pin
    {
        if( volt  > 2.5 ) avr_raise_irq( m_Write_stat_irq, 1 );
        else              avr_raise_irq( m_Write_stat_irq, 0 );

        if( m_Write_acomp_irq ) // Comparator
            avr_raise_irq( m_Write_acomp_irq, m_ePin[0]->getVolt()*1000 );
    }
    else if( m_pinType == 21 )             // reset
    {
        if( volt < 3 )  m_processor->hardReset( true );
        else            m_processor->hardReset( false );
    }
    else if( m_pinType == 22 ) { m_avrProcessor->vcc  = volt*1000;}
    else if( m_pinType == 23 ) { m_avrProcessor->avcc = volt*1000;}
    else if( m_pinType == 24 ) { m_avrProcessor->aref = volt*1000;}
}

void AVRComponentPin::setState( bool state )
{
    if( m_isInput ) setPullup( state );
    McuComponentPin::setState( state );
}

void AVRComponentPin::pullupNotConnected( bool up )
{
    avr_raise_irq( m_Write_stat_irq, up? 1:0 );
}

void AVRComponentPin::adcread()
{
    //qDebug() << "ADC Read channel:    pin: " << m_id <<m_ePin[0]->getVolt()*1000 ;
    if( m_Write_adc_irq ) avr_raise_irq( m_Write_adc_irq, m_ePin[0]->getVolt()*1000 );
}

void AVRComponentPin::setImp( double imp ) // Used by I2C
{
    m_gndAdmEx = 1/imp;
    update();
}

#include "moc_avrcomponentpin.cpp"
