/***************************************************************************
 *   Copyright (C) 2018 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "bcdbase.h"
#include "iopin.h"

const uint8_t BcdBase::m_values[]={
        0b00111111,
        0b00000110,
        0b01011011,
        0b01001111,
        0b01100110,
        0b01101101,
        0b01111101,
        0b00000111,
        0b01111111,
        0b01101111,
        0b01110111,
        0b01111100,
        0b00111001,
        0b01011110,
        0b01111001,
        0b01110001,
        0b00000000
};

BcdBase::BcdBase( QObject* parent, QString type, QString id )
       : LogicComponent( parent, type, id )
{
    m_digit = m_values[0];
}
BcdBase::~BcdBase(){}

void BcdBase::initialize()
{
    m_digit = m_values[0];
    update();
}

void BcdBase::stamp()
{
    LogicComponent::stamp();
    for( int i=0; i<4; ++i ) m_inPin[i]->changeCallBack( this );

    m_changed = true;
}

void BcdBase::voltChanged()
{
    m_changed = true;

    bool a = m_inPin[0]->getInpState();
    bool b = m_inPin[1]->getInpState();
    bool c = m_inPin[2]->getInpState();
    bool d = m_inPin[3]->getInpState();

    m_digit = m_values[a*1+b*2+c*4+d*8];
}
