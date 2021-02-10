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

#ifndef USARTM_H
#define USARTM_H

#include<QHash>

#include "mcuuart.h"
//#include "regsignal.h"

class eMcu;

enum parity_t{
    parNONE=0,
    parEVEN,
    parODD,
};

class MAINMODULE_EXPORT UsartM
{
        friend class McuCreator;

    public:
        UsartM( eMcu* mcu,  QString name );
        ~UsartM();

        virtual void configure( uint8_t ){;}
        virtual void step( uint8_t ){;}

        virtual uint8_t getBit9(){return 0;}
        virtual void    setBit9( uint8_t bit ){;}

        void parityError();

        uint8_t m_mode;
        uint8_t m_stopBits;
        uint8_t m_dataBits;
        uint8_t m_dataMask;
        parity_t m_parity;

    protected:
        void setPeriod( uint64_t period );

        QString m_name;
        eMcu*   m_mcu;

        UartTx m_sender;
        UartRx m_receiver;

        bool m_running;   // is Uart running?

 static QHash<QString, UsartM*> m_usarts; // Access Usarts by name
};

#endif
