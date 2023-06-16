/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef BATTERY_H
#define BATTERY_H

#include "comp2pin.h"
#include "e-resistor.h"

class LibraryItem;

class MAINMODULE_EXPORT Battery : public Comp2Pin, public eResistor
{
    public:
        Battery( QObject* parent, QString type, QString id );
        ~Battery();

 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem* libraryItem();

        double volt() { return m_volt; }
        void setVolt( double volt );

        virtual void setRes( double resist ) override;

        virtual void stamp() override;
        virtual void updateStep() override;

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget ) override;

    private:
        double m_volt;
};

#endif
