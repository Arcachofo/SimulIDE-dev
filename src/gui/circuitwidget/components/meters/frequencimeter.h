/***************************************************************************
 *   Copyright (C) 2019 by santiago González                               *
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

#ifndef FREQUENCIMETER_H
#define FREQUENCIMETER_H

#include "itemlibrary.h"
#include "component.h"
#include "e-element.h"

class MAINMODULE_EXPORT Frequencimeter : public Component, public eElement
{
    Q_OBJECT
    Q_PROPERTY( double Filter   READ filter  WRITE setFilter DESIGNABLE true USER true )

    public:

        Frequencimeter( QObject* parent, QString type, QString id );
        ~Frequencimeter();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem *libraryItem();

        double filter() { return m_filter; }
        void setFilter( double filter ){ m_filter = filter; }

        virtual void initialize() override;
        virtual void stamp() override;
        virtual void updateStep() override;
        virtual void voltChanged() override;
        
        virtual void paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget );
        
    private:
        bool m_rising;
        bool m_falling;
        
        double m_filter;
        double m_lastData;
        double m_freq;
        
        int m_numMax;

        uint64_t m_lastMax;
        uint64_t m_totalP;
        
        QGraphicsSimpleTextItem m_display;
};

#endif
