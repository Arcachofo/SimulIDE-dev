/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "plotspinbox.h"
#include <QDebug>

PlotSpinBox::PlotSpinBox( QWidget* parent )
           : QDoubleSpinBox( parent )
{
    m_this = this;
}
PlotSpinBox::~PlotSpinBox(){;}

QValidator::State PlotSpinBox::validate( QString& text, int& n ) const
{
    QStringList l =text.split(" ");
    QString val = l.first();
    QString suf = l.last();

    if     ( val.contains("p") ) { val.remove("p"); m_this->setSuffix(" p"+m_unit); }
    else if( val.contains("n") ) { val.remove("n"); m_this->setSuffix(" n"+m_unit); }
    else if( val.contains("u") ) { val.remove("u"); m_this->setSuffix(" u"+m_unit); }
    else if( val.contains("m") ) { val.remove("m"); m_this->setSuffix(" m"+m_unit); }
    else if( val.contains("k") ) { val.remove("k"); m_this->setSuffix(" k"+m_unit); }
    else if( val.contains("M") ) { val.remove("M"); m_this->setSuffix(" M"+m_unit); }
    else if( val.contains("G") ) { val.remove("G"); m_this->setSuffix(" G"+m_unit); }
    else if( val.contains(m_unit) ) { val.remove(m_unit); m_this->setSuffix(" "+m_unit); }
    else{
        QString unitL = m_unit.toLower();
        if( val.contains( unitL ) ) { val.remove(unitL); m_this->setSuffix(" "+m_unit); }
    }
    text = val+" "+suf;

    QValidator::State state = QDoubleSpinBox::validate( val, n );
    return state;
}

