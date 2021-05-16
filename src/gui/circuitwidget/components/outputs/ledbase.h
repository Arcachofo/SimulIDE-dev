/***************************************************************************
 *   Copyright (C) 2012 by santiago González                               *
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

#ifndef LEDBASE_H
#define LEDBASE_H

#include "e-led.h"
#include "component.h"
#include <QColor>

class MAINMODULE_EXPORT LedBase : public Component, public eLed
{
    Q_OBJECT
    Q_PROPERTY( LedColor Color    READ color      WRITE setColor      DESIGNABLE true USER true )
    Q_PROPERTY( double Threshold  READ threshold  WRITE setThreshold  DESIGNABLE true USER true )
    Q_PROPERTY( double MaxCurrent READ maxCurrent WRITE setMaxCurrent DESIGNABLE true USER true )
    Q_PROPERTY( double Resistance READ res        WRITE setRes        DESIGNABLE true USER true )
    Q_PROPERTY( bool   Grounded   READ grounded   WRITE setGrounded   DESIGNABLE true USER true )

    public:
        LedBase( QObject* parent, QString type, QString id );
        ~LedBase();
        
        enum LedColor {
            yellow = 0,
            red,
            green,
            blue,
            orange,
            purple
        };
        Q_ENUM( LedColor )
        
        void setColor( LedColor color ) { m_ledColor = color; }
        LedColor color() { return m_ledColor; } 

        bool grounded();
        void setGrounded( bool grounded );

        virtual void initialize();
        virtual void updateStep() override;
        virtual void remove() override;

        virtual void paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget );

    protected:
        virtual void drawBackground( QPainter *p )=0;
        virtual void drawForeground( QPainter *p )=0;
        
        bool   m_grounded;
        eNode* m_scrEnode;
        
        LedColor m_ledColor;
        
        bool m_overCurrent;
        int  m_counter;
};

#endif
