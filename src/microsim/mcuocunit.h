/***************************************************************************
 *   Copyright (C) 2020 by santiago González                               *
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

#ifndef MCUOCUNIT_H
#define MCUOCUNIT_H

#include "mcumodule.h"
#include "e-element.h"

class McuPin;
class McuTimer;
class McuOcm;

enum ocAct_t{
    ocNON=0,
    ocTOG,
    ocCLR,
    ocSET,
};

class MAINMODULE_EXPORT McuOcUnit : public McuModule, public eElement
{
        friend class McuCreator;

    public:
        McuOcUnit( eMcu* mcu, QString name );
         ~McuOcUnit();

        virtual void initialize();
        virtual void runEvent();
        virtual void configure( uint8_t ){;}
        virtual void ocrWriteL( uint8_t val );
        virtual void ocrWriteH( uint8_t val );
        virtual void sheduleEvents( uint32_t ovf, uint32_t countVal, int rot=0 );
        virtual void tov() { drivePin( m_tovAct ); }

        virtual void setOcActs( ocAct_t comAct, ocAct_t tovAct );

        void setCtrlPin( bool c ) { m_ctrlPin = c; }

        void clockStep( uint16_t count );

        uint8_t getMode() { return m_mode; }
        McuPin* getPin() { return m_ocPin; }

    protected:
        virtual void drivePin( ocAct_t act );
        virtual void setPinSate( bool state );

        McuOcm*   m_ocm;
        McuTimer* m_timer;
        McuPin*   m_ocPin;

        uint8_t* m_ocRegL;
        uint8_t* m_ocRegH;

        ocAct_t  m_comAct;
        ocAct_t  m_tovAct;

        bool m_enabled;
        bool m_ctrlPin;
        uint8_t m_mode;

        uint16_t m_comMatch;  // counter vale to match a comparation
        uint16_t m_extMatch;  // counter vale to match a comparation

        bool m_pinSet;
};

#endif
