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

#include "logiccomponent.h"

class LibraryItem;
class IoPin;
class Pin;

class MAINMODULE_EXPORT Max72xx_matrix : public LogicComponent
{
        Q_OBJECT
    public:
        Max72xx_matrix( QObject* parent, QString type, QString id );
        ~Max72xx_matrix();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();

        QString colorStr() { return m_enumUids.at((int)m_ledColor ); }
        void setColorStr( QString color );

        int numDisplays() { return m_numDisplays; }
        void setNumDisplays( int dispNumber );

        virtual void stamp() override;
        virtual void initialize() override;
        virtual void voltChanged() override;
        virtual void updateStep() override;

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

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

        int m_ledColor;
        QColor m_colors[6];

        //Inputs
        Pin* m_pinCS;
        Pin* m_pinDin;
        IoPin* m_pinSck;
};

#endif
