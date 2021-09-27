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

#ifndef SHAPE_H
#define SHAPE_H

#include "component.h"

class MAINMODULE_EXPORT Shape : public Component
{
    public:
        Shape( QObject* parent, QString type, QString id );
        ~Shape();
        
        QRectF boundingRect() const {
            return QRectF( m_area.x()-m_border/2-1, m_area.y()-m_border/2-1, 
                           m_area.width()+m_border+2, m_area.height()+m_border+2 ); 
        }
        int hSize() { return m_hSize; }
        void setHSize( int size );
        
        int vSize() { return m_vSize; }
        void setVSize( int size );
        
        int border() { return m_border; }
        void setBorder( int border );
        
        QColor color() { return m_color; }
        void setColor( QColor color );

        double zVal() { return zValue(); }
        void setZVal( double v) { setZValue( v ); }

        double opac() { return opacity(); }
        void setOpac( double o ) { setOpacity( o ); }


        QString colorStr() { return m_color.name(); }
        void setColorStr( QString n ) { setColor( QColor(n) ); }
        
    protected:
        int m_hSize;
        int m_vSize;
        int m_border;
};

#endif
