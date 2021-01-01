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

#ifndef MCUCREATOR_H
#define MCUCREATOR_H

#include <QHash>

class Mcu;
class Component;
class QString;
class QDomElement;

class MAINMODULE_EXPORT McuCreator
{
    public:
        McuCreator();
        ~McuCreator();

        static int createMcu( Mcu* mcu );

    private:
        static void createProgMem( uint32_t size , Mcu* mcu);
        static void createDataMem( uint32_t size, Mcu* mcu );
        static void createRegisters( QDomElement* e, Mcu* mcu);
        static void createInterrupts( QDomElement* i, Mcu* mcu );
        static void createPort( QDomElement* p, Mcu* mcu );
        static void createTimer( QDomElement* t, Mcu* mcu );
        static void createCore( QString core, Mcu* mcu);
        static void createStack( QDomElement* s, Mcu* mcu );

        static QString m_core;
};

#endif
