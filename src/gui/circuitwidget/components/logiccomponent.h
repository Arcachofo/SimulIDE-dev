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
    public:
        LogicComponent( QObject* parent, QString type, QString id );
        ~LogicComponent();

        virtual void initialize() override;
        virtual void updateStep() override;
        virtual void stamp() override;

        void createOePin ( QString d, QString id ) { setOePin( createPin( d, id ) ); }
        void setOePin( IoPin* pin );
        void enableOutputs( bool en );
        void updateOutEnabled();
        bool outputEnabled();

        virtual void setInputHighV( double volt ) override;
        virtual void setInputLowV( double volt ) override;
        virtual void setInputImp( double imp ) override;

        bool tristate() { return m_tristate; }
        virtual void setTristate( bool t );

        QString triggerStr() { return m_triggers.at( (int)m_trigger ); }
        void setTriggerStr( QString t );

        virtual void remove() override;

        virtual QStringList getEnums( QString ) override { return m_triggers; }

    protected:
        bool m_outEnable;
        bool m_tristate;

        IoPin*  m_oePin;
 static QStringList m_triggers;
};

#endif
