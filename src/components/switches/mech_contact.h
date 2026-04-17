/***************************************************************************
 *   Copyright (C) 2018 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#pragma once

#include "component.h"
#include "e-resistor.h"

class MechContact : public Component, public eElement
{
    public:
        MechContact( QString type, QString id );
        ~MechContact();

        virtual int poles() { return m_numPoles; }
        virtual void setPoles( int poles );

        virtual bool dt() { return (m_numthrows>1); }
        virtual void setDt( bool dt );
        
        virtual bool nClose() { return m_nClose; }
        virtual void setNClose( bool nc );

        void stamp() override;
        void remove() override;
        void updateStep() override { if( m_changed ) { m_changed = false; update(); } }

        virtual void  SetupSwitches( int poles, int throws );
        void  SetupButton();

    protected:
        void paint( QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w ) override;

        virtual void setSwitch( bool on );

        std::vector<eResistor*> m_switches;

        bool m_closed;
        bool m_nClose;
        bool m_ButHidden;

        int m_numPoles;
        int m_numthrows;

        int m_pin0;
};
