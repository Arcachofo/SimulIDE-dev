/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef MCUINTERRUPTS_H
#define MCUINTERRUPTS_H

#include <QString>
#include <QMap>
#include <map>

#include "mcutypes.h"

class eMcu;
class Interrupts;
class McuModule;
class IoPin;

class Interrupt
{
        friend class McuCreator;

    public:
        Interrupt( QString name, uint16_t vector, eMcu* mcu );
        virtual ~Interrupt();

        virtual void reset();
        virtual void raise( uint8_t v=1 );
        virtual void execute();
        virtual void exitInt();

        uint8_t enabled() { return m_enabled; }
        uint8_t raised() { return m_raised; }
        void clearFlag();
        void flagCleared( uint8_t f=0 );
        void writeFlag( uint8_t v );
        void enableFlag( uint8_t en );

        uint8_t priority() { return m_priority; }
        void setPriority( uint8_t p ) { m_priority = p; }

        void setAutoClear( bool a ) { m_autoClear = a; }
        void setContinuous( bool c ); // Pin INT
        bool isContinuous() { return m_continuous; }

        void callBack( McuModule* mod, bool call );
        void exitCallBack( McuModule* mod, bool call );

        Interrupt* m_nextInt;

    protected:
        eMcu* m_mcu;
        uint8_t* m_ram;

        IoPin* m_intPin;

        Interrupts* m_interrupts;

        QString  m_name;
        uint8_t  m_number;
        uint16_t m_vector;

        uint8_t m_enabled;
        uint8_t m_priority;

        uint8_t  m_flagMask;
        uint16_t m_flagReg;

        uint8_t m_wakeup;

        bool m_raised;
        bool m_autoClear;
        bool m_remember;
        bool m_continuous;

        QList<McuModule*> m_callBacks;
        QList<McuModule*> m_exitCallBacks;
};

//------------------------               ------------------------
//---------------------------------------------------------------
class Interrupts
{
        friend class McuCreator;

    public:
        Interrupts( eMcu* mcu );
        virtual ~Interrupts();

        void enableGlobal( uint8_t en ) ;
        uint8_t enabled() { return m_enabled; }

        void runInterrupts();
        void retI() { m_reti = true; }
        void remove();
        void resetInts();
        void writeGlobalFlag( uint8_t flag );

        void addToPending( Interrupt* newInt );
        void remFromPending( Interrupt* remInt );

        Interrupt* getInterrupt( QString name );

    protected:
        eMcu* m_mcu;

        bool m_reti;

        regBits_t m_enGlobalFlag;

        uint8_t    m_enabled;   // Global Interrupt Flag
        Interrupt* m_active;    // Active interrupt

        Interrupt* m_pending;  // First pending Interrupt (linked list)
        Interrupt* m_running;  // First running Interrupt (linked list)

        QMap<QString, Interrupt*> m_intList;         // Access Interrupts by name
};

#endif
