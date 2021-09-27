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

#ifndef LDR_H
#define LDR_H

#include "varresbase.h"

class LibraryItem;

class MAINMODULE_EXPORT Ldr : public VarResBase
{
    public:
        Ldr( QObject* parent, QString type, QString id );
        ~Ldr();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();

        int r1() { return m_r1; }
        void setR1( int r1 );

        double gamma() { return m_gamma; }
        void setGamma( double ga );

        virtual void updateStep() override;

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    private:
        int m_r1;

        double m_gamma;
};

#endif
