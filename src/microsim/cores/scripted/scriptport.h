/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef SCRIPTPORT_H
#define SCRIPTPORT_H

#include "scriptmodule.h"
#include "mcuport.h"

class MAINMODULE_EXPORT ScriptPort : public ScriptModule, public McuPort
{
    public:
        ScriptPort( eMcu* mcu, QString name );
        ~ScriptPort();

        virtual void configureA( uint8_t newVal ) override;
        virtual void configureB( uint8_t newVal ) override;
        virtual void configureC( uint8_t newVal ) override;

        virtual void reset() override;

        virtual void setScript( QString script ) override;

        void setExtIntTrig( int pinNumber, int trig );

    protected:


};
#endif
