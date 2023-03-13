/***************************************************************************
 *   Copyright (C) 2018 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef LANALIZER_H
#define LANALIZER_H

#include "plotbase.h"

class LibraryItem;
class LaChannel;
class LaWidget;
class DataLaWidget;

struct sample_t{
    double value;
    uint channel;
};

class MAINMODULE_EXPORT LAnalizer : public PlotBase
{
    public:
        LAnalizer( QObject* parent, QString type, QString id );
        ~LAnalizer();

 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem* libraryItem();

        virtual void updateStep() override;

        virtual QString timPos() override;
        virtual void setTimPos( QString tp ) override;

        virtual QString volDiv() override;
        virtual void setVolDiv( QString vd ) override;

        virtual void setTimeDiv( uint64_t td ) override;

        int64_t timePos(){ return m_timePos; }
        void setTimePos( int64_t tp );
        virtual void moveTimePos( int64_t delta ) override;

        double voltDiv(){ return m_voltDiv; }
        void setVoltDiv( double vd );

        void setIsBus( int ch, bool b ) { m_channel[ch]->setIsBus( b ); }

        virtual void setTrigger( int ch ) override;

        double thresholdR() { return m_thresholdR; }
        void setThresholdR( double thr );

        QString busStr();
        void setBusStr( QString b );

        double thresholdF() { return m_thresholdF; }
        void setThresholdF( double thr );

        virtual void setConds( QString conds ) override;

        virtual void setTunnels( QString tunnels ) override;

        virtual void expand( bool e ) override;

        virtual void dumpData( const QString& fn ) override;

    private:
        uint64_t getGcd( uint64_t a, uint64_t b ); // greatest Common Denominator

        double m_voltDiv;
        double m_thresholdR;
        double m_thresholdF;

        int m_updtCount;

        int64_t m_timePos;

        LaWidget*  m_laWidget;
        DataLaWidget* m_dataWidget;
};

#endif
