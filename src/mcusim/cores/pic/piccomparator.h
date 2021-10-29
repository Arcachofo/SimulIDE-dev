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

#ifndef PICCOMPARATOR_H
#define PICCOMPARATOR_H

#include "mcucomparator.h"
//#include "mcutypes.h"

class MAINMODULE_EXPORT PicComp : public McuComp
{
    public:
        PicComp( eMcu* mcu, QString name );
        ~PicComp();

        //virtual void initialize() override;
        virtual void voltChanged() override;

        virtual void configureA( uint8_t newCMCON ) override;

 static PicComp* getComparator( eMcu* mcu, QString name );

    protected:
        //void compare( uint8_t );
        void connect( McuPin* pinN, McuPin* pinP=NULL, McuPin* pinOut=NULL );

        double getVref();

        bool m_cis;
        bool m_inv;

        regBits_t m_CM;
        regBits_t m_CIS;
        regBits_t m_CINV;
        regBits_t m_COUT;
};

class MAINMODULE_EXPORT PicComp01 : public PicComp
{
    public:
        PicComp01( eMcu* mcu, QString name );
        ~PicComp01();

    protected:
        virtual void setMode( uint8_t mode );
};

class MAINMODULE_EXPORT PicComp02 : public PicComp
{
    public:
        PicComp02( eMcu* mcu, QString name );
        ~PicComp02();

    protected:
        virtual void setMode( uint8_t mode );
};


#endif
