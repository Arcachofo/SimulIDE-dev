/***************************************************************************
 *   Copyright (C) 2020 by santiago González                               *
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

#ifndef MCUINTERRUPTS_H
#define MCUINTERRUPTS_H

#include <QString>
#include <QHash>
#include <map>

class eMcu;
class Interrupts;
class McuModule;

class MAINMODULE_EXPORT Interrupt
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
        void writeFlag( uint8_t v );
        void enableFlag( uint8_t en );

        uint8_t priority() { return m_priority; }
        void setPriority( uint8_t p ) { m_priority = p; }

        void setMode( uint8_t mod )   { m_mode = mod;}

        void callBack( McuModule* mod, bool call );
        void exitCallBack( McuModule* mod, bool call );

    protected:
        eMcu* m_mcu;
        uint8_t* m_ram;

        Interrupts* m_interrupts;

        QString  m_name;
        uint8_t  m_number;
        uint16_t m_vector;
        uint8_t  m_mode;

        uint8_t m_enabled;

        uint8_t  m_flagMask;
        uint16_t m_flagReg;

        uint8_t m_priority;
        uint8_t m_raised;

        bool m_autoClear;
        bool m_remember;

        QList<McuModule*> m_callBacks;
        QList<McuModule*> m_exitCallBacks;
};

//------------------------               ------------------------
//---------------------------------------------------------------
class MAINMODULE_EXPORT Interrupts
{
        friend class McuCreator;

    public:
        Interrupts( eMcu* mcu );
        virtual ~Interrupts();

        void enableGlobal( uint8_t en ) { m_enabled = en; }
        uint8_t enabled() { return m_enabled; }
        void runInterrupts();
        void retI();
        void remove();
        void resetInts();
        void addToPending( uint8_t pri, Interrupt* i );

    protected:
        eMcu* m_mcu;

        uint8_t    m_enabled;   // Global Interrupt Flag
        Interrupt* m_active;    // Active interrupt

        std::multimap<uint8_t, Interrupt*> m_running; // Interrups that were interrupted
        std::multimap<uint8_t, Interrupt*> m_pending; // Interrupts pending to service
        QHash<QString, Interrupt*> m_intList;         // Access Interrupts by name
};

#endif
