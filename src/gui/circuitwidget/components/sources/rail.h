/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef RAIL_H
#define RAIL_H

#include "component.h"
#include "e-element.h"

class LibraryItem;
class IoPin;

class MAINMODULE_EXPORT Rail : public Component, public eElement
{
    public:
        Rail( QObject* parent, QString type, QString id );
        ~Rail();

 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem* libraryItem();

        double volt() { return m_volt; }
        void setVolt( double v );

        virtual void stamp() override;

        virtual QPainterPath shape() const override;
        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget ) override;

    private:
        double m_volt;

        IoPin* m_out;
};

#endif
