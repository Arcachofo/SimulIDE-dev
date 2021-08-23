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

#ifndef MCUUART_H
#define MCUUART_H

#include "usartmodule.h"
#include "mcumodule.h"



class MAINMODULE_EXPORT McuUsart : public McuModule, public UsartModule
{
        friend class McuCreator;

    public:
        McuUsart( eMcu* mcu, QString name, int number );
        virtual ~McuUsart();

        virtual void sendByte( uint8_t data ) override{;}
        virtual void bufferEmpty() override;
        virtual void frameSent( uint8_t data ) override;
        virtual void readByte( uint8_t data ) override {;}
        virtual void byteReceived( uint8_t data ) override;

    protected:
        int m_number;
};

// ----------------------------------------

/*class MAINMODULE_EXPORT McuUsarts
{
        friend class McuCreator;

    public:
        McuUsarts( eMcu* mcu );
        ~McuUsarts();

       void remove();
       McuUsart* getUsart( int number ) { return m_usartList.at(number); }

    protected:
       eMcu* m_mcu;

       std::vector<McuUsart*> m_usartList;// Access Usarts by name
};*/

#endif
