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

#ifndef MCUOCM_H
#define MCUOCM_H

#include "mcuprescaled.h"
#include "e-element.h"

class eMcu;
class McuPin;
class McuOcUnit;

class MAINMODULE_EXPORT McuOcm : public McuPrescaled, public eElement
{
        friend class McuCreator;

    public:
        McuOcm( eMcu* mcu, QString name );
        ~McuOcm();

        virtual void initialize() override;

        void setOcActive( McuOcUnit* oc, bool a );
        void setState( McuOcUnit* oc, bool s );

    protected:
        virtual void OutputOcm()=0;

        bool m_state1;
        bool m_state2;

        bool m_oc1Active;
        bool m_oc2Active;

        bool m_mode;

        McuPin* m_oPin;

        McuOcUnit* m_OC1;
        McuOcUnit* m_OC2;
};

#endif
