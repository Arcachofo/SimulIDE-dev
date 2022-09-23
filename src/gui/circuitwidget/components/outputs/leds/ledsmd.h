/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef LEDSMD_H
#define LEDSMD_H

#include "ledbase.h"

class MAINMODULE_EXPORT LedSmd : public LedBase
{
    public:
        LedSmd( QObject* parent, QString type, QString id, QRectF area, ePin* pin0=NULL, ePin* pin1=NULL );
        ~LedSmd();

    protected:
        void drawBackground( QPainter* p );
        void drawForeground( QPainter* p );
};

#endif
