/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef SCRIPTUSART_H
#define SCRIPTUSART_H

#include "mcuuart.h"

class ScriptCpu;
class asIScriptFunction;

class MAINMODULE_EXPORT ScriptUsart : public McuUsart
{
    public:
        ScriptUsart( eMcu* mcu, QString name, int number );
        ~ScriptUsart();

        virtual void reset() override;
        virtual void byteReceived( uint8_t data ) override;
        virtual void frameSent( uint8_t data ) override;

    private:
        ScriptCpu* m_scriptCpu;

        QString m_uartName;

        bool m_initialized;

        asIScriptFunction* m_byteReceived;
        asIScriptFunction* m_frameSent;
};

#endif
