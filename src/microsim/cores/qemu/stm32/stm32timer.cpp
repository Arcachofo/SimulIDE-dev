/***************************************************************************
 *   Copyright (C) 2026 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QDebug>

#include "stm32timer.h"
#include "stm32ocunit.h"
#include "stm32pin.h"

#define CR1_OFFSET   0x00
#define CR2_OFFSET   0x04
#define SMCR_OFFSET  0x08
#define DIER_OFFSET  0x0c
#define SR_OFFSET    0x10
#define EGR_OFFSET   0x14
#define CCMR1_OFFSET 0x18
#define CCMR2_OFFSET 0x1c
#define CCER_OFFSET  0x20
#define CNT_OFFSET   0x24
#define PSC_OFFSET   0x28
#define ARR_OFFSET   0x2c
#define RCR_OFFSET   0x30
#define CCR1_OFFSET  0x34
#define CCR2_OFFSET  0x38
#define CCR3_OFFSET  0x3c
#define CCR4_OFFSET  0x40
#define BDTR_OFFSET  0x44
#define DCR_OFFSET   0x48
#define DMAR_OFFSET  0x4C

#define CR1_CEN 1<<0
#define CR1_OPM 1<<3


Stm32Timer::Stm32Timer( QemuDevice* mcu, QString name, int n, uint32_t* clk, uint64_t memStart, uint64_t memEnd )
          : QemuTimer( mcu, name, n, clk, memStart, memEnd )
{
    for( int i=0; i<4; ++i )
    {
        m_channel[i] = new Stm32OcUnit( this, name+"-OcUnit"+QString::number(i), i );
    }
}
Stm32Timer::~Stm32Timer() {}

void Stm32Timer::reset()
{
    for( int i=0; i<4; ++i ) m_channel[i]->reset();
}

void Stm32Timer::writeRegister()
{
    uint64_t offset = m_eventAddress - m_memStart;

    if( offset != 0 ) return;

    int ch = m_eventValue & 0x03;
    int state = m_eventValue & 1<<8;

    m_channel[ch]->m_pin->setOutState( state );
    //qDebug() << "Stm32Timer::writeRegister" << m_number << ch << m_channel[ch]->m_pin->pinId();

    //switch( offset )
    //{
    //case CR1_OFFSET:   writeCR1();                         break;
    //case CR2_OFFSET:   m_cr2  = m_eventValue & 0x00F8;     break;
    //case SMCR_OFFSET:  m_smcr = m_eventValue;              break;
    //case DIER_OFFSET:  m_dier = m_eventValue & 0x5F5F;     break;
    //case SR_OFFSET:    updateUIF();                        break;
    //case EGR_OFFSET:   writeEGR();                         break;
    //case CCMR1_OFFSET: writeCCMR( 0 );                     break;
    //case CCMR2_OFFSET: writeCCMR( 1 );                     break;
    //case CCER_OFFSET:  writeCCER();                        break;
    //case CNT_OFFSET:   setCount();                         break;
    //case PSC_OFFSET:   updtFreq();                         break;
    //case ARR_OFFSET:   updtPeriod();                       break;
    //case RCR_OFFSET:   m_rcr = m_eventValue;               break;
    //case CCR1_OFFSET:  m_channel[0]->m_CCR = m_eventValue; break;
    //case CCR2_OFFSET:  m_channel[1]->m_CCR = m_eventValue; break;
    //case CCR3_OFFSET:  m_channel[2]->m_CCR = m_eventValue; break;
    //case CCR4_OFFSET:  m_channel[3]->m_CCR = m_eventValue; break;
    //case BDTR_OFFSET:  m_bdtr = m_eventValue;              break;
    //case DCR_OFFSET:   m_dcr  = m_eventValue;              break;
    //case DMAR_OFFSET:  m_dmar = m_eventValue;              break;
    //default:           write();                            break;
    //}
}

void Stm32Timer::readRegister()
{
    uint64_t offset = m_eventAddress - m_memStart;

    uint64_t val = 0;

    switch( offset )
    {
    case CR1_OFFSET:   val = m_cr1;               break;
    case CR2_OFFSET:   val = m_cr2;               break;
    case SMCR_OFFSET:  val = m_smcr;              break;
    case DIER_OFFSET:  val = m_dier;              break;
    case SR_OFFSET:    val = m_sr;                break;
    case EGR_OFFSET:   val = m_egr;               break;
    case CCMR1_OFFSET: val = m_ccmr[0];           break;
    case CCMR2_OFFSET: val = m_ccmr[1];           break;
    case CCER_OFFSET:  val = m_ccer;              break;
    case CNT_OFFSET:   val = getCount();          break;
    case PSC_OFFSET:   val = m_psc;               break;
    case ARR_OFFSET:   val = m_arr;               break;
    case RCR_OFFSET:   val = m_rcr;               break;
    case CCR1_OFFSET:  val = m_channel[0]->m_CCR; break;
    case CCR2_OFFSET:  val = m_channel[1]->m_CCR; break;
    case CCR3_OFFSET:  val = m_channel[2]->m_CCR; break;
    case CCR4_OFFSET:  val = m_channel[3]->m_CCR; break;
    case BDTR_OFFSET:  val = m_bdtr;              break;
    case DCR_OFFSET:   val = m_dcr;               break;
    case DMAR_OFFSET:  val = m_dmar;              break;
    default:           val = read();              break;
    }
    m_arena->regData = val;
    m_arena->qemuAction = SIM_READ;
}

uint32_t Stm32Timer::getCount()
{
    uint32_t val = 0;

    return val;
}

void Stm32Timer::setCount()
{
    //uint32_t count =  m_eventValue;
}

void Stm32Timer::writeCR1()
{
    uint16_t cr1 = m_eventValue & 0x3FF;

    if( m_cr1 == cr1 ) return;
    m_cr1 = cr1;

    uint8_t enabled = cr1 & CR1_CEN;
    uint8_t oneShot = cr1 & CR1_OPM;

    if( m_enabled != enabled || m_oneShot != oneShot )
    {
        m_enabled = enabled;
        m_oneShot = oneShot;

        if( !enabled ) //Can't switch from edge-aligned to center-aligned if enabled (CEN=1)
        {
            uint8_t CMS = (cr1 & 0b1100000) >> 5;

            switch( CMS ) {
            case 0: m_bidirectional = 0; break;
            case 1: //break;
            case 2: //break;
            case 3: m_bidirectional = 1; break;
            default: break;
            }
        }

        // if( enabled ) ptimer_run( s->timer, s->oneShot ); // DPRINTF("%s Enabling timer\n", stm32_periph_name(s->periph));
        // else          ptimer_stop( s->timer );            // DPRINTF("%s Disabling timer\n", stm32_periph_name(s->periph));
    }
    if( !m_bidirectional )
    {
        m_direction = m_cr1 & 1<<4; // DIR bit
    }
}

void Stm32Timer::writeCCER()
{
    uint16_t ccer = m_eventValue;

    if( m_ccer == ccer ) return;
    m_ccer = ccer;

    for( int i=0; i<4; ++i )
    {
        m_channel[i]->writeCCER( ccer & 0x0F );
        ccer >>= 4;
    }
}

void Stm32Timer::writeEGR()
{
    uint16_t egr = m_eventValue & 0x1E;
    if( m_egr == egr ) return;
    m_egr = egr;
    if( m_eventValue & 0x40 ) m_sr |= 0x40;      // TG bit
    if( m_eventValue & 0x1  ) updtPeriod();      // UG bit - reload count
}

void Stm32Timer::writeCCMR( int i )
{
    uint16_t ccmr = m_eventValue;
    if( m_ccmr[i] == ccmr ) return;
    m_ccmr[i] = ccmr;

    int ch = i*2;
    m_channel[ch]->writeCCMR( ccmr );
    m_channel[ch+1]->writeCCMR( ccmr>>8 );
}

void Stm32Timer::updateUIF()
{
    m_sr ^= (m_eventValue ^ 0xFFFF);
    m_sr &= 0x1EFF;

    //uint8_t value = m_sr & 1;

    //if( !(m_sr & 1) )
    //{
    //    //s->sr &= ~0x1; /* update interrupt flag in status reg */
    //    s->sr |= (value & 0x1);
    //    s->dier |= 0x01;                 //FIXME what value must be used? RESET = 0
    //    qemu_set_irq( s->irq, value );
    //}
}

void Stm32Timer::updtFreq()
{
    m_psc = m_eventValue;
}

void Stm32Timer::updtPeriod()
{
    m_arr = m_eventValue;
}

void Stm32Timer::setOcPins( Stm32Pin* oc0Pin, Stm32Pin* oc1Pin, Stm32Pin* oc2Pin , Stm32Pin *oc3Pin)
{
    //qDebug() << "Stm32Timer::setOcPins" << m_number << oc0Pin->pinId() << oc1Pin->pinId() << oc2Pin->pinId() << oc3Pin->pinId();
    m_channel[0]->m_pin = oc0Pin;
    m_channel[1]->m_pin = oc1Pin;
    m_channel[2]->m_pin = oc2Pin;
    m_channel[3]->m_pin = oc3Pin;
}
