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
class eMcu;
class Component;
class QString;
class QDomElement;
class McuModule;
class Interrupt;

class MAINMODULE_EXPORT McuCreator
{
    public:
        McuCreator();
        ~McuCreator();

        static int createMcu( Mcu* mcuComp, QString name );
        static void convert( QString fileName );
    private:
        static int  processFile( QString fileName );
        static void createProgMem( uint32_t size );
        static void createDataMem( uint32_t size );
        static void createRomMem( uint32_t size );
        static void createEeprom( QDomElement* e );
        //static void createCfgWord( QDomElement* e );
        static void createDataBlock( QDomElement* d );
        static void createRegisters( QDomElement* e );
        static void getRegisters( QDomElement* e, uint16_t offset=0 );
        static void createProgBlock( QDomElement* p );
        static void createInterrupts( QDomElement* i );
        static void createPort( QDomElement* p );
        static void createTimer( QDomElement* t );
        static void createCcpUnit( QDomElement* c );
        static void createUsart( QDomElement* u );
        static void createAdc( QDomElement* e );
        static void createAcomp( QDomElement* e );
        static void createVref( QDomElement* e );
        static void createTwi( QDomElement* e );
        static void createSpi( QDomElement* e );
        static void createWdt( QDomElement* e );
        static void createCore( QString core );
        static void createStack( QDomElement* s );
        static void createInterrupt( QDomElement* el );
        static void setInterrupt( QDomElement* el, McuModule* module );
        static void setConfigRegs( QDomElement* u, McuModule* module );

        static QString m_CompName;
        static QString m_basePath;
        static QString m_core;
        static QString m_txRegName;

        static QDomElement m_stackEl;
        static Mcu* m_mcuComp;
        static eMcu* mcu;
};

#endif
