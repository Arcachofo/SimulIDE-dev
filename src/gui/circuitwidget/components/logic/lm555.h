/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef LM555_H
#define LM555_H

#include "e-element.h"
#include "component.h"
#include "e-resistor.h"
#include "e-pin.h"

class LibraryItem;
class IoPin;
class eNode;

class MAINMODULE_EXPORT Lm555 : public Component, public eElement
{
    public:

        Lm555( QObject* parent, QString type, QString id );
        ~Lm555();
        
        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem *libraryItem();

        virtual void initialize() override;
        virtual void stamp() override;
        virtual void voltChanged() override;
        virtual void runEvent() override;
        
        virtual void paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget );

    protected:
        IoPin* m_output;
        Pin* m_discharge;

        Pin* m_Gnd;
        Pin* m_Vcc;
        Pin* m_cv;
        Pin* m_trigger;
        Pin* m_threshold;
        Pin* m_Reset;

        ePin m_ePinA;
        ePin m_ePinB;
        ePin m_ePinC;
        ePin m_ePinD;

        eResistor m_resA;
        eResistor m_resB;
        eResistor m_resD;

        eNode* m_thrEnode;

        double m_voltPos;
        double m_voltNeg;

        uint64_t m_propDelay;

        bool m_outState;
};

#endif
