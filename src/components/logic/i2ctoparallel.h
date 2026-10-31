/***************************************************************************
 *   Copyright (C) 2018 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#pragma once

#include "twimodule.h"
#include "iocomponent.h"

class LibraryItem;

class I2CToParallel : public IoComponent, public TwiModule
{
    public:
        I2CToParallel( QString type, QString id );
        ~I2CToParallel();

 static Component* construct( QString type, QString id );
 static LibraryItem* libraryItem();

        void stamp() override;
        void voltChanged() override;
        void writeByte() override;
        void readByte() override;
        void startWrite() override;

    public slots:
        void slotOpenMon();

    protected:
        void contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu ) override;

        IoPin* m_int;

        uint8_t m_portState;
};
