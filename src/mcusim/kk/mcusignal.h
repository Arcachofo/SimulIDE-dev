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

#ifndef MCUSIGNAL_H
#define MCUSIGNAL_H

#include <functional>
#include <map>

template <typename... Args>
class McuSignal
{
    public:
        McuSignal()  = default;
        ~McuSignal() = default;

        McuSignal( McuSignal const& /*unused*/ ) {}

        McuSignal& operator=( McuSignal&& other ) noexcept
        {
            if (this != &other) {
                m_slots      = std::move( other.m_slots );
                m_current_id = other.m_current_id;
            }
            return *this;
        }

        void connectFunc( std::function<void(Args...)> const& slot )
        {
            m_slots.insert( std::make_pair( ++m_current_id, slot ));
        }

        template <typename T>
        void connect( T* inst, void (T::*func)(Args...) )
        {
            connectFunc( [=](Args... args ) { (inst->*func)(args...); });
        }

        void emitValue(Args... p) // Calls all connected functions.
        {
            for( auto const& it : m_slots ) { it.second( p... ); }
        }

    private:
        std::map<int, std::function<void(Args...)>> m_slots;

        int m_current_id{0};
};

#endif
