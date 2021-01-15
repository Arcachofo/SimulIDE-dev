/***************************************************************************
 *   Copyright (C) 2012 by santiago González                               *
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

#ifndef ESOURCE_H
#define ESOURCE_H


#include "e-element.h"
#include "e-node.h"

class MAINMODULE_EXPORT eSource : public eElement
{
    public:
        eSource( QString id, ePin* epin );
        virtual ~eSource();

        virtual void initialize() override;
        virtual void runEvent() override;
        
        void stamp() override;
        void stampOutput();

        double voltHight() { return m_voltHigh; }
        void  setVoltHigh( double v );

        double voltLow() { return m_voltLow; }
        void  setVoltLow( double v );
        
        bool out() { return m_out; }
        void setOut( bool out );
        void setTimedOut( bool out );

        bool  isInverted() { return m_inverted; }
        void  setInverted( bool inverted );

        double imp() { return m_imp; }
        virtual void  setImp( double imp );

        uint64_t riseTime() { return m_timeLH; }
        void setRiseTime( uint64_t time );

        uint64_t fallTime() { return m_timeHL; }
        void setFallTime( uint64_t time );

        double getVolt();

    protected:
        double m_voltHigh;
        double m_voltLow;
        double m_voltOut;
        double m_imp;
        double m_admit;

        bool m_out;
        bool m_nextOut;
        bool m_inverted;

        double m_timeLH;
        double m_timeHL;

        eNode* m_scrEnode;
};
#endif

