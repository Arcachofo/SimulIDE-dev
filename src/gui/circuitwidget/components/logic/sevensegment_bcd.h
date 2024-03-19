/***************************************************************************
 *   Copyright (C) 2010 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef SEVENSEGMENTBCD_H
#define SEVENSEGMENTBCD_H

#include "bcdbase.h"

class LibraryItem;

class SevenSegmentBCD : public BcdBase
{
    public:
        SevenSegmentBCD( QString type, QString id );
        ~SevenSegmentBCD();

 static Component* construct( QString type, QString id );
 static LibraryItem* libraryItem();

        virtual void updateStep() override;
        virtual void voltChanged() override;

        bool isShowEnablePin() { return m_showEnablePin; }
        void setShowEnablePin( bool show );

        bool isShowDotPin() { return m_showDotPin; }
        void setShowDotPin( bool show );

        virtual bool setLinkedTo( Linker* li ) override;
        virtual void setLinkedValue( double v, int i=0  ) override;

        void paint( QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w ) override;

    private:
        bool m_showEnablePin;
        bool m_showDotPin;

        IoPin *m_dotPin;
        IoPin *m_enablePin;
};

#endif
