/***************************************************************************
 *   Copyright (C) 2021 by santiago González                               *
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

#ifndef MCUBASE_H
#define MCUBASE_H

#include "chip.h"

class McuInterface;

class MAINMODULE_EXPORT McuBase : public Chip
{
    public:
        McuBase(  QObject* parent, QString type, QString id  );
        ~McuBase();

 static McuBase* self() { return m_pSelf; }

        virtual bool load( QString fileName )=0;
        virtual double freq()=0;

        virtual void reset()=0;

        QString device() { return m_device; }

        McuInterface* proc() { return m_proc; }

    protected:
 static McuBase* m_pSelf;

        McuInterface* m_proc;
        QString m_device;       // Name of device

        bool m_crashed;
};

#endif
