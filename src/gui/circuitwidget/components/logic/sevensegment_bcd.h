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
        Q_OBJECT
    public:
        SevenSegmentBCD( QObject* parent, QString type, QString id );
        ~SevenSegmentBCD();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();

        void updateStep() override;
        
        void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );
};

#endif
