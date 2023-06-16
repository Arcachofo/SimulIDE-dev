/***************************************************************************
 *   Copyright (C) 2010 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef CLOCKBASE_H
#define CLOCKBASE_H

#include "fixedvolt.h"

class MAINMODULE_EXPORT ClockBase : public FixedVolt
{

    public:
        ClockBase( QObject* parent, QString type, QString id );
        ~ClockBase();

        virtual bool setPropStr( QString prop, QString val ) override;

        virtual void stamp() override;
        virtual void updateStep() override;

        bool alwaysOn() { return m_alwaysOn; }
        void setAlwaysOn( bool on );

        double freq() { return m_freq; }
        virtual void setFreq( double freq );

        bool running() { return m_isRunning; }
        virtual void setRunning( bool running );

        virtual void setOut( bool ) override {;}

    public slots:
        virtual void onbuttonclicked() override;

    protected:
        bool m_state;
        bool m_isRunning;
        bool m_alwaysOn;

        uint64_t m_lastTime;
        uint64_t m_stepsPC;
        double m_fstepsPC;

        double m_freq;
        double m_remainder;
};

#endif
