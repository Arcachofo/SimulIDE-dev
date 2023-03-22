/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef TRIAC_H
#define TRIAC_H

#include "component.h"
#include "e-element.h"

class LibraryItem;
class eDiode;
class eResistor;

class MAINMODULE_EXPORT Triac : public Component, public eElement
{
    public:
        Triac( QObject* parent, QString type, QString id );
        ~Triac();

 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem* libraryItem();

        virtual void initialize() override;
        virtual void stamp() override;
        virtual void updateStep() override;
        virtual void voltChanged() override;

        double gateRes() { return m_gateRes; }
        void setGateRes( double r ) { m_gateRes = r; m_changed = true; }

        double trigCurr() { return m_trigCurr; }
        void setTrigCurr( double c ) { m_trigCurr = c; m_changed = true; }

        double holdCurr() { return m_holdCurr; }
        void setHoldCurr( double v ) { m_holdCurr = v; m_changed = true; }

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    private:
        double m_gateRes;
        double m_trigCurr;
        double m_holdCurr;

        bool m_state;

        eNode*     m_midEnode;
        eResistor* m_resistor;
        eResistor* m_resistGa;

        eDiode* m_diode1;
        eDiode* m_diode2;
};

#endif
