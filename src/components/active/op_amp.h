/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef OPAMP_H
#define OPAMP_H

#include "component.h"
#include "e-element.h"

class LibraryItem;
class IoPin;

class OpAmp : public Component, public eElement
{
    public:
        OpAmp( QString type, QString id );
        ~OpAmp();
        
        static Component* construct( QString type, QString id );
        static LibraryItem* libraryItem();

        virtual void initialize() override;
        virtual void stamp() override;
        virtual void updateStep() override;
        virtual void voltChanged() override;

        double gain() { return m_gain; }
        void setGain( double g ) { m_gain = g; m_changed = true; }

        double outImp() { return m_outImp; }
        void setOutImp( double imp );

        double voltPos() { return m_voltPosDef; }
        void setVoltPos( double v ) { m_voltPosDef = v; m_changed = true; }

        double voltNeg() { return m_voltNegDef; }
        void setVoltNeg( double v ) { m_voltNegDef = v; m_changed = true; }

        bool powerPins() {return m_powerPins; }
        void setPowerPins( bool set );

        bool switchPins() { return m_switchPins; }
        void setSwitchPins( bool s );

        virtual QPainterPath shape() const override;
        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w ) override;

    protected:
        virtual void slotProperties() override;
        void updtProperties();

        bool m_powerPins;
        bool m_switchPins;

        double m_accuracy;
        double m_gain;
        double m_k;
        double m_voltPos;
        double m_voltNeg;
        double m_voltPosDef;
        double m_voltNegDef;
        double m_lastOut;
        double m_lastIn;
        double m_outImp;

        IoPin* m_inputP;
        IoPin* m_inputN;
        IoPin* m_output;
};

#endif
