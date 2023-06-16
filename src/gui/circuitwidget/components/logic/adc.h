/***************************************************************************
 *   Copyright (C) 2017 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef ADC_H
#define ADC_H

#include "logiccomponent.h"

class LibraryItem;

class MAINMODULE_EXPORT ADC : public LogicComponent
{
    public:
        ADC( QObject* parent, QString type, QString id );
        ~ADC();

 static Component*   construct( QObject* parent, QString type, QString id );
 static LibraryItem* libraryItem();

        virtual void stamp() override;
        virtual void voltChanged() override;
        virtual void runEvent() override { IoComponent::runOutputs(); }

        double maxVolt() { return m_maxVolt; }
        void setMaxVolt( double v ) { m_maxVolt = v; }

        void setNumOutputs( int pins );
        
    protected:
        double m_maxVolt;
        double m_maxValue;
};

#endif
