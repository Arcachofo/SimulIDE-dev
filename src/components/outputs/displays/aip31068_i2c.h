/***************************************************************************
 *   Copyright (C) 2016 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#pragma once

#include "hd44780_base.h"
#include "twimodule.h"

class LibraryItem;
class IoPin;

class Aip31068_i2c : public Hd44780_Base, public TwiModule
{
    public:
        Aip31068_i2c( QString type, QString id );
        ~Aip31068_i2c();
        
        static Component* construct( QString type, QString id );
        static LibraryItem* libraryItem();

        virtual void initialize() override;
        virtual void stamp() override;
        virtual void updateStep() override;
        virtual void voltChanged() override;

        virtual void startWrite() override;
        virtual void readByte() override;

        void showPins( bool show );

    public slots:
        void slotOpenMon();

    protected:
        void contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu ) override;

    private:
        int m_controlByte;
        int m_phase;

        IoPin* m_pinSda;
};
