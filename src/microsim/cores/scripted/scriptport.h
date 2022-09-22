/***************************************************************************
 *   Copyright (C) 2022 by santiago González                               *
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

#ifndef SCRIPTPORT_H
#define SCRIPTPORT_H

#include "scriptmodule.h"
#include "mcuport.h"

class MAINMODULE_EXPORT ScriptPort : public ScriptModule, public McuPort
{
    public:
        ScriptPort( eMcu* mcu, QString name );
        ~ScriptPort();

        virtual void configureA( uint8_t newVal ) override;
        virtual void configureB( uint8_t newVal ) override;
        virtual void configureC( uint8_t newVal ) override;

        virtual void reset() override;

        virtual void setScript( QString script ) override;

        void setExtIntTrig( int pinNumber, int trig );

    protected:


};
#endif
