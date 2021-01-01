/***************************************************************************
 *   Copyright (C) 2018 by santiago González                               *
 *   santigoro@gmail.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.  *
 *                                                                         *
 ***************************************************************************/

#ifndef WAVEGEN_H
#define WAVEGEN_H

#include "clock-base.h"
#include "itemlibrary.h"
#include <QObject>

class MAINMODULE_EXPORT WaveGen : public ClockBase
{
    Q_OBJECT
    Q_PROPERTY( double    Volt_Base READ voltBase WRITE setVoltBase DESIGNABLE true USER true )
    Q_PROPERTY( double    Duty      READ duty     WRITE setDuty     DESIGNABLE true USER true )
    Q_PROPERTY( int       Steps     READ steps    WRITE setSteps    DESIGNABLE true USER true )
    Q_PROPERTY( wave_type Wave_Type READ waveType WRITE setWaveType DESIGNABLE true USER true )
    Q_ENUMS( wave_type )
    
    public:

        WaveGen( QObject* parent, QString type, QString id );
        ~WaveGen();
        
        enum wave_type {
            Sine = 0,
            Saw,
            Triangle,
            Square,
            Random
        };

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem *libraryItem();
        
        double voltBase()            { return m_voltBase; }
        void setVoltBase( double v ) { m_voltBase = v; }
        
        double duty();
        void setDuty( double duty );
        
        int steps();
        void setSteps( int steps );
        
        wave_type waveType()              { return m_type; }
        void setWaveType( wave_type typ ) { m_type = typ; }
        
        virtual void updateStep() override;
        virtual void runEvent() override;

        virtual void paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget );
        
    private slots:
        void updateValues();
        
    private:
        void genSine();
        void genSaw();
        void genTriangle();
        void genSquare();
        void genRandom();
        
        wave_type m_type;
        double m_duty;
        double m_vOut;
        double m_voltBase;
        double m_lastVout;
        double m_halfW;
        double m_time;
        
        int      m_steps;
        uint64_t m_qSteps;
        uint64_t m_nextStep;
};

#endif
