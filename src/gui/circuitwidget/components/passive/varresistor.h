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

#ifndef VARRESISTOR_H
#define VARRESISTOR_H

#include "itemlibrary.h"
#include "varresbase.h"


class MAINMODULE_EXPORT VarResistor : public VarResBase
{
    Q_OBJECT
    Q_PROPERTY( double Max_Resistance READ maxRes WRITE setMax DESIGNABLE true USER true )
    Q_PROPERTY( double Min_Resistance READ minRes WRITE setMin DESIGNABLE true USER true )

    public:
        VarResistor( QObject* parent, QString type, QString id );
        ~VarResistor();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();

        void setMax( double r );
        void setMin( double r );

        virtual void updateStep() override;

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget *widget );

};

#endif
