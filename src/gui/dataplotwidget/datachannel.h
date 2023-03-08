/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef DATACHANNEL_H
#define DATACHANNEL_H

#include "e-element.h"
#include "updatable.h"

#include <QVector>

enum cond_t{
    C_NONE=0,
    C_LOW,
    C_RISING,
    C_HIGH,
    C_FALLING
};

class PlotBase;
class Pin;

class MAINMODULE_EXPORT DataChannel : public eElement, public Updatable
{
        friend class PlotBase;
        friend class Oscope;
        friend class LAnalizer;
        friend class PlotDisplay;

    public:

        DataChannel( PlotBase* plotBase, QString id );
        ~DataChannel();

        virtual void stamp() override;

        virtual void setFilter( double f ) {;}

        bool isBus();
        virtual void setIsBus( bool b ){;}

        QString getChName() { return m_chTunnel; }

    protected:
        QVector<double> m_buffer;
        QVector<uint64_t> m_time;

        bool m_connected;
        bool m_rising;
        bool m_falling;
        bool m_trigger;
        int m_trigIndex;

        uint64_t m_risEdge;
        uint64_t m_period;

        int m_channel;
        int m_bufferCounter;

        bool m_pauseOnCond;
        //cond_t m_cond;
        cond_t m_lastCond;

        QString m_chTunnel;

        Pin* m_pin;

        PlotBase* m_plotBase;
};

#endif
