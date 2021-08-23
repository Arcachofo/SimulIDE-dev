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
#include "mcubase.h"
//#include "chip.h"

class LibraryItem;
class MCUMonitor;

class MAINMODULE_EXPORT Mcu : public McuBase
{
        Q_OBJECT
        Q_PROPERTY( QStringList varList  READ varList  WRITE setVarList )
        Q_PROPERTY( QVector<int> eeprom  READ eeprom   WRITE setEeprom )
        Q_PROPERTY( double   Mhz         READ freq     WRITE setFreq     DESIGNABLE true  USER true )
        Q_PROPERTY( QString  Program     READ program  WRITE setProgram  DESIGNABLE true  USER true )
        Q_PROPERTY( bool     Auto_Load   READ autoLoad WRITE setAutoLoad DESIGNABLE true  USER true )

        friend class McuCreator;

    public:
        Mcu( QObject* parent, QString type, QString id );
        ~Mcu();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();

        virtual QList<propGroup_t> propGroups() override;

        QStringList varList();
        void setVarList( QStringList vl );

        void setEeprom(QVector<int> eep );
        QVector<int> eeprom();

        QString program();
        void setProgram( QString pro );

        bool autoLoad() { return m_autoLoad; }
        void setAutoLoad( bool al ) { m_autoLoad = al; }

        virtual double freq() override { return m_eMcu.m_freq; }
        virtual void setFreq( double freq ) { m_eMcu.setFreq( freq ); }

        virtual void initialize() override;
        virtual void updateStep() override;
        virtual void attach() override;
        virtual void remove() override;

        virtual void reset() override;

        virtual bool load( QString fileName ) override;

        virtual void addPin( QString id, QString type, QString label,
                             int pos, int xpos, int ypos, int angle , int length=8);
        QString loadHex( QString file, int WordSize );

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    public slots:
        void slotmain();
        void slotLoad();
        void slotReload();
        void slotOpenTerm( int num );
        void slotOpenMcuMonitor();

    protected:
        virtual void contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu );
        virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent* event);

        bool m_autoLoad;

        QString m_subcDir;      // Subcircuit Path
        QString m_lastFirmDir;  // Last firmware folder used
        QString m_dataFile;
        //QString m_device;       // Name of device

        eMcu m_eMcu;

        QList<Pin*> m_pinList;

        MCUMonitor* m_mcuMonitor;
};


#endif
