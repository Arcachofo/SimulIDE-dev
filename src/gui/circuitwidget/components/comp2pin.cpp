/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "comp2pin.h"
#include "pin.h"

Comp2Pin::Comp2Pin( QObject* parent, QString type, QString id )
        : Component( parent, type, id )
{
    m_area = QRectF(-11,-4.5, 22, 9 );

    m_pin.resize(2);
    m_pin[0] = new Pin( 180, QPoint(-16, 0 ), id+"-lPin", 0, this);
    m_pin[1] = new Pin( 0,   QPoint( 16, 0 ), id+"-rPin", 1, this);
    
    setValLabelPos(-16, 6, 0 );
    setLabelPos(-16,-24, 0 );
}
