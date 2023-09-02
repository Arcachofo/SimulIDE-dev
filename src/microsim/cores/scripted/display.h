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
        Display( uint width, uint height, QString name, QWidget* parent );
        ~Display();

        virtual void initialize() override;
        virtual void updateStep() override;

        void setWidth( uint w );
        void setHeight( uint h );
        void setSize( uint w, uint h );
        void setBackground( int b );
        void setPixel( uint x, uint y, int color );

        //void setNextPixel( int color );
        //void setLine( std::vector<int> line );
        //void setStart( int x, int y ) { m_x = x; m_y = y; }

        void setMonitorScale( double scale );

        std::vector<std::vector<int>>* getBackData() { return &m_data; }

    protected:
        virtual void paintEvent( QPaintEvent* e ) override;

        void updtImageSize();

        bool m_changed;

        uint m_width;
        uint m_height;

        int m_x;
        int m_y;

        double m_scale;

        int m_background;

        std::vector<std::vector<int>> m_data;
        QRectF  m_area;
};

#endif
