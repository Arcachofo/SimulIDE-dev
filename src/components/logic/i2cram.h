/***************************************************************************
 *   Copyright (C) 2018 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#pragma once

#include "twimodule.h"
#include "iocomponent.h"
#include "memdata.h"

class LibraryItem;

class I2CRam : public IoComponent, public TwiModule, public MemData
{
    public:
        I2CRam( QString type, QString id );
        ~I2CRam();

 static Component* construct( QString type, QString id );
 static LibraryItem* libraryItem();

        void stamp() override;
        void initialize() override;
        void updateStep() override;
        void voltChanged() override;

        void startWrite() override;
        void writeByte() override;
        void readByte() override;

        void setMem( QString m );
        QString getMem();
        
        int rSize() { return m_size; }
        void setRSize( int size );

        bool persistent() { return m_persistent; }
        void setPersistent( bool p ) { m_persistent = p; }

    public slots:
        void loadData() { MemData::loadData( &m_ram ); }
        void saveData() { MemData::saveData( &m_ram ); }
        void slotShowTable();
        void slotOpenMon();


    protected:
        void contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu ) override;

    private:
        QVector<int> m_ram;
        int m_size;
        int m_addrPtr;
        int m_phase;

        bool m_persistent;
};
