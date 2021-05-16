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

#ifndef LOGICCOMPONENT_H
#define LOGICCOMPONENT_H

#include "iocomponent.h"
#include "e-clocked_device.h"

class IoPin;

class MAINMODULE_EXPORT LogicComponent : public IoComponent, public eClockedDevice
{
    Q_OBJECT

    public:
        LogicComponent( QObject* parent, QString type, QString id );
        ~LogicComponent();

        enum Trigger {
            None = 0,
            Clock,
            InEnable
        };
        Q_ENUM( Trigger )

        void initState();
        void stamp( eElement* el );

        void setOePin( IoPin* pin );
        void setOutputEnabled( bool enabled );
        void updateOutEnabled();
        bool outputEnabled();


        Trigger trigger() { return m_trigger; }
        virtual void setTrigger( Trigger trigger );

        virtual void remove() override;

        virtual void paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget );

    protected:
        bool m_outEnable;

        IoPin*  m_oePin;

        Trigger m_trigger;
};

#endif
