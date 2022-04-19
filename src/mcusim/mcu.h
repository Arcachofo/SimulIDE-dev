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
class MCUMonitor;

class MAINMODULE_EXPORT Mcu : public Chip
{
        Q_OBJECT

        friend class McuCreator;

    public:
        Mcu( QObject* parent, QString type, QString id );
        ~Mcu();

 static Mcu* self() { return m_pSelf; }
 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem* libraryItem();

        virtual bool setPropStr( QString prop, QString val ) override;

        QString program() { return m_eMcu.getFileName(); }
        void setProgram( QString pro );

        bool autoLoad() { return m_autoLoad; }
        void setAutoLoad( bool al ) { m_autoLoad = al; }

        double freq() { return m_eMcu.m_freq; }
        void setFreq( double freq ) { m_eMcu.setFreq( freq ); }

        bool rstPinEnabled();
        void enableRstPin( bool en );

        bool extOscEnabled() { return m_extClock; }
        void enableExtOsc( bool en );

        bool wdtEnabled();
        void enableWdt( bool en );

        QString varList();
        void setVarList( QString vl );

        void setEeprom( QString eep );
        QString getEeprom();

        virtual void initialize() override;
        virtual void stamp() override;
        virtual void updateStep() override;
        virtual void voltChanged() override;
        virtual void remove() override;

        QString device() { return m_device; }

        void reset() { m_eMcu.cpuReset( true ); }

        bool load( QString fileName );

        virtual void addPin( QString id, QString type, QString label,
                             int pos, int xpos, int ypos, int angle , int length=8);

        //void createCfgWord( QString name, uint16_t addr, uint16_t v );

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    public slots:
        void slotmain();
        void slotLoad();
        void slotReload();
        void slotOpenTerm( int num );
        void slotOpenMcuMonitor();

        void loadEEPROM();
        void saveEEPROM();

    protected:
 static Mcu* m_pSelf;

        virtual void contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu );
        virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent* event);

        bool m_autoLoad;
        bool m_extClock;

        QString m_subcDir;      // Subcircuit Path
        QString m_lastFirmDir;  // Last firmware folder used
        QString m_dataFile;
        QString m_device;       // Name of device

        eMcu m_eMcu;

        IoPin*  m_resetPin;
        McuPin* m_mcuRstPin;
        McuPin* m_clkPin[2];

        QList<Pin*> m_pinList;

        MCUMonitor* m_mcuMonitor;
};

#endif
