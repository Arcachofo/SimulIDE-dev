/***************************************************************************
 *   Copyright (C) 2018 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef I2CRAM_H
#define I2CRAM_H

#include "twimodule.h"
#include "iocomponent.h"
#include "memdata.h"

class LibraryItem;

class MAINMODULE_EXPORT I2CRam : public IoComponent, public TwiModule, public MemData
{
    public:
        I2CRam( QObject* parent, QString type, QString id );
        ~I2CRam();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();

        virtual void stamp() override;
        virtual void initialize() override;
        virtual void updateStep() override;
        virtual void voltChanged() override;

        virtual void startWrite() override;
        virtual void writeByte() override;
        virtual void readByte() override;

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


    protected:
        virtual void contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu ) override;

    private:
        QVector<int> m_ram;
        int m_size;
        int m_addrPtr;
        int m_phase;

        bool m_persistent;
};

#endif
