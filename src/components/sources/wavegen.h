/***************************************************************************
 *   Copyright (C) 2018 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#pragma once

#include "clock-base.h"

class LibraryItem;

class WaveGen : public ClockBase
{
    public:
        WaveGen( QString type, QString id );
        ~WaveGen();
        
        enum waveType_t {
            Sine = 0,
            Saw,
            Triangle,
            Square,
            Random,
            Wav
        };

 static Component* construct( QString type, QString id );
 static LibraryItem* libraryItem();

        bool propNotFound( QString prop, QString val ) override;

        //void updateStep() override{;}
        void initialize() override;
        void stamp() override;
        void runEvent() override;

        int minSteps() { return m_minSteps; }
        void setMinSteps( int steps );

        double duty() { return m_duty; }
        void setDuty( double duty );

        double phaseShift() { return m_phaseShift; }
        void setPhaseShift( double p ) { m_phaseShift = p; }

        QString waveType() { return m_waveTypeStr; }
        void setWaveType( QString type );

        double semiAmpli() { return m_voltage/2; }
        void setSemiAmpli( double v );

        double midVolt() { return m_voltMid; }
        void setMidVolt( double v );

        QString fileName() { return m_fileName; }
        void setFile( QString fileName );

        bool bipolar() { return m_bipolar; }
        void setBipolar( bool b );

        bool floating() { return m_floating; }
        void setFloating( bool f );

        void setFreq( double freq ) override;

        void setLinkedValue( double v, int i=0 ) override;
        
        void slotLoad();

        void paint( QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w )override;
        void contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu ) override;

    protected:
        void slotProperties() override;

    private:
        void genSine();
        void genSaw();
        void genTriangle();
        void genSquare();
        void genRandom();
        void genWav();

        void updtProperties();

        double normalize( double data );
        
        bool m_bipolar;
        bool m_floating;

        QString m_waveTypeStr;
        waveType_t m_waveType;

        double m_duty;
        double m_vOut;
        double m_voltMid;
        double m_voltBase;
        double m_lastVout;
        double m_halfW;
        double m_time;
        double m_phaseShift;
        double m_phaseTime;
        
        //int      m_steps;
        uint64_t m_minSteps;
        uint64_t m_eventTime;

        uint m_index;
        uint16_t m_audioFormat;
        uint16_t m_numChannels;
        uint32_t m_sampleRate;
        uint16_t m_blockSize;
        uint16_t m_bitsPerSample;

        double m_maxValue;
        double m_minValue;
        double m_mult;
        std::vector<double> m_data;
        QString m_fileName;

        IoPin* m_gndpin;

        QStringList m_waves;
        QPixmap* m_wavePixmap;
};
