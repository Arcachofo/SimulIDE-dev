/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef MCUICUNIT_H
#define MCUICUNIT_H

#include "mcumodule.h"
#include "e-element.h"

class McuPin;
class McuTimer;

class MAINMODULE_EXPORT McuIcUnit : public McuModule, public eElement
{
        friend class McuCreator;

    public:
        McuIcUnit( eMcu* mcu, QString name );
        virtual ~McuIcUnit();

        virtual void initialize() override;
        virtual void voltChanged() override;

        virtual void configure( uint8_t val ) {;}

        void enable( bool en );

    protected:
        McuTimer* m_timer;
        McuPin*   m_icPin;

        uint8_t* m_icRegL;
        uint8_t* m_icRegH;

        bool m_enabled;
        bool m_inState;

        uint8_t m_mode;

        bool m_fallingEdge;
};

#endif
