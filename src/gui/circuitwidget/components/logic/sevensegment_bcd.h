/***************************************************************************
 *   Copyright (C) 2010 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef SEVENSEGMENTBCD_H
#define SEVENSEGMENTBCD_H

#include "bcdbase.h"

class LibraryItem;

class MAINMODULE_EXPORT SevenSegmentBCD : public BcdBase
{
    public:
        SevenSegmentBCD( QObject* parent, QString type, QString id );
        ~SevenSegmentBCD();

 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem* libraryItem();

        virtual void updateStep() override;

        virtual void setLinked( bool l ) override;
        virtual void setLinkedValue( int v, int i=0  ) override;
        
        void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget ) override;

    private:
        int m_intensity;
};

#endif
