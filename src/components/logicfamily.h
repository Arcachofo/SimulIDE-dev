/***************************************************************************
 *   Copyright (C) 2024 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef LOGICFAMILY_H
#define LOGICFAMILY_H

#include <QMap>

struct logicFamily_t{
    double supply;
    double inpLHp;
    double inpHLp;
    double inpImp;
    double outHip;
    double outLop;
    double outImp;
};

class LogicFamily
{
    public:
        LogicFamily();
        ~LogicFamily();

        double supplyV() { return m_supplyV; }
        virtual void setSupplyV( double v ){;}

        double inpHighV() { return m_inHighV; }
        virtual void setInpHighV( double v );

        double inpLowV() { return m_inLowV; }
        virtual void setInpLowV( double v );

        double outHighV() { return m_ouHighV; }
        virtual void setOutHighV( double v );

        double outLowV() { return m_ouLowV; }
        virtual void setOutLowV( double v );

        double inputImp() { return m_inImp; }
        virtual void setInputImp( double i ){;}

        double outImp() { return m_ouImp; }
        virtual void setOutImp( double i ){;}

        double propSize() { return m_propSize; }
        void setPropSize( double g ) { m_propSize = g; }

        double propDelay() { return m_propDelay*1e-12; }
        void setPropDelay( double pd );

        double riseTime() { return m_timeLH*1e-12; }
        virtual void setRiseTime( double time );

        double fallTime() { return m_timeHL*1e-12; }
        virtual void setFallTime( double time );

        QString family() { return m_family; }
        void setFamily( QString f );
        void setFamilyData( logicFamily_t lf );

        virtual QStringList getEnumUids( QString ) { return m_families.keys(); }
        virtual QStringList getEnumNames( QString ) { return m_families.keys(); }

 static void getFamilies();

    protected:
        void updateData();

        double m_propDelay; // Propagation delay
        double m_timeLH;    // Time for Output voltage to switch from 10% to 90% (1 gate)
        double m_timeHL;    // Time for Output voltage to switch from 90% to 10% (1 gate)
        double m_propSize;  // Nunmber of gates for total Propagation delay

        bool m_blocked;
        bool m_enableSupply;

        double m_supplyV;
        double m_inHighV;
        double m_inHighVp;
        double m_inLowV;
        double m_inLowVp;
        double m_ouHighV;
        double m_ouHighVp;
        double m_ouLowV;
        double m_ouLowVp;

        double m_inImp;
        double m_ouImp;

        QString m_family;

 static QMap<QString, logicFamily_t> m_families;
};
#endif
