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

#ifndef BOARD_H
#define BOARD_H

#include "subcircuit.h"

class ShieldSubc;

class MAINMODULE_EXPORT BoardSubc : public SubCircuit
{
    Q_OBJECT

    public:
        BoardSubc( QObject* parent, QString type, QString id );
        ~BoardSubc();

        void attachShield( ShieldSubc* shield );
        void detachShield( ShieldSubc* shield ) { m_shields.removeAll( shield); }

        virtual void remove() override;

    protected:
        QList<ShieldSubc*> m_shields; // A shield attached to this

};
#endif
