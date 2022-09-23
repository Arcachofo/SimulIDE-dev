/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef MCUPRESCALED_H
#define MCUPRESCALED_H

#include "mcumodule.h"

class MAINMODULE_EXPORT McuPrescaled : public McuModule
{
        friend class McuCreator;

    public:
        McuPrescaled( eMcu* mcu, QString name );
        virtual ~McuPrescaled();

    protected:
        regBits_t m_prSelBits;              // Bits configuring prescaler index
        uint8_t  m_prIndex;                 // Prescaler index
        uint16_t m_prescaler;               // Actual Prescaler value
        std::vector<uint16_t> m_prescList;  // Prescaler values
};

#endif
