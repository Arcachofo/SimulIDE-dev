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

#ifndef MCUCOMPONENTPIN_H
#define MCUCOMPONENTPIN_H

#include "mcucomponent.h"
#include "iopin.h"

class BaseProcessor;

class MAINMODULE_EXPORT McuComponentPin : public IoPin
{
    public:
        McuComponentPin( McuComponent *mcu, QString id, QString type, QString label, int pos, int xpos, int ypos, int angle );
        ~McuComponentPin();

        virtual void stamp() override;
        virtual void initialize() override;
        virtual void setState( bool state );

        void setDirection( bool out );
        void setPullup(bool up );
        void setExtraSource( double vddAdmit, double gndAdmit );

        void enableIO( bool en );
        
        QString ptype() { return m_type; }
        QString id() { return m_id; }

        bool m_enableIO;

    protected:
        McuComponent*  m_mcuComponent;
        BaseProcessor* m_processor;

        bool m_attached;
        bool m_openColl;

        char m_port;
        int  m_pinN;
        int  m_pos;

        int m_pinType;

        QString m_type;
};

#endif

