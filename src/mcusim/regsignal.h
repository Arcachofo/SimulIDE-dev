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

#ifndef REGSIGNAL_H
#define REGSIGNAL_H

#include <functional>
#include <map>

template <typename... Args>
class RegSignal
{
    public:
        RegSignal()  = default;
        ~RegSignal() = default;

        RegSignal( RegSignal const& /*unused*/ ) {}

        RegSignal& operator=( RegSignal&& other ) noexcept
        {
            if( this != &other )
            {
                m_slots = std::move( other.m_slots );
                m_curId = other.m_curId;
            }
            return *this;
        }

        template <typename T>
        void connect( T* inst, void (T::*func)(uint8_t), uint8_t mask=0xFF )
        {
            m_slots.emplace( m_curId, [=](uint8_t args ) { (inst->*func)(args); } );
            m_masks.emplace( m_curId, mask );
            m_curId++;
        }

        void emitValue( uint8_t p ) // Calls all connected functions.
        {
            for( auto const& it : m_slots )
            {
                int id = it.first;
                uint8_t mask = m_masks.at( id );
                uint8_t val = p & mask;
                it.second( val );
            }
        }

    private:
        std::map<int, std::function<void(uint8_t)>> m_slots;

        std::map<int, uint8_t> m_masks;  // Mask which bits should be sent

        int m_curId{0};
};

#endif
