/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#pragma once

#include "scriptperif.h"
#include "mcutwi.h"

class asIScriptFunction;

class ScriptTwi : public McuTwi, public ScriptPerif
{
    public:
        ScriptTwi( eMcu* mcu, QString name );
        ~ScriptTwi();

        void reset() override;

        void setAddress( uint8_t a );

        void readByte() override;

        void writeByte() override;
        void sendByte( uint8_t data );

        void setTwiState( twiState_t state ) override;

        QStringList registerScript( ScriptCpu* cpu ) override;
        void startScript() override;

    private:
        uint8_t getStaus() { return *m_statReg &= 0b11111000; }

        asIScriptFunction* m_byteReceived;
        asIScriptFunction* m_writeByte;
        asIScriptFunction* m_setTwiState;
};
