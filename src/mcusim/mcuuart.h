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
        McuUsart( eMcu* mcu, QString name );
        ~McuUsart();

        virtual void dataAvailable( uint8_t data ) override;

 static QHash<QString, UsartModule*> m_usarts; // Access Usarts by name

    protected:
        uint8_t* m_rxRegister;
};



#endif
