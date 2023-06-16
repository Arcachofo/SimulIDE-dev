/***************************************************************************
 *   Copyright (C) 2017 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef SERVO_H
#define SERVO_H

#include "logiccomponent.h"

class LibraryItem;

class MAINMODULE_EXPORT Servo : public LogicComponent
{
    public:
        Servo( QObject* parent, QString type, QString id );
        ~Servo();
        
 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem* libraryItem();

        double speed() { return m_speed; }
        void setSpeed( double speed ) { m_speed = speed; }

        double minPulse() { return m_minPulse; }
        void setMinPulse( double w );

        double maxPulse() { return m_maxPulse; }
        void setMaxPulse( double w );

        virtual void stamp() override;
        virtual void updateStep() override;
        virtual void voltChanged() override;

        virtual QPainterPath shape() const override;
        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget ) override;

    private:
        double m_pos;            // Actual Angular position 0-180
        double m_targetPos;      // Target Angular position 0-180
        double m_speed;               // Angular speed sec/60º
        double m_minPulse;        // Minimum pulse width,   0º
        double m_maxPulse;        // Maximum pulse width, 180º
        double m_minAngle;      // Angle to move evrey repaint

        uint64_t m_pulseStart;              // Simulation step
        uint64_t m_lastUpdate;              // Simulation step
};

#endif
