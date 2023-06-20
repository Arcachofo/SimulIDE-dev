/***************************************************************************
 *   Copyright (C) 2018 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

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

        double opac() { return m_opac; }
        void setOpac( double op );


        QString colorStr() { return m_color.name(); }
        void setColorStr( QString n ) { setColor( QColor(n) ); }
        
    protected:
        int m_hSize;
        int m_vSize;
        int m_border;

        double m_opac;
};

#endif
