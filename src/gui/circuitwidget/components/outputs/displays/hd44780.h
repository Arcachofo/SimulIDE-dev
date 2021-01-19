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

#ifndef HD44780_H
#define HD44780_H

#include "itemlibrary.h"
#include "hd44780_base.h"
#include "e-element.h"
#include "pin.h"

class MAINMODULE_EXPORT Hd44780 : public Hd44780_Base, public eElement
{
    Q_OBJECT

    public:
        Hd44780( QObject* parent, QString type, QString id );
        ~Hd44780();
        
        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();

        virtual QList<propGroup_t> propGroups() override;

        virtual void initialize() override;
        virtual void stamp() override;
        virtual void updateStep() override;
        virtual void voltChanged() override;
        virtual void remove() override;

        void showPins( bool show );

    private:
        Pin* m_pinRS;
        Pin* m_pinRW;
        Pin* m_pinEn;
        std::vector<Pin*> m_dataPin;
};

#endif

