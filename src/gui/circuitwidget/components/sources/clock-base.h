/***************************************************************************
 *   Copyright (C) 2010 by santiago González                               *
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

#ifndef CLOCKBASE_H
#define CLOCKBASE_H

#include "logicinput.h"

class MAINMODULE_EXPORT ClockBase : public LogicInput
{
    Q_OBJECT
    Q_PROPERTY( bool Always_On READ alwaysOn WRITE setAlwaysOn DESIGNABLE true USER true )
    Q_PROPERTY( double    Freq READ freq     WRITE setFreq     DESIGNABLE true USER true )
    //Q_PROPERTY( quint64  Tr_ps READ riseTime WRITE setRiseTime DESIGNABLE true USER true )
    //Q_PROPERTY( quint64  Tf_ps READ fallTime WRITE setFallTime DESIGNABLE true USER true )
    Q_PROPERTY( bool   Running READ running  WRITE setRunning )

    public:
        ClockBase( QObject* parent, QString type, QString id );
        ~ClockBase();

        bool alwaysOn() { return m_alwaysOn; }
        void setAlwaysOn( bool on );

        double freq() { return m_freq; }
        virtual void setFreq( double freq );
        
        bool running();
        virtual void setRunning( bool running );
        virtual void setOut( bool ){;}

        virtual void stamp() override;
        virtual void updateStep() override;
        
    signals:
        void freqChanged();

    public slots:
        virtual void onbuttonclicked();

    protected:
        bool m_isRunning;
        bool m_alwaysOn;

        uint64_t m_stepsPC;
        double m_fstepsPC;

        double m_freq;
        double m_remainder;
};

#endif
