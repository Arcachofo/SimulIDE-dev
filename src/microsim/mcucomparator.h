/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef MCUCOMPARATOR_H
#define MCUCOMPARATOR_H

#include "mcumodule.h"
#include "e-element.h"

class McuPin;

class MAINMODULE_EXPORT McuComp : public McuModule, public eElement
{
        friend class McuCreator;

    public:
        McuComp( eMcu* mcu, QString name );
        ~McuComp();

        virtual void initialize() override;

        virtual void setPinN( McuPin* ){;}

        virtual void callBackDoub( double vref ) override;

    protected:
        virtual void setMode( uint8_t mode ) { m_mode = mode; }

        bool m_fixVref;
        bool m_enabled;
        bool m_compOut;

        double m_vref;

        uint8_t m_mode;

        std::vector<McuPin*> m_pins;
        McuPin* m_pinP;
        McuPin* m_pinN;
        McuPin* m_pinOut;
};

#endif
