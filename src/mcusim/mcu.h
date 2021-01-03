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

#ifndef MCU_H
#define MCU_H

#include <QHash>

#include "e_mcu.h"
#include "chip.h"

class LibraryItem;

class MAINMODULE_EXPORT Mcu : public Chip
{
        Q_OBJECT
        Q_PROPERTY( double   Mhz         READ freq     WRITE setFreq     DESIGNABLE true  USER true )
        Q_PROPERTY( QString  Program     READ program  WRITE setProgram  DESIGNABLE true  USER true )

        friend class McuCreator;

    public:
        Mcu( QObject* parent, QString type, QString id );
        ~Mcu();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();

        QString program()   const { return m_firmware; }
        void setProgram( QString pro );

        double freq(){ return m_eMcu.m_freq; }
        virtual void setFreq( double freq ) { m_eMcu.setFreq( freq ); }

        virtual void initialize() override;
        virtual void remove() override;
        virtual void setLogicSymbol( bool ls ) override;

        void load( QString fileName );

        virtual void addPin( QString id, QString type, QString label,
                             int pos, int xpos, int ypos, int angle , int length=8);
        QString loadHex( QString file, int WordSize );

    public slots:
        void slotLoad();

    protected:
        virtual void contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu );
        virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent* event);

        QString m_subcDir;      // Subcircuit Path
        QString m_lastFirmDir;  // Last firmware folder used
        QString m_firmware;     // firmware file loaded
        QString m_dataFile;
        QString m_device;       // Name of device

        eMcu m_eMcu;

        QList<Pin*> m_pinList;
};


#endif
