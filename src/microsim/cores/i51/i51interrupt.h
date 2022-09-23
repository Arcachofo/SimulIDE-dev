/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef I51INTERRUPT_H
#define I51INTERRUPT_H

#include "mcuinterrupts.h"

class MAINMODULE_EXPORT I51Interrupt : public Interrupt
{
    public:
        I51Interrupt( QString name, uint16_t vector, eMcu* mcu );
        ~I51Interrupt();

 static Interrupt* getInterrupt( QString name, uint16_t vector, eMcu* mcu );

    private:
};
//_________________________________________________________________
//_________________________________________________________________

/*class MAINMODULE_EXPORT I51ExtInt : public Interrupt
{
    public:
        I51ExtInt( QString name, uint16_t vector, eMcu* mcu );
        ~I51ExtInt();

        virtual void reset() override;
        virtual void raise( uint8_t v ) override;
        virtual void exitInt() override;

    private:
        uint8_t m_lastValue;
};*/
//_________________________________________________________________
//_________________________________________________________________

class I51Usart;

class MAINMODULE_EXPORT I51T1Int : public Interrupt
{
    public:
        I51T1Int( QString name, uint16_t vector, eMcu* mcu );
        ~I51T1Int();

        virtual void reset() override;
        virtual void raise( uint8_t v ) override;

        void setUsart( I51Usart* usart ) { m_usart = usart; }

    private:
        I51Usart* m_usart;
};

#endif
