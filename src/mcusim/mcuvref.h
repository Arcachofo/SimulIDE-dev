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

#ifndef MCUVREF_H
#define MCUVREF_H

#include <QList>

#include "mcumodule.h"
#include "e-element.h"

class McuPin;

class MAINMODULE_EXPORT McuVref : public McuModule, public eElement
{
        friend class McuCreator;

    public:
        McuVref( eMcu* mcu, QString name );
        ~McuVref();

        virtual void initialize() override;

        double getVref() { return m_vref; }

        void callBack( McuModule* mod, bool call );

    protected:
        //virtual void setMode( uint8_t mode );

        bool m_enabled;
        uint8_t m_mode;

        double m_vref;

        McuPin* m_pinOut;

        QList<McuModule*> m_callBacks;
};

#endif
