/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef MCUMODULE_H
#define MCUMODULE_H

#include <QString>

#include "mcutypes.h"

class eMcu;
class Interrupt;

class McuModule
{
        friend class McuCreator;

    public:
        McuModule( eMcu* mcu, QString name );
        virtual ~McuModule();

        virtual void setup(){;}
        virtual void reset(){;}

        virtual void configureA( uint8_t ){;}
        virtual void configureB( uint8_t ){;}
        virtual void configureC( uint8_t ){;}
        virtual void callBackDoub( double ) {;}
        virtual void callBack() {;}
        virtual void sleep( int mode );

        void setSleepMode( uint8_t m ) { m_sleepMode = m; }

        virtual void setInterrupt( Interrupt* i ) { m_interrupt = i; }
        Interrupt* getInterrupt() { return m_interrupt; }

        eMcu* getMcu() { return m_mcu; }

    protected:
        QString m_name;
        eMcu*   m_mcu;

        uint8_t m_sleepMode;
        bool m_sleeping;

        Interrupt* m_interrupt;

        regBits_t m_configBitsA;
        regBits_t m_configBitsB;
        regBits_t m_configBitsC;
};
#endif
