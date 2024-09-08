/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef PLOTBASE_H
#define PLOTBASE_H

#include "component.h"
#include "scriptbase.h"
#include "datachannel.h"

class IoPin;
class PlotDisplay;
class QGraphicsProxyWidget;

class PlotBase : public Component, public ScriptBase
{
    public:
        PlotBase( QString type, QString id );
        ~PlotBase();

        virtual bool setPropStr( QString prop, QString val ) override;

        virtual void initialize() override;
        virtual void runEvent() override;

        int baSizeX() { return m_baSizeX; }
        void setBaSizeX( int size );

        int baSizeY() { return m_baSizeY; }
        void setBaSizeY( int size );

        int bufferSize() { return m_bufferSize; }
        void setBufferSize( int bs );

        bool connectGnd() { return m_connectGnd; }
        void setConnectGnd( bool c );

        double inputImped(){ return 1/m_inputAdmit; }
        void setInputImped( double i );

        bool doTest() { return m_doTest; }
        void setDoTest( bool t ) { m_doTest = t; }

        double testTime() { return m_testTime; }
        void setTestTime( double t ) { m_testTime = t;}

        QString testData();
        void setTestData( QString td );

        QString timDiv();
        void setTimDiv( QString td );

        virtual QString timPos()=0;
        virtual void setTimPos( QString tp )=0;

        virtual QString volDiv()=0;
        virtual void setVolDiv( QString vd )=0;

        uint64_t timeDiv() { return m_timeDiv; }
        virtual void setTimeDiv( uint64_t td );

        virtual void moveTimePos( int64_t delta )=0;

        int trigger() { return m_trigger; }
        virtual void setTrigger( int ch )=0;

        virtual QString tunnels();
        virtual void setTunnels( QString tunnels )=0;

        virtual void expand( bool ){;}
        void toggleExpand() { expand( !m_expand ); }

        QString conds() { return m_conditions; }
        virtual void setConds( QString ){;}
        void updateConds( QString conds );

        int timeStep() { return m_timeStep; }
        void setTimeStep( int t ) { m_timeStep = t; }

        bool autoExport() { return m_autoExport; }
        void setAutoExport( bool a ) { m_autoExport = a; }
        QString getExportFile() { return m_exportFile; }
        void dump() { dumpData( m_exportFile ); }
        virtual void dumpData( const QString& ){;}

        virtual void channelChanged( int ch, QString name ) { m_channel[ch]->m_chTunnel = name; }

        PlotDisplay* display() { return m_display; }

        QColor getColor( int c ) { return m_color[c]; }

        void conditonMet( int ch, cond_t cond );

        virtual void remove() override;

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w ) override;

    protected:
        virtual void slotProperties() override;
        void updtProperties();

        int m_bufferSize;
        int m_trigger;

        bool m_expand;

        int m_screenSizeX;
        int m_screenSizeY;
        int m_baSizeX;
        int m_baSizeY;

        uint64_t m_timeDiv;
        uint64_t m_risEdge;

        int m_numChannels;
        DataChannel* m_channel[8];

        int m_condCh1;
        int m_condCh2;
        int m_condCh3;
        int m_condCh4;
        int m_condCh5;
        int m_condCh6;
        int m_condCh7;
        int m_condCh8;
        bool m_pause;

        bool m_doTest;
        double m_testTime;

        int m_timeStep;    // For export
        bool m_autoExport;
        QString m_exportFile;

        bool m_connectGnd;
        double m_inputAdmit;

        std::vector<IoPin*> m_inPin;

        asIScriptFunction* m_pauseFunc;
        QString m_conditions;

        QHash<QString, QString> m_condTo;

        QColor m_color[5];

        PlotDisplay* m_display;

        QGraphicsProxyWidget* m_proxy;
};

#endif
