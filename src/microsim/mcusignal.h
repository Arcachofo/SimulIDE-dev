/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

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

        CallBackBase* nextCallBack;

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
            nextCallBack = NULL;
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
        McuSignal()
        {
            m_slot = NULL;
        }
        ~McuSignal()
        {
            CallBackBase* slot = m_slot;
            while( slot ) // delete slots
            {
                CallBackBase* slotDel = slot;
                slot = slot->nextCallBack;
                delete slotDel;
        }   }

        template <class Obj>
        void connect( Obj* obj, void (Obj::*func)(uint8_t), uint8_t mask=0xFF )
        {
            CallBack<Obj>* slot = new CallBack<Obj>( obj, func );
            slot->m_mask = mask;

            // New slots are prepended (LIFO)
            // This means Interrupt flag clearing after register write callback
            // Because Interrupts are created first
            slot->nextCallBack = m_slot;
            m_slot = slot;
        }

        template <class Obj>
        void disconnect( Obj* obj, void (Obj::*func)(uint8_t) )
        {
            CallBackBase* preSlot = NULL;
            CallBackBase* posSlot = m_slot;
            while( posSlot )
            {
                CallBack<Obj>* cb = dynamic_cast<CallBack<Obj>*>(posSlot);

                if( (cb->m_object == obj) && (cb->m_func == func))
                {
                    if( preSlot ) preSlot->nextCallBack = posSlot->nextCallBack;
                    else          m_slot = posSlot->nextCallBack;
                    delete posSlot;
                    break;
                }
                preSlot = posSlot;
                posSlot= posSlot->nextCallBack;
        }   }

        void emitValue( uint8_t val ) // Calls all connected functions with masked val.
        {
            CallBackBase* slot = m_slot;
            while( slot )
            {
                slot->call( val & slot->m_mask );
                slot = slot->nextCallBack;
        }   }

    private:
        CallBackBase* m_slot;
};

#endif
