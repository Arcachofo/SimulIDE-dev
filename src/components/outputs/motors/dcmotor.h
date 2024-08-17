/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef DCMOTOR_H
#define DCMOTOR_H

#include "linkercomponent.h"
#include "e-resistor.h"


class LibraryItem;

class DcMotor : public LinkerComponent, public eResistor
{
    public:
        DcMotor( QString type, QString id );
        ~DcMotor();

        virtual QString getPropStr( QString prop ) override;

 static Component* construct( QString type, QString id );
 static LibraryItem* libraryItem();

        virtual void initialize() override;
        virtual void stamp() override;
        virtual void voltChanged() override;
        virtual void updateStep() override;

        int rpm() { return m_rpm; }
        void setRpm( int rpm );

        double volt() { return m_voltNom; }
        void setVolt( double v ) { m_voltNom = v; }

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w ) override;

    protected:
        void updatePos();

        int m_rpm;

        double m_LastVolt;
        double m_voltNom;
        double m_ang;
        double m_motStPs;
        double m_speed;
        double m_delta;

        uint64_t m_lastTime;
        uint64_t m_updtTime;
};

#endif
