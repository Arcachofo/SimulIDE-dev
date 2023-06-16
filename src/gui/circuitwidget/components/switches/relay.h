/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef RELAY_H
#define RELAY_H

#include "mech_contact.h"
#include "inductor.h"
//#include "e-resistor.h"

class LibraryItem;
class eNode;

class MAINMODULE_EXPORT Relay : public MechContact
{
    public:
        Relay( QObject* parent, QString type, QString id );
        ~Relay();

 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem* libraryItem();

        double iTrig() { return m_trigCurrent; }
        void setITrig( double c ) { if( c > 0.0 ) m_trigCurrent = c; }

        double iRel() { return m_relCurrent; }
        void setIRel( double current ) { m_relCurrent = current; }

        virtual void initialize() override;
        virtual void stamp() override;
        virtual void voltChanged() override;

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget ) override;

    protected:
        Inductor* m_inductor;

        eNode* m_internalEnode;

        double m_trigCurrent;
        double m_relCurrent;

        bool m_relayOn;
};

#endif
