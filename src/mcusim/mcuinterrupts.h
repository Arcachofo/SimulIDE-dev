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

/*enum {
    AVR_INT_IRQ_PENDING = 0,
    AVR_INT_IRQ_RUNNING,
    AVR_INT_IRQ_COUNT,
    AVR_INT_ANY        = 0xff,    // for avr_get_interrupt_irq()
};*/

class eMcu;

class MAINMODULE_EXPORT Interrupt
{
        friend class McuCreator;

    public:
        Interrupt( QString name, uint16_t vector, eMcu* mcu );
        virtual ~Interrupt();

 static void enableGlobal( uint8_t en );
 static void runInterrupts();
 static void retI();
 static void remove();
 static void resetInts();

        virtual void reset();
        virtual void raise( uint8_t v );
        virtual void execute();
        virtual void exitInt();

        void enableFlag( uint8_t en );
        void setPriority( uint8_t p );
        void setMode( uint8_t mod ){ m_mode = mod;}
        //void readFlag( uint8_t p );

        void clearFlag();

        uint8_t priority() { return m_priority; }

    protected:
        eMcu* m_mcu;
        uint8_t* m_ram;

        QString  m_name;
        uint8_t  m_number;
        uint16_t m_vector;
        uint8_t  m_mode;

        uint8_t m_enable;

        uint8_t  m_flagMask;
        uint16_t m_flagReg;

        uint8_t m_priority;
        uint8_t m_raised;

 static uint8_t    m_enGlobal;   // Global Interrupt Flag
 static Interrupt* m_active;     // Active interrupt

 static std::multimap<uint8_t, Interrupt*> m_running; // Interrups thay were interrupted
 static std::multimap<uint8_t, Interrupt*> m_pending; // Interrupts pending to service
 static QHash<QString, Interrupt*> m_interrupts;      // Access Interrupts by name
};

#endif
