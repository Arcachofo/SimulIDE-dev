/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef MCUOCM_H
#define MCUOCM_H

#include "mcuprescaled.h"
#include "e-element.h"

class eMcu;
class McuPin;
class McuOcUnit;

class MAINMODULE_EXPORT McuOcm : public McuPrescaled, public eElement
{
        friend class McuCreator;

    public:
        McuOcm( eMcu* mcu, QString name );
        ~McuOcm();

        virtual void initialize() override;

        void setOcActive( McuOcUnit* oc, bool a );
        void setState( McuOcUnit* oc, bool s );

    protected:
        virtual void OutputOcm()=0;

        bool m_state1;
        bool m_state2;

        bool m_oc1Active;
        bool m_oc2Active;

        bool m_mode;

        McuPin* m_oPin;

        McuOcUnit* m_OC1;
        McuOcUnit* m_OC2;
};

#endif
