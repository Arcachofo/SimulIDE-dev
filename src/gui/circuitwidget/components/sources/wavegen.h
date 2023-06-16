/***************************************************************************
 *   Copyright (C) 2018 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef WAVEGEN_H
#define WAVEGEN_H

#include "clock-base.h"

class LibraryItem;

class MAINMODULE_EXPORT WaveGen : public ClockBase
{
    public:
        WaveGen( QObject* parent, QString type, QString id );
        ~WaveGen();
        
        enum wave_type {
            Sine = 0,
            Saw,
            Triangle,
            Square,
            Random,
            Wav
        };

 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem* libraryItem();

        virtual bool setPropStr( QString prop, QString val ) override;

        virtual void stamp() override;
        virtual void runEvent() override;

        double duty() { return m_duty; }
        void setDuty( double duty );

        int steps() { return m_steps; }
        void setSteps( int steps );

        QString waveType() { return m_enumUids.at((int)m_waveType); }
        void setWaveType( QString t );

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

        virtual void setFreq( double freq ) override;

        virtual void updtValues() override;

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )override;
        
    public slots:
        void slotLoad();
        virtual void contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu ) override;

    protected slots:
        virtual void slotProperties() override;

    private:
        void genSine();
        void genSaw();
        void genTriangle();
        void genSquare();
        void genRandom();
        void genWav();

        void udtProperties();

        double normalize( double data );
        
        bool m_bipolar;
        bool m_floating;

        wave_type m_waveType;
        double m_duty;
        double m_vOut;
        double m_voltMid;
        double m_voltBase;
        double m_lastVout;
        double m_halfW;
        double m_time;
        
        int      m_steps;
        uint64_t m_qSteps;
        uint64_t m_nextStep;

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

#endif
