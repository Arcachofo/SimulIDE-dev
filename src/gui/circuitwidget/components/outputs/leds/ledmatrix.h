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

class MAINMODULE_EXPORT LedMatrix : public Component, public eElement
{
    public:
        LedMatrix( QObject* parent, QString type, QString id );
        ~LedMatrix();

 static Component* construct( QObject* parent, QString type, QString id );
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

        double maxCurrent() { return m_maxCurr; }
        void   setMaxCurrent( double current );
        
        double res() { return m_resist; }
        void  setRes( double resist );

        virtual void stamp() override;
        virtual void remove() override;

        virtual void setHidden( bool hid, bool hidArea=false, bool hidLabel=false ) override;

        virtual QStringList getEnumUids( QString ) override;
        virtual QStringList getEnumNames( QString ) override;
        
        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget ) override;

    private:
        void setupMatrix( int rows, int cols );
        void createMatrix();
        void deleteMatrix();
        
        std::vector<std::vector<LedSmd*>> m_led;
        std::vector<Pin*> m_rowPin;
        std::vector<Pin*> m_colPin;
        
        bool m_verticalPins;

        double m_resist;
        double m_maxCurr;
        double m_threshold;
        
        int m_rows;
        int m_cols;
};

#endif
