/***************************************************************************
 *   Copyright (C) 2018 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QPainter>
#include <QFileInfo>
#include <QFileDialog>
#include <QDir>
#include <QtMath>
#include <QDebug>

#include "wavegen.h"
#include "iopin.h"
#include "simulator.h"
#include "circuit.h"
#include "itemlibrary.h"
#include "propdialog.h"
#include "mainwindow.h"
#include "circuitwidget.h"

#include "doubleprop.h"
#include "stringprop.h"
#include "boolprop.h"
#include "intprop.h"

Component* WaveGen::construct( QObject* parent, QString type, QString id )
{ return new WaveGen( parent, type, id ); }

LibraryItem* WaveGen::libraryItem()
{
    return new LibraryItem(
        tr( "Wave Gen." ),
        "Sources",
        "wavegen.png",
        "WaveGen",
        WaveGen::construct );
}

WaveGen::WaveGen( QObject* parent, QString type, QString id )
       : ClockBase( parent, type, id )
{
    m_bipolar  = false;
    m_floating = false;
    m_voltBase = -2.5;
    m_voltMid  = 0;
    m_lastVout = 0;
    m_waveType = Sine;
    m_wavePixmap = NULL;

    m_pin.resize(2);
    m_pin[1] = m_gndpin = new IoPin( 0, QPoint(16,4), id+"-gndnod", 0, this, source );
    m_gndpin->setVisible( false );

    m_enumUids = QStringList()
        << "Sine"
        << "Saw"
        << "Triangle"
        << "Square"
        << "Random"
        << "Wav";

    m_enumNames = QStringList()
        <<tr("Sine")
        <<tr("Saw")
        <<tr("Triangle")
        <<tr("Square")
        <<tr("Random")
        <<   "Wav";
    
    setSteps( 100 );
    setDuty( 50 );

    Simulator::self()->addToUpdateList( this );

    remPropGroup( tr("Main") );
    addPropGroup( { tr("Main"), {
new StrProp <WaveGen>("Wave_Type", tr("Wave Type"),""      , this, &WaveGen::waveType, &WaveGen::setWaveType,0,"enum" ),
new DoubProp<WaveGen>("Freq"     , tr("Frequency"),"Hz"    , this, &WaveGen::freq,     &WaveGen::setFreq ),
new IntProp <WaveGen>("Steps"    , tr("Quality")  ,"_Steps", this, &WaveGen::steps,    &WaveGen::setSteps ),
new DoubProp<WaveGen>("Duty"     , tr("Duty")     ,"_\%"   , this, &WaveGen::duty,     &WaveGen::setDuty ),
new StrProp <WaveGen>("File"     , tr("File"),""           , this, &WaveGen::fileName, &WaveGen::setFile ),
new BoolProp<WaveGen>("Always_On", tr("Always On"),""      , this, &WaveGen::alwaysOn, &WaveGen::setAlwaysOn )
    },0} );
    addPropGroup( { tr("Electric"), {
new BoolProp<WaveGen>("Bipolar"   , tr("Bipolar")       ,"" , this, &WaveGen::bipolar,   &WaveGen::setBipolar, propNoCopy ),
new BoolProp<WaveGen>("Floating"  , tr("Floating")      ,"" , this, &WaveGen::floating,  &WaveGen::setFloating, propNoCopy ),
new DoubProp<WaveGen>("Semi_Ampli", tr("Semi Amplitude"),"V", this, &WaveGen::semiAmpli, &WaveGen::setSemiAmpli ),
new DoubProp<WaveGen>("Mid_Volt"  , tr("Middle Voltage"),"V", this, &WaveGen::midVolt,   &WaveGen::setMidVolt )
    },0} );

    setWaveType( "Sine" );
}
WaveGen::~WaveGen()
{
    delete m_wavePixmap;
}

bool WaveGen::setPropStr( QString prop, QString val )
{
    if( prop =="Volt_Base" ) m_voltBase = val.toDouble(); //  Old: TODELETE
    else return ClockBase::setPropStr( prop, val );
    return true;
}

void WaveGen::stamp()
{
    ClockBase::stamp();
    m_lastVout = m_vOut = 0;
    m_index = 0;

    if( m_bipolar && m_floating )
    {
        m_outpin->skipStamp( true );
        m_gndpin->skipStamp( true );

        m_outpin->setEnodeComp( m_gndpin->getEnode() );
        m_gndpin->setEnodeComp( m_outpin->getEnode() );

        m_outpin->createCurrent();
        m_gndpin->createCurrent();

        m_outpin->setImp( cero_doub );
        m_gndpin->setImp( cero_doub );
    }
    else{
        m_outpin->skipStamp( false );
        m_gndpin->skipStamp( false );

        m_outpin->setImp( cero_doub );
        m_gndpin->setImp( cero_doub );
    }
}

void WaveGen::runEvent()
{
    m_time = fmod( (Simulator::self()->circTime()-m_lastTime), m_fstepsPC );
    
    if     ( m_waveType == Sine )     genSine();
    else if( m_waveType == Saw )      genSaw();
    else if( m_waveType == Triangle ) genTriangle();
    else if( m_waveType == Square )   genSquare();
    else if( m_waveType == Random )   genRandom();
    else if( m_waveType == Wav )      genWav();

    if( m_vOut != m_lastVout )
    {
        m_lastVout = m_vOut;

        if( m_bipolar )
        {
            double volt = m_voltage*(m_vOut-0.5);
            if( !m_floating ) volt /= 2;

            m_outpin->setVoltage( m_voltMid+volt );
            m_gndpin->setVoltage( m_voltMid-volt );
        }
        else m_outpin->setVoltage( m_voltBase+m_voltage*m_vOut );
    }
    m_remainder += m_fstepsPC-(double)m_stepsPC;
    uint64_t remainerInt = m_remainder;
    m_remainder -= remainerInt;

    if( m_isRunning )
        Simulator::self()->addEvent( m_nextStep+remainerInt, this );
}

void WaveGen::genSine()
{
    m_time = qDegreesToRadians( (double)m_time*360/m_fstepsPC );
    m_vOut = sin( m_time )/2+0.5;
    m_nextStep = m_qSteps;
}

void WaveGen::genSaw()
{
    m_vOut = m_time/m_fstepsPC;
    m_nextStep = m_qSteps;
}

void WaveGen::genTriangle()
{
    if( m_time >= m_halfW ) m_vOut = 1-(m_time-m_halfW)/(m_fstepsPC-m_halfW);
    else                    m_vOut = m_time/m_halfW;
    m_nextStep = m_qSteps;
}

void WaveGen::genSquare()
{
    if( m_vOut == 1 )
    {
        m_vOut = 0;
        m_nextStep = m_fstepsPC-m_halfW;
    }else{
        m_vOut = 1;
        m_nextStep = m_halfW;
}   }

void WaveGen::genRandom()
{
    m_vOut = (double)rand()/(double)RAND_MAX;
    m_nextStep = m_halfW;
}

void WaveGen::genWav()
{
    m_vOut = m_data.at( m_index );
    m_nextStep = m_qSteps;
    m_index++;
    if( m_index >= m_data.size() ) m_index = 0;
}

void WaveGen::setDuty( double duty )
{
    if( duty > 100 ) duty = 100;
    m_duty = duty;
    m_halfW = m_fstepsPC*m_duty/100;
}

void WaveGen::setSteps(int steps )
{
    if( steps < 1 ) steps = 1;
    m_steps = steps;
    m_qSteps  = m_stepsPC/steps;
}

void WaveGen::setFreq( double freq )
{
    ClockBase::setFreq( freq );
    setDuty( m_duty );
    setSteps( m_steps );
}

void WaveGen::setSemiAmpli( double v )
{
    m_voltBase = m_voltMid-v;
    m_voltage = v*2;
}

void WaveGen::setMidVolt( double v )
{
    m_voltBase = v-m_voltage/2;
    m_voltMid = v;
}

void WaveGen::setBipolar(bool b )
{
    if( m_bipolar == b ) return;
    m_bipolar = b;

    m_gndpin->setVisible( b );
    if( !b ) m_gndpin->removeConnector();

    if( b ) m_outpin->setY( -4 );
    else    m_outpin->setY( 0 );

    udtProperties();
}

void WaveGen::setFloating( bool f )
{
    if( m_floating == f ) return;
    m_floating = f;

    if( Simulator::self()->isRunning() ) CircuitWidget::self()->powerCircOff();

    udtProperties();
}

void WaveGen::setWaveType( QString t )
{
    int type = getEnumIndex( t );
    m_waveType = (wave_type)type;
    if( m_showVal && (m_showProperty == "Wave_Type") )
        setValLabelText( m_enumNames.at( type ) );

    QString pixmapPath;
    switch( m_waveType ) {
    case Sine:     pixmapPath = ":/sin.png"; break;
    case Saw:      pixmapPath = ":/saw.png"; break;
    case Triangle: pixmapPath = ":/tri.png"; break;
    case Square:   pixmapPath = ":/sqa.png"; break;
    case Random:   pixmapPath = ":/rnd.png"; break;
    case Wav:      pixmapPath = ":/wav.png"; break;
    }
    if( m_wavePixmap ) delete m_wavePixmap;
    m_wavePixmap = new QPixmap( pixmapPath );
    udtProperties();
    update();
}

void WaveGen::udtProperties()
{
    if( !m_propDialog ) return;

    bool showFile = false;
    bool showDuty = false;
    bool showSteps = true;

    switch( m_waveType ) {
    case Triangle: showDuty = true; break;
    case Square:   showDuty = true; showSteps = false; break;
    case Wav:      showFile = true; showSteps = false; break;
    default: break;
    }
    m_propDialog->showProp("File", showFile );
    m_propDialog->showProp("Duty", showDuty );
    m_propDialog->showProp("Steps", showSteps );

    //m_propDialog->showProp("Mid_Volt", !m_bipolar || !m_floating );
    m_propDialog->showProp("Floating", m_bipolar );
}

void WaveGen::slotProperties()
{
    Component::slotProperties();
    udtProperties();
}

void WaveGen::updtValues()
{
    //m_propDialog
}

void WaveGen::contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu )
{
    if( m_waveType == Wav )
    {
        QAction* loadAction = menu->addAction( QIcon(":/load.svg"),tr("Load Wav File") );
        connect( loadAction, &QAction::triggered,
                       this, &WaveGen::slotLoad, Qt::UniqueConnection );

        menu->addSeparator();
    }
    Component::contextMenu( event, menu );
}

void WaveGen::slotLoad()
{
    QString fil = m_background;
    if( fil.isEmpty() ) fil = Circuit::self()->getFilePath();

    const QString dir = fil;

    QString fileName = QFileDialog::getOpenFileName( 0l, tr("Load Image"), dir,
                       tr("Wav files (*.wav);;All files (*.*)"));

    if( fileName.isEmpty() ) return; // User cancels loading

    setFile( fileName );
}

void WaveGen::setFile( QString fileName )
{
    if( Simulator::self()->isRunning() ) CircuitWidget::self()->powerCircOff();

    m_fileName = fileName;
    if( fileName.isEmpty() ) return;

    QDir circuitDir;
    circuitDir = QFileInfo( Circuit::self()->getFilePath() ).absoluteDir();
    QString fileNameAbs  = circuitDir.absoluteFilePath( fileName );

    if( !QFileInfo::exists( fileNameAbs ) )
    {
        qDebug() << "WaveGen::setFile Error: file doesn't exist:\n"<<fileNameAbs<<"\n";
        return;
    }
    QFile WAVFile( fileNameAbs );
    if( !WAVFile.open( QIODevice::ReadWrite ) )
    {
       qDebug() << "WaveGen::setFile Could not open:\n" << fileNameAbs<<"\n";
       return;
    }
    Simulator::self()->pauseSim();

    QDataStream dataStream( &WAVFile );
    dataStream.setByteOrder( QDataStream::LittleEndian );
    char strm[4];
    int32_t dataSize = 0;
    while( true )                   // Read Header
    {
        dataStream.readRawData( strm, 4 );    // 4 File Format = "RIFF"
        if( QString( QByteArray(strm,4) ) != "RIFF" ) break;
        dataStream.readRawData( strm, 4 );    // 4 File Size
        dataStream.readRawData( strm, 4 );    // 4 File Type = "WAVE"
        if( QString( QByteArray(strm,4) ) != "WAVE" ) break;
        dataStream.readRawData( strm, 4 );    // 4 Format section header= "fmt "
        if( QString( QByteArray(strm,4) ) != "fmt " ) break;
        dataStream.readRawData( strm, 4 );    // 4 Size of Format section
        dataStream >> m_audioFormat;          // 2 Format type
        dataStream >> m_numChannels;          // 2 Number of channels
        dataStream >> m_sampleRate;           // 4 Sample rate
        dataStream.readRawData( strm, 4 );    // 4 Byte rate: (Sample Rate * BitsPerSample * Channels) / 8.
        dataStream >> m_blockSize;            // 2 Block size (bytes): (BitsPerSample * Channels) / 8.
        dataStream >> m_bitsPerSample;        // 2 Bits per sample
        dataStream.readRawData( strm, 4 );    // 4 Data section header = "data"
        QString section = QString( QByteArray(strm,4) );
        while( section  != "data" )
        {
            qDebug() << "WaveGen::setFile Warning: Section not supported: " << section;
            uint32_t size = 0;   // 4 Size of Format section
            dataStream >> size;
            uint8_t data = 0;
            for( uint i=0; i<size; i++ ) dataStream >> data;
            dataStream.readRawData( strm, 4 );
            section = QString( QByteArray(strm,4) );
        }
        dataStream >> dataSize; // Size of Data section
        break;
    }
    if( dataSize )                  // Read samples
    {
        int bytes = m_blockSize/m_numChannels;

        if( m_audioFormat == 1 ) // PCM
        {
            qDebug() << "WaveGen::setFile Audio format: PCM" << m_bitsPerSample << "bits"<<m_numChannels<<"Channels";
            if( bytes == 1 ){
                m_minValue = 0;
                m_maxValue = 255;
            }else if( bytes == 2 ){
                m_minValue = -32768;
                m_maxValue = 32767;
            }
            else { qDebug() << "WaveGen::setFile Error: PCM format"<<bytes<<"bytes"; WAVFile.close(); return;}
        }
        else if( m_audioFormat == 3 ) // IEEE_FLOAT
        {
            if( bytes == 4 || bytes == 8 ){
                qDebug() << "WaveGen::setFile Audio format: IEEE_FLOAT" << m_bitsPerSample << "bits"<<m_numChannels<<"Channels";
                m_minValue = -1;
                m_maxValue = 1;
            }
            else { qDebug() << "WaveGen::setFile Error: IEEE_FLOAT format"<<bytes<<"bytes"; WAVFile.close(); return;}
        }
        else { qDebug() << "WaveGen::setFile Error: Audio format not supported:"; WAVFile.close(); return;}

        char sample[bytes];
        m_data.clear();
        //m_data.resize( m_numChannels );

        for( int block=0; block < dataSize/m_blockSize; ++block )
        {
            for( int ch=0; ch < m_numChannels; ++ch )
            {
                dataStream.readRawData( sample, bytes );
                if( ch != 0 ) continue;

                if( m_audioFormat == 1 ) // PCM
                {
                    int16_t data = 0;
                    memcpy( &data, sample, bytes );
                    m_data.emplace_back( normalize( data ) );
                }
                else if( m_audioFormat == 3 ) // IEEE_FLOAT
                {
                    if( bytes == 4 ){         // 32 bits
                        float data = 0;
                        memcpy( &data, sample, bytes );
                        m_data.emplace_back( normalize( data ) );
                    }
                    else if( bytes == 8 ){    // 64 bits
                        double data = 0;
                        memcpy( &data, sample, bytes );
                        m_data.emplace_back( normalize( data ) );
                    }
                }
            }
        }
        setFreq( m_sampleRate );
        setSteps( 1 );
        qDebug() << "WaveGen::setFile Success Loaded wav file:\n" << fileNameAbs;
    }
    else qDebug() << "WaveGen::setFile Error reading wav file:\n" << fileNameAbs;
    qDebug() << "\n";

    WAVFile.close();
}

inline double WaveGen::normalize( double data )
{
    if     ( data > m_maxValue ) return 1;
    else if( data < m_minValue ) return 0;

    data = data-m_minValue;
    data /= (m_maxValue-m_minValue);
    return data;
}

void WaveGen::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    if( m_hidden ) return;
    Component::paint( p, option, widget );

    if (  m_isRunning ) p->setBrush( QColor( 250, 200, 50 ) );
    else                p->setBrush( QColor( 230, 230, 255 ) );

    p->drawRoundedRect( m_area,2 ,2 );
    p->drawPixmap( m_area.x()+3, m_area.y()+4,16, 8, *m_wavePixmap );
}
