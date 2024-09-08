/***************************************************************************
 *   Copyright (C) 2016 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef STEPPER_H
#define STEPPER_H

#include "linkercomponent.h"
#include "e-element.h"
#include "e-resistor.h"
#include "pin.h"

class LibraryItem;

class Stepper : public LinkerComponent, public eElement
{
    public:
        Stepper( QString type, QString id );
        ~Stepper();

 static Component* construct( QString type, QString id );
 static LibraryItem* libraryItem();

        virtual void stamp() override;
        virtual void voltChanged() override;
        virtual void updateStep() override;

        int steps() { return m_steps; }
        void setSteps( int steps ); //" 4, 8,16,32"
        
        double resistance()  { return m_resistance; }
        void setResistance( double rows );

        bool bipolar() { return m_bipolar; }
        void setBipolar( bool bi );

        virtual void setHidden( bool hid, bool hidArea=false, bool hidLabel=false ) override;

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w ) override;

    protected:
        int m_steps;
        double m_resistance;
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

