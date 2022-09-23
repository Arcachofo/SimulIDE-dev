/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef EELEMENT_H
#define EELEMENT_H

#include <vector>
#include <QString>

class ePin;

class MAINMODULE_EXPORT eElement
{
    public:
        eElement( QString id );
        virtual ~eElement();

        virtual void initialize(){;}
        virtual void stamp(){;}

        virtual void runEvent(){;}
        virtual void voltChanged(){;}

        virtual void setNumEpins( int n );

        virtual ePin* getEpin( int num );
        virtual void setEpin( int num, ePin* pin );

        QString getId(){ return m_elmId; }

        static constexpr double cero_doub = 1e-14;
        static constexpr double high_imp  = 1e14;

        // Simulator engine
        eElement* nextChanged;
        bool added;

        eElement* nextEvent;
        uint64_t eventTime;

    protected:
        std::vector<ePin*> m_ePin;

        QString m_elmId;

        bool m_changed;
        double m_step;
};

#endif

