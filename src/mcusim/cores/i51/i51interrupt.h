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

#ifndef I51INTERRUPT_H
#define I51INTERRUPT_H

#include "mcuinterrupts.h"

class MAINMODULE_EXPORT I51Interrupt : public Interrupt
{
    public:
        I51Interrupt( QString name, uint16_t vector, eMcu* mcu );
        ~I51Interrupt();

 static Interrupt* getInterrupt( QString name, uint16_t vector, eMcu* mcu );

    private:
};
//_________________________________________________________________
//_________________________________________________________________

class MAINMODULE_EXPORT I51ExtInt : public Interrupt
{
    public:
        I51ExtInt( QString name, uint16_t vector, eMcu* mcu );
        ~I51ExtInt();

        virtual void reset() override;
        virtual void raise( uint8_t v ) override;
        virtual void exitInt() override;

    private:
        uint8_t m_lastValue;
};
//_________________________________________________________________
//_________________________________________________________________

class I51Usart;

class MAINMODULE_EXPORT I51T1Int : public Interrupt
{
    public:
        I51T1Int( QString name, uint16_t vector, eMcu* mcu );
        ~I51T1Int();

        virtual void reset() override;
        virtual void raise( uint8_t v ) override;
        //virtual void exitInt() override;

        void setUsart( I51Usart* usart ) { m_usart = usart; }

    private:
        I51Usart* m_usart;
};

#endif
