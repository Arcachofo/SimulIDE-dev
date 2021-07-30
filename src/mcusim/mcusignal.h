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

#include <vector>
#include <inttypes.h>

class CallBackBase
{
        friend class McuSignal;

    public:
        CallBackBase(){;}
        virtual ~CallBackBase(){;}

        virtual void call( uint8_t ){;}

    private:
        uint8_t m_mask;
};

template <class Obj>
class CallBack : public CallBackBase
{
        friend class McuSignal;

    public:
        CallBack( Obj* object, void (Obj::*func)(uint8_t) )
        : CallBackBase()
        {
            m_object = object;
            m_func = func;
        }
        ~CallBack() {;}

        virtual void call( uint8_t val ) override
        { (m_object->*m_func)(val); }

    private:
        Obj* m_object;
        void (Obj::*m_func)(uint8_t);
};

class McuSignal
{
    public:
        McuSignal(){;}
        ~McuSignal()
        {
            for( CallBackBase* slot : m_slots ) { delete slot; }
        }

        template <class Obj>
        void connect( Obj* obj, void (Obj::*func)(uint8_t), uint8_t mask=0xFF )
        {
            CallBack<Obj>* slot = new CallBack<Obj>( obj, func );
            slot->m_mask = mask;
            m_slots.emplace_back( slot );
        }

        template <class Obj>
        void disconnect( Obj* obj, void (Obj::*func)(uint8_t) )
        {
            for( unsigned i=0; i<m_slots.size(); ++i )
            {
                CallBack<Obj>* cb = dynamic_cast<CallBack<Obj>*>(m_slots[i]);
                if( (cb->m_object == obj) && (cb->m_func == func))
                {
                    delete m_slots[i];
                    m_slots.erase( m_slots.begin()+i );
                    break;
        }   }   }

        void emitValue( uint8_t val ) // Calls all connected functions with masked val.
        {
            for( unsigned i=0; i<m_slots.size(); ++i ) { m_slots[i]->call( val & m_slots[i]->m_mask ); }
        }

    private:
        std::vector<CallBackBase*> m_slots;
};

#endif
