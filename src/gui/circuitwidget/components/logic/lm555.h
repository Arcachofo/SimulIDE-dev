/***************************************************************************
 *                                                                         *
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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef LM555_H
#define LM555_H

#include "e-element.h"
#include "component.h"

class LibraryItem;
class IoPin;

class MAINMODULE_EXPORT Lm555 : public Component, public eElement
{
    Q_OBJECT
    /// Q_PROPERTY( quint64 Tpd_ps  READ propDelay   WRITE setPropDelay   DESIGNABLE true USER true )

    public:

        Lm555( QObject* parent, QString type, QString id );
        ~Lm555();
        
        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem *libraryItem();

        virtual void stamp() override;
        virtual void initialize() override;
        virtual void voltChanged() override;
        virtual void runEvent() override;
        
        virtual void paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget );

    protected:

        IoPin* m_output;
        IoPin* m_cv;
        IoPin* m_dis;

        double m_volt;
        double m_voltLast;
        double m_voltNeg;
        double m_voltNegLast;
        double m_voltHight;
        double m_voltHightLast;
        double m_disImp;
        double m_disImpLast;

        uint64_t m_propDelay;

        bool m_outState;
};

#endif
