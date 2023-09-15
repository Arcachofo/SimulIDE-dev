/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef DIODE_H
#define DIODE_H

#include "e-diode.h"
#include "comp2pin.h"
#include "linkable.h"

class LibraryItem;

class MAINMODULE_EXPORT Diode : public Comp2Pin, public eDiode, public Linkable
{
    public:
        Diode( QString type, QString id, bool zener=false );
        ~Diode();

 static Component* construct( QString type, QString id );
 static LibraryItem *libraryItem();

        virtual void voltChanged() override;

        virtual bool setPropStr( QString prop, QString val ) override;

        virtual void initialize() override;
        virtual void stamp() override;
        virtual void updateStep() override;

        virtual double res() override{ return m_resistor->res(); }
        virtual void setResSafe( double resist ) override { m_resistor->setResSafe(resist);}



        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget ) override;

    private:
        bool m_isZener;

        eNode* m_midEnode;
        eResistor* m_resistor;
};

#endif
