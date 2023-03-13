/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef LOGICSUBC_H
#define LOGICSUBC_H

#include "subcircuit.h"

class MAINMODULE_EXPORT LogicSubc : public SubCircuit
{
    public:
        LogicSubc( QObject* parent, QString type, QString id );
        ~LogicSubc();

        double inputHighV() { return m_inHighV; }
        void setInputHighV( double volt );
        double inputLowV() { return m_inLowV; }
        void setInputLowV( double volt );
        double outHighV() { return m_ouHighV; }
        void  setOutHighV( double volt );
        double outLowV() { return m_ouLowV; }
        void  setOutLowV( double volt );
        double inputImp() { return m_inImp; }
        void setInputImp( double imp );
        double outImp() { return m_ouImp; }
        void  setOutImp( double imp );
        double propDelay() { return m_propDelay*1e-12; }
        void setPropDelay( double pd );
        double riseTime() { return m_timeLH*1e-12; }
        void setRiseTime( double time );
        double fallTime() { return m_timeHL*1e-12; }
        void setFallTime( double time );

    protected:
        double m_inHighV;
        double m_inLowV;
        double m_ouHighV;
        double m_ouLowV;

        double m_inImp;
        double m_ouImp;

        uint64_t m_propDelay; // Propagation delay
        uint64_t m_timeLH;    // Time for Output voltage to switch from 10% to 90%
        uint64_t m_timeHL;    // Time for Output voltage to switch from 90% to 10%

};
#endif
