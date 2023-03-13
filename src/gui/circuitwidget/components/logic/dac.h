/***************************************************************************
 *   Copyright (C) 2017 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef DAC_H
#define DAC_H

#include "logiccomponent.h"

class LibraryItem;

class MAINMODULE_EXPORT DAC : public LogicComponent
{
    public:
        DAC( QObject* parent, QString type, QString id );
        ~DAC();

 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem *libraryItem();

        virtual void stamp() override;
        virtual void voltChanged() override;
        virtual void runEvent() override;

        double maxVolt() { return m_maxVolt; }
        void setMaxVolt( double v ) { m_maxVolt = v; }

        void setNumInps( int pins );
        
    protected:
        double m_maxVolt;
        double m_maxValue;

        int m_val;
};

#endif
