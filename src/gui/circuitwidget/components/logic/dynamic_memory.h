/***************************************************************************
 *   Copyright (C) 2018 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef DYNAMICMEMORY_H
#define DYNAMICMEMORY_H

#include "logiccomponent.h"
#include "memdata.h"

class LibraryItem;

class MAINMODULE_EXPORT DynamicMemory : public LogicComponent, public MemData
{
    public:
        DynamicMemory( QObject* parent, QString type, QString id );
        ~DynamicMemory();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();

        virtual void stamp() override;
        virtual void updateStep() override;
        virtual void voltChanged() override;
        virtual void runEvent() override;

        int rowAddrBits() { return m_rowAddrBits; }
        int colAddrBits() { return m_colAddrBits; }
        void setRowAddrBits( int bits );
        void setColAddrBits( int bits );
        void setAddrBits( int bits );
        void deleteAddrBits( int bits );
        void createAddrBits( int bits );

        int dataBits() { return m_dataBits; }
        void setDataBits( int bits );
        void deleteDataBits( int bits );
        void createDataBits( int bits );

        double refreshPeriod() { return m_refreshPeriod*1e-12; }
        void setRefreshPeriod( double rp ) { m_refreshPeriod = rp*1e12; }

        void updatePins();

    public slots:
        void loadData();
        void saveData();
        void slotShowTable();

    protected:
        virtual void contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu ) override;

    private:
        void write( bool w );

        int m_refreshPeriod;
        int m_rowAddrBits;
        int m_colAddrBits;
        int m_addrBits;
        int m_dataBits;
        int m_dataBytes;
        int m_intRefresh;
        int m_rowAddress;
        int m_address;
        bool m_refreshError;

        QVector<int> m_ram;
        QVector<uint64_t> m_rowLastRefresh;

        bool m_ras;
        bool m_cas;
        bool m_we;
        bool m_write;

        IoPin* m_RasPin;
        IoPin* m_CasPin;
        IoPin* m_WePin;
};

#endif
