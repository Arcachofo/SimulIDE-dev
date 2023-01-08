/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef IOPORT_H
#define IOPORT_H

#include <vector>

#include "iopin.h"

class IoPin;
class Component;
class asIScriptEngine;

class MAINMODULE_EXPORT IoPort
{
        friend class McuCreator;

    public:
        IoPort( QString name );
        ~IoPort();

        void reset();

        void setOutState( uint32_t val );
        void setOutStatFast( uint32_t val );
        uint32_t getInpState();

        void setDirection( uint32_t val );
        void setPinMode( pinMode_t mode );
        void setPinMode( uint mode ) { setPinMode( (pinMode_t) mode ); }

        void changeCallBack( eElement* el, bool ch );

        IoPin* getPinN( uint8_t i );
        IoPin* getPin( QString pinName );

        QString name() { return m_name; }

        static void registerScript( asIScriptEngine* engine );

    protected:
        void createPins( Component* comp, QString pins, uint32_t pinMask );
        virtual IoPin* createPin( int i, QString id , Component* comp );

        QString m_name;
        QString m_shortName;

        std::vector<IoPin*> m_pins;
        uint8_t m_numPins;
        uint m_pinState;
        uint m_pinDirection;
};

#endif
