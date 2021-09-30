/***************************************************************************
 *   Copyright (C) 2018 by santiago González                               *
 *   santigoro@gmail.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.  *
 *                                                                         *
 ***************************************************************************/

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
    m_nextOutVal = m_values[0];
}
BcdBase::~BcdBase(){}

void BcdBase::initialize()
{
    LogicComponent::initialize();
    update();
}

void BcdBase::stamp()
{
    LogicComponent::stamp();
    for( int i=0; i<4; ++i ) m_inPin[i]->changeCallBack( this );

    m_nextOutVal = m_values[0];
    m_changed = true;
}

void BcdBase::voltChanged()
{
    m_changed = true;

    bool a = m_inPin[0]->getInpState();
    bool b = m_inPin[1]->getInpState();
    bool c = m_inPin[2]->getInpState();
    bool d = m_inPin[3]->getInpState();

    int digit = a*1+b*2+c*4+d*8;
    m_nextOutVal = m_values[digit];
}
