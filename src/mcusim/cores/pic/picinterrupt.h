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

#ifndef PicINTERRUPT_H
#define PicINTERRUPT_H

#include "mcuinterrupts.h"
#include "mcutypes.h"

class MAINMODULE_EXPORT PicInterrupt : public Interrupt
{
    public:
        PicInterrupt( QString name, uint16_t vector, eMcu* mcu );
        ~PicInterrupt();

 static Interrupt* getInterrupt( QString name, uint16_t vector, eMcu* mcu );

        virtual void execute() override;
        virtual void exitInt() override;

    private:
        regBits_t m_GIE;
};

#endif
