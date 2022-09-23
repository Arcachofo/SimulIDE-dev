/***************************************************************************
 *   Copyright (C) 2018 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef VOLTREG_H
#define VOLTREG_H

#include "component.h"
#include "e-resistor.h"

class LibraryItem;

class MAINMODULE_EXPORT VoltReg : public Component, public eResistor
{
        Q_OBJECT
    public:
        VoltReg( QObject* parent, QString type, QString id );
        ~VoltReg();
        
        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();

        virtual void stamp() override;
        virtual void voltChanged() override;

        double outVolt() { return m_vRef; }
        void setOutVolt( double v ) { m_vRef = v; }

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    protected:
        double m_accuracy;
        double m_vRef;
        double m_voltPos;
        double m_voltNeg;
        double m_lastOut;
};

#endif
