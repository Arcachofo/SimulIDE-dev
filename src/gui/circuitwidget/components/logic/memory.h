/***************************************************************************
 *   Copyright (C) 2018 by santiago González                               *
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

#ifndef MEMORY_H
#define MEMORY_H

#include "logiccomponent.h"
#include "memdata.h"

class LibraryItem;

class MAINMODULE_EXPORT Memory : public LogicComponent, public MemData
{
    Q_OBJECT
    Q_PROPERTY( QVector<int> Mem  READ mem        WRITE setMem )
    Q_PROPERTY( int  Address_Bits READ addrBits   WRITE setAddrBits   DESIGNABLE true USER true )
    Q_PROPERTY( int  Data_Bits    READ dataBits   WRITE setDataBits   DESIGNABLE true USER true )
    Q_PROPERTY( bool Persistent   READ persistent WRITE setPersistent DESIGNABLE true USER true )

    public:
        Memory( QObject* parent, QString type, QString id );
        ~Memory();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();

        virtual QList<propGroup_t> propGroups() override;

        virtual void updateStep() override;
        virtual void stamp() override;
        virtual void initialize() override;
        virtual void voltChanged() override;
        virtual void runEvent() override;

        void setMem( QVector<int> m );
        QVector<int> mem();

        int addrBits(){ return m_addrBits; }
        void setAddrBits( int bits );
        void deleteAddrBits( int bits );
        void createAddrBits( int bits );

        int dataBits(){ return m_dataBits; }
        void setDataBits( int bits );
        void deleteDataBits( int bits );
        void createDataBits( int bits );

        bool persistent() { return m_persistent; }
        void setPersistent( bool p ){ m_persistent = p; }

        void updatePins();

        virtual void remove() override;

    public slots:
        void loadData();
        void saveData();
        void showTable();
        virtual void contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu );

    protected:
        virtual void contextMenuEvent( QGraphicsSceneContextMenuEvent* event );
        
    private:
        int m_addrBits;
        int m_dataBits;
        int m_dataBytes;
        int m_address;

        QVector<int> m_ram;

        bool m_we;
        bool m_cs;
        bool m_oe;
        bool m_read;
        bool m_persistent;

        IoPin* m_CsPin;
        IoPin* m_WePin;
};

#endif

