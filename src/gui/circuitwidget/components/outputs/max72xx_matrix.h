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

#ifndef MAX72XX_MATRIX_H
#define MAX72XX_MATRIX_H

#include "itemlibrary.h"
#include "component.h"
#include "e-logic_device.h"
#include "pin.h"

class MAINMODULE_EXPORT Max72xx_matrix : public Component, public eLogicDevice
{
    Q_OBJECT
    Q_PROPERTY( int  NumDisplays   READ numDisplays WRITE setNumDisplays DESIGNABLE true USER true )

    public:
        Max72xx_matrix( QObject* parent, QString type, QString id );
        ~Max72xx_matrix();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem *libraryItem();

        virtual QList<propGroup_t> propGroups() override;
        
        int numDisplays();
        void setNumDisplays( int dispNumber );

        virtual void stamp() override;
        virtual void initialize() override;
        virtual void voltChanged() override;
        virtual void updateStep() override;
        virtual void remove() override;
        
        virtual void paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget );

    private:
        void proccessCommand();

        int m_numDisplays;

        int  m_ram[11][8];
        int  m_intensity[11];
        int  m_decodemode;
        int  m_scanlimit;
        bool m_shutdown;
        bool m_test;

        int m_rxReg;        // Received value
        int m_inBit;        // How many bits have we read since last value
        int m_inDisplay;    // Current display

        //Inputs
        Pin* m_pinCS;
        Pin* m_pinDin;
        Pin* m_pinSck;
};

#endif
