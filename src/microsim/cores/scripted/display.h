/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef DISPLAY_H
#define DISPLAY_H

#include <QWidget>

#include "updatable.h"
#include "e-element.h"

#include "scriptarray.h"

class Display : public QWidget, public Updatable, public eElement
{
    public:
        Display( int width, int height, QString name, QWidget* parent );
        ~Display();

        virtual void initialize() override;
        virtual void updateStep() override;

        void setWidth( int w );
        void setHeight( int h );
        void setSize( int w, int h );
        void setBackground( int b );
        void setPixel( int x, int y, int color );
        //void setNextPixel( int color );
        //void setLine( std::vector<int> line );
        //void setStart( int x, int y ) { m_x = x; m_y = y; }

        void setMonitorScale( double scale );

        QImage* getImage() { return &m_image; }

    protected:
        virtual void paintEvent( QPaintEvent* e ) override;

        void updtImageSize();

        bool m_changed;

        int m_width;
        int m_height;

        int m_x;
        int m_y;

        double m_scale;

        int m_background;

        QRectF  m_area;

        QImage m_image;    //Visual representation
};

#endif
