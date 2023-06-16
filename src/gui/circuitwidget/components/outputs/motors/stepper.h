/***************************************************************************
 *   Copyright (C) 2016 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef STEPPER_H
#define STEPPER_H

#include "itemlibrary.h"
#include "component.h"
#include "e-element.h"
#include "e-resistor.h"
#include "pin.h"

class MAINMODULE_EXPORT Stepper : public Component, public eElement
{
    public:
        Stepper( QObject* parent, QString type, QString id );
        ~Stepper();

 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem* libraryItem();

        int steps() { return m_steps; }
        void setSteps( int steps ); //" 4, 8,16,32"
        
        double res()  { return m_res; }
        void setRes( double rows );

        bool bipolar() { return m_bipolar; }
        void setBipolar( bool bi );

        virtual void stamp() override;
        virtual void voltChanged() override;
        virtual void updateStep() override;
        virtual void remove() override;
        
        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget ) override;

    protected:
        int m_steps;
        double m_res;
        int m_ang;
        int m_stpang;
        int m_Ppos;

        bool m_bipolar;
        
        eResistor m_resA1;
        eResistor m_resA2;
        eResistor m_resB1;
        eResistor m_resB2;
        
        Pin m_pinA1;
        Pin m_pinA2;
        Pin m_pinCo;
        Pin m_pinB1;
        Pin m_pinB2;
        
        ePin m_ePinA1Co;
        ePin m_ePinA2Co;
        ePin m_ePinB1Co;
        ePin m_ePinB2Co;
};

#endif

