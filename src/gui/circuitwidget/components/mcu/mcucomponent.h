/***************************************************************************
 *   Copyright (C) 2012 by santiago González                               *
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

#ifndef MCUCOMPONENT_H
#define MCUCOMPONENT_H

#include "mcubase.h"

class BaseProcessor;
class McuComponentPin;
class MCUMonitor;

class MAINMODULE_EXPORT McuComponent : public McuBase
{
    Q_OBJECT

    public:
        McuComponent( QObject* parent, QString type, QString id );
        ~McuComponent();

        virtual void initialize() override;
        virtual void updateStep() override;
        virtual void attach() override;
        virtual void remove() override;

        virtual void initChip();

        QStringList varList();
        void setVarList( QStringList vl );

        QString program() { return m_symbolFile; }
        void setProgram( QString pro );

        virtual double freq() override { return m_freq; }
        virtual void setFreq( double freq ) override;

        bool autoLoad() { return m_autoLoad; }
        void setAutoLoad( bool al ) { m_autoLoad = al; }

        void setEeprom(QVector<int> eep );
        QVector<int> eeprom();
        
        QList<McuComponentPin*> getPinList() { return m_pinList; }

        virtual void terminate();
        virtual void reset() override;
        virtual bool load( QString fileName );
        virtual void setSubcDir( QString dir );
        virtual void crash(){;}

        BaseProcessor* processor() { return m_processor; }

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    signals:
        void closeSerials();
        void openSerials();
  
    public slots:
        void slotmain();
        void slotLoad();
        void slotReload();
        void slotOpenTerm();
        void slotOpenSerial();
        void slotOpenMcuMonitor();

        void loadData();
        void saveData();

    protected:
        virtual void contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu );
        virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent* event);

        virtual void addPin( QString id, QString type, QString label,
                             int pos, int xpos, int ypos, int angle, int length=8 )=0;

        virtual void updatePin( QString id, QString type, QString label,
                                int pos, int xpos, int ypos, int angle );

        virtual void attachPins()=0;

        BaseProcessor* m_processor;

        double m_freq;           // Clock Frequency Mhz
        int m_cpi;
        
        bool m_attached;
        bool m_autoLoad;

        int m_warning;

        QString m_symbolFile;   // firmware file loaded
        QString m_lastFirmDir;  // Last firmware folder used
        QString m_subcDir;      // Subcircuit Path

        QList<McuComponentPin*> m_pinList;

        MCUMonitor* m_mcuMonitor;
};
#endif
