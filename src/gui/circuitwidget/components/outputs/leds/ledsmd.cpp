/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QPainter>

#include "ledsmd.h"
#include "e-pin.h"
#include "label.h"

LedSmd::LedSmd( QObject* parent, QString type, QString id, QRectF area, ePin* pin0, ePin* pin1 )
      : LedBase( parent, type, id )
{
    m_area = area;
    m_valLabel->setEnabled( false );

    if( !pin0 ) pin0 = new ePin( m_elmId+"-ePin"+QString::number(0), 0 );
    if( !pin1 ) pin1 = new ePin( m_elmId+"-ePin"+QString::number(1), 1 );
    setEpin( 0, pin0 );
    setEpin( 1, pin1 );
}
LedSmd::~LedSmd(){}

void LedSmd::drawBackground( QPainter* p )
{
    p->drawRoundedRect( m_area, 0, 0);
}

void LedSmd::drawForeground( QPainter* p )
{
    p->drawRoundedRect( m_area, 0, 0 );
}
