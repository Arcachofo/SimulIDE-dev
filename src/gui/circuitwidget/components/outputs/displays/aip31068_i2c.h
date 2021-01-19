/***************************************************************************
 *   Copyright (C) 2016 by santiago González                               *
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

#ifndef AIP31068_I2C_H
#define AIP31068_I2C_H

#include "itemlibrary.h"
#include "hd44780_base.h"
#include "e-i2c.h"
#include "pin.h"

class MAINMODULE_EXPORT Aip31068_i2c : public Hd44780_Base, public eI2C
{
    Q_OBJECT
    Q_PROPERTY( int Control_Code READ cCode   WRITE setCcode  DESIGNABLE true USER true )

    public:
        Aip31068_i2c( QObject* parent, QString type, QString id );
        ~Aip31068_i2c();
        
        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();
        
        virtual QList<propGroup_t> propGroups() override;

        int cCode();
        void setCcode( int code );

        virtual void initialize() override;
        virtual void updateStep() override;
        virtual void voltChanged() override;
        virtual void remove() override;
        virtual void startWrite() override;
        virtual void readByte() override;

        void showPins( bool show );

    private:
        int m_cCode;
        int m_controlByte;
        int m_phase;

        //Inputs
        Pin* m_pinSDA;
        Pin* m_pinSCL;
};

#endif

