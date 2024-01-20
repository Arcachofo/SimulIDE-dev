/***************************************************************************
 *   Copyright (C) 2016 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef EMOSFET_H
#define EMOSFET_H

#include "e-resistor.h"

class eMosfet : public eResistor
{
    public:
        eMosfet( QString id );
        ~eMosfet();

        virtual void stamp() override;
        virtual void voltChanged() override;

        bool pChannel() { return m_Pchannel; }
        virtual void setPchannel( bool p ) { m_Pchannel = p; m_changed = true; }

        bool depletion() { return m_depletion; }
        virtual void setDepletion( bool d ) { m_depletion = d; m_changed = true; }

        double rdson() { return m_RDSon; }
        void setRDSon( double rdson );

        double threshold() { return m_threshold; }
        void setThreshold( double th );
        
    protected:
        void updateValues();

        double m_accuracy;
        double m_lastCurrent;
        double m_threshold;
        double m_kRDSon;
        double m_RDSon;
        double m_gateV;
        double m_Gth;

        bool m_Pchannel;
        bool m_depletion;
};

#endif
