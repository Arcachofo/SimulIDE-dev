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

class LibraryItem;

class MAINMODULE_EXPORT WaveGen : public ClockBase
{
        Q_OBJECT
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
 static LibraryItem* libraryItem();

        virtual bool setPropStr( QString prop, QString val ) override;

        virtual void runEvent() override;

        double duty() { return m_duty; }
        void setDuty( double duty );

        int steps() { return m_steps; }
        void setSteps( int steps );

        QString waveType() { return m_enumUids.at((int)m_waveType); }
        void setWaveType( QString t );

        double semiAmpli() { return m_voltage/2; }
        void setSemiAmpli( double v ) { m_voltage = v*2; }

        double midVolt() { return m_voltBase+m_voltage/2; }
        void setMidVolt( double v ) { m_voltBase = v-m_voltage/2;}

        virtual void setFreq( double freq ) override;

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );
        
    private:
        void genSine();
        void genSaw();
        void genTriangle();
        void genSquare();
        void genRandom();
        
        wave_type m_waveType;
        double m_duty;
        double m_vOut;
        double m_voltBase;
        double m_lastVout;
        double m_halfW;
        double m_time;
        
        int      m_steps;
        uint64_t m_qSteps;
        uint64_t m_nextStep;

        QStringList m_waves;
};

#endif
