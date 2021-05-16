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

#ifndef I2CRAM_H
#define I2CRAM_H

#include "twimodule.h"
#include "iocomponent.h"
#include "memdata.h"

class LibraryItem;

class MAINMODULE_EXPORT I2CRam : public IoComponent, public TwiModule, public MemData
{
    Q_OBJECT
    Q_PROPERTY( QVector<int> Mem  READ mem        WRITE setMem )
    Q_PROPERTY( double Frequency  READ freqKHz    WRITE setFreqKHz    DESIGNABLE true USER true )
    Q_PROPERTY( int  Control_Code READ cCode      WRITE setCcode      DESIGNABLE true USER true )
    Q_PROPERTY( int  Size_bytes   READ rSize      WRITE setRSize      DESIGNABLE true USER true )
    Q_PROPERTY( bool Persistent   READ persistent WRITE setPersistent DESIGNABLE true USER true )

    public:
        I2CRam( QObject* parent, QString type, QString id );
        ~I2CRam();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();
        
        virtual QList<propGroup_t> propGroups() override;

        virtual void stamp() override;
        virtual void initialize() override;
        virtual void updateStep() override;
        virtual void voltChanged() override;

        virtual void startWrite() override;
        virtual void writeByte() override;
        virtual void readByte() override;

        void setMem( QVector<int> m );
        QVector<int> mem();

        int cCode();
        void setCcode( int code );
        
        int rSize();
        void setRSize( int size );

        bool persistent();
        void setPersistent( bool p );


    public slots:
        void loadData();
        void saveData();
        void showTable();
        virtual void contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu );

    protected:
        virtual void contextMenuEvent( QGraphicsSceneContextMenuEvent* event );

    private:
        QVector<int> m_ram;
        int m_size;
        int m_addrPtr;
        int m_cCode;
        int m_phase;

        bool m_persistent;
};

#endif

