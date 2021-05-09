/***************************************************************************
 *   Copyright (C) 2021 by santiago González                               *
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

#ifndef TWISMASTER_H
#define TWIMASTER_H

#include "twimodule.h"

class Component;

class MAINMODULE_EXPORT TwiMaster : public TwiTR
{
    public:
        TwiMaster( TwiModule* twi, QString name );
        ~TwiMaster();

        virtual void stamp() override;
        virtual void runEvent() override;

        virtual void masterStart( uint8_t addr );
        virtual void masterWrite( uint8_t data );
        virtual void masterRead();

    protected:
        void keepClocking();

        bool m_toggleScl;

        Component* m_comp;
};

#endif

