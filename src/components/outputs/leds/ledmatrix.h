/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef LEDMATRIX_H
#define LEDMATRIX_H

#include "component.h"
#include "e-element.h"
#include "ledsmd.h"

class LibraryItem;

class LedMatrix : public Component, public eElement
{
    public:
        LedMatrix( QString type, QString id );
        ~LedMatrix();

 static Component* construct( QString type, QString id );
 static LibraryItem* libraryItem();

        QString colorStr();
        void setColorStr( QString color );

        int  rows() { return m_rows; }
        void setRows( int rows );
        
        int  cols() { return m_cols; }
        void setCols( int cols );
        
        bool verticalPins() { return m_verticalPins; }
        void setVerticalPins( bool v );
        
        double threshold() { return m_threshold; }
        void   setThreshold( double threshold );

        double maxCurrent() { return m_maxCurrent; }
        void   setMaxCurrent( double current );
        
        double resistance() { return m_resistance; }
        void  setResistance( double resist );

        virtual void stamp() override;
        virtual void remove() override;

        virtual void setHidden( bool hid, bool hidArea=false, bool hidLabel=false ) override;
        
        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w ) override;

    private:
        void setupMatrix( int rows, int cols );
        void createMatrix();
        void deleteMatrix();
        
        std::vector<std::vector<LedSmd*>> m_led;
        std::vector<Pin*> m_rowPin;
        std::vector<Pin*> m_colPin;
        
        bool m_verticalPins;

        double m_resistance;
        double m_maxCurrent;
        double m_threshold;
        
        int m_rows;
        int m_cols;
};

#endif
