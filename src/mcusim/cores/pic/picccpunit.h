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

#ifndef PICCCPUNIT_H
#define PICCCPUNIT_H

#include "mcumodule.h"
#include "e-element.h"

class PicOcUnit;
class PicIcUnit;
class PicPwmUnit;
class McuPin;

enum ccpMode_t{
    ccpOFF=0,
    ccpCAP,
    ccpCOM,
    ccpPWM,
};

class MAINMODULE_EXPORT PicCcpUnit : public McuModule, public eElement
{
    friend class McuCreator;

    public:
        PicCcpUnit( eMcu* mcu, QString name, int type );
        ~PicCcpUnit();

        virtual void initialize();

        virtual void ccprWriteL( uint8_t val );
        virtual void ccprWriteH( uint8_t val );

        virtual void configureA( uint8_t CCPxCON ) override;

        virtual void setInterrupt( Interrupt* i ) override;

        void setPin( McuPin* pin );

    protected:
        uint8_t m_mode;
        ccpMode_t m_ccpMode;

        uint8_t* m_ccpRegL;
        uint8_t* m_ccpRegH;

        regBits_t m_CCPxM;

        PicIcUnit*  m_capUnit;
        PicOcUnit*  m_comUnit;
        PicPwmUnit* m_pwmUnit;
};

#endif
