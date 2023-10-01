/***************************************************************************
 *   Copyright (C) 2016 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef EMOSFET_H
#define EMOSFET_H

#include "e-resistor.h"

class MAINMODULE_EXPORT eMosfet : public eResistor
{
    public:
        eMosfet( QString id );
        ~eMosfet();

        virtual void stamp() override;
        virtual void voltChanged() override;

        bool pChannel() { return m_Pchannel; }
        void setPchannel( bool p ) { m_Pchannel = p; m_changed = true; }

        bool depletion() { return m_depletion; }
        void setDepletion( bool d ) { m_depletion = d; m_changed = true; }

        double rdson() { return m_RDSon; }
        void setRDSon( double rdson );

        double threshold() { return m_threshold; }
        void setThreshold( double th );
        
    protected:
        bool converged( double pre, double post );

        double m_threshold;
        double m_kRDSon;
        double m_RDSon;
        double m_gateV;
        double m_Gth;

        double m_Vd;
        double m_Vs;
        double m_Vg;

        bool m_Pchannel;
        bool m_depletion;
};

#endif
