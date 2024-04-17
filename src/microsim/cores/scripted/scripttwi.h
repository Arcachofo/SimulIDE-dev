/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef SCRIPTTWI_H
#define SCRIPTTWI_H

#include "scriptperif.h"
#include "mcutwi.h"

class asIScriptFunction;

class ScriptTwi : public McuTwi, public ScriptPerif
{
    public:
        ScriptTwi( eMcu* mcu, QString name );
        ~ScriptTwi();

        virtual void reset() override;

        void setAddress( uint8_t a );

        virtual void readByte() override;
        void sendByte( uint8_t data );

        virtual QStringList registerScript( ScriptCpu* cpu ) override;
        virtual void startScript() override;

    private:
        uint8_t getStaus() { return *m_statReg &= 0b11111000; }

        asIScriptFunction* m_byteReceived;

};

#endif
