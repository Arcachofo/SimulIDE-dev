/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef IOPORT_H
#define IOPORT_H

#include <vector>

#include "iopin.h"
#include "iopin.h"
#include "e-element.h"

#include "scriptarray.h"

class IoPin;
class Component;
class asIScriptEngine;

class IoPort :public eElement
{
        friend class McuCreator;

    public:
        IoPort( QString name );
        ~IoPort();

        void reset();
        virtual void runEvent() override;

        void scheduleState( uint32_t val, uint64_t time );
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

        // ----------------
        struct outState_t{
            uint64_t time;
            uint64_t state;
        };
        void trigger( uint n=0 );
        void addSequence( CScriptArray* t );

 static void registerScript( asIScriptEngine* engine );

    protected:
        void createPins( Component* comp, QString pins, uint32_t pinMask );
        virtual IoPin* createPin( int i, QString id , Component* comp );

        inline void nextStep();

        QString m_name;
        QString m_shortName;

        uint m_pinState;
        uint m_nextState;
        uint m_pinDirection;

        pinMode_t m_pinMode;

        uint8_t m_numPins;
        std::vector<IoPin*> m_pins;

        uint m_index;
        std::vector<outState_t>* m_outVector;
        std::vector<std::vector<outState_t>> m_outVectors;
};

#endif
