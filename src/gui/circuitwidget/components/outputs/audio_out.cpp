/***************************************************************************
 *   Copyright (C) 2018 by santiago González                               *
 *   santigoro@gmail.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.  *
 *                                                                         *
 ***************************************************************************/

#include "audio_out.h"
#include "connector.h"
#include "itemlibrary.h"
#include "pin.h"
#include "simulator.h"

static const char* AudioOut_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","Impedance"),
    QT_TRANSLATE_NOOP("App::Property","Buzzer")

};

Component* AudioOut::construct( QObject* parent, QString type, QString id )
{ return new AudioOut( parent, type, id ); }

LibraryItem* AudioOut::libraryItem()
{
    return new LibraryItem(
            tr( "Audio Out" ),
            tr( "Outputs" ),
            "audio_out.png",
            "AudioOut",
            AudioOut::construct);
}

AudioOut::AudioOut( QObject* parent, QString type, QString id )
        : Component( parent, type, id )
        , eResistor( id )
{
    Q_UNUSED( AudioOut_properties );
    
    m_area = QRect( -12, -24, 24, 40 );
    
    m_pin.resize( 2 );
    
    QString pinId = m_id;
    pinId.append(QString("-lPin"));
    QPoint pinPos = QPoint(-8-8,-8);
    m_pin[0] = new Pin( 180, pinPos, pinId, 0, this);
    m_pin[0]->setLabelText( "+" );
    m_pin[0]->setLabelColor( QColor( 0, 0, 0 ) );
    m_ePin[0] = m_pin[0];

    pinId = m_id;
    pinId.append(QString("-rPin"));
    pinPos = QPoint(-8-8,0);
    m_pin[1] = new Pin( 180, pinPos, pinId, 1, this);
    m_pin[1]->setLabelText( " -" );
    m_pin[1]->setLabelColor( QColor( 0, 0, 0 ) );
    m_ePin[1] = m_pin[1];

    m_idLabel->setPos(-12,-24);
    setLabelPos(-20,-36, 0);
    
    m_resist = 8;
    m_buzzer = false;
    
    int refreshPeriod = 10; // mS
    int sampleRate    = 40000; // samples/S
    
    m_deviceinfo = QAudioDeviceInfo::defaultOutputDevice(); 
    if( m_deviceinfo.isNull() ) 
    {
        qDebug() <<"No defaulf Audio Output Device Found" ;
        return;
    }
    m_format.setSampleRate( sampleRate );  
    m_format.setChannelCount(1);
    m_format.setSampleSize(8);
    m_format.setCodec( "audio/pcm" );  
    m_format.setByteOrder( QAudioFormat::LittleEndian );  
    m_format.setSampleType( QAudioFormat::UnSignedInt );  
    
    if( !m_deviceinfo.isFormatSupported( m_format )) 
    {  
        qDebug() << "Default format not supported - trying to use nearest";  
        m_format = m_deviceinfo.nearestFormat( m_format );  
        
        qDebug() << m_format.sampleRate() << m_format.channelCount()<<m_format.sampleSize();
    }  
    m_audioOutput = new QAudioOutput( m_deviceinfo, m_format );   
    
    m_dataSize = refreshPeriod*sampleRate/1000;
    m_dataBuffer.resize( m_dataSize );

    initialize();
}

AudioOut::~AudioOut()
{
}

void AudioOut::stamp()
{
    if( m_deviceinfo.isNull() ) return;
    
    if( m_ePin[0]->isConnected() && m_ePin[1]->isConnected() )
        Simulator::self()->addEvent( 1, this );
    
    eResistor::stamp();
}

void AudioOut::initialize()
{
    if( m_deviceinfo.isNull() ) return;
    m_dataCount = 0;
    
    m_auIObuffer = m_audioOutput->start();
}

void AudioOut::runEvent()
{
    double voltPN = m_ePin[0]->getVolt()-m_ePin[1]->getVolt();

    int outVal = 128;

    if( m_buzzer)
    {
        if( voltPN > 2.5 )
        {
            double stepsPC = 1e12/1000;
            double time = Simulator::self()->circTime();
            time = remainder( time, stepsPC );
            time = qDegreesToRadians( time*360/stepsPC );

            outVal += sin( time )*128;
        }
    }
    else outVal += voltPN*51;

    if     ( outVal > 255 ) outVal = 255;
    else if( outVal < 0 )   outVal = 0;

    m_dataBuffer[ m_dataCount ] = (char)outVal;
    m_dataCount++;

    if( m_dataCount == m_dataSize )
    {
        m_dataCount = 0;
        m_auIObuffer->write( m_dataBuffer.data(), m_dataSize );
    }
    double realSpeed = Simulator::self()->realSpeed();
    if( realSpeed < 1e-6 )
    {
        realSpeed = Simulator::self()->stepsPerSec();
        realSpeed *= Simulator::self()->stepSize();
        realSpeed /= 1e8; // 1e12/10000
    }
    double nextEvent = realSpeed*25*1e2;//(realSpeed/10000)*25*1e6
    Simulator::self()->addEvent( nextEvent, this ); // 25 us
}

QPainterPath AudioOut::shape() const
{
    QPainterPath path;
    
    QVector<QPointF> points;
    
    points << QPointF(-10,-12 )
           << QPointF(-10, 4 )
           << QPointF( 0, 4 )
           << QPointF( 10, 16 )
           << QPointF( 10, -24 )
           << QPointF( 0, -12 )
           << QPointF(-10, -12 );
        
    path.addPolygon( QPolygonF(points) );
    path.closeSubpath();
    return path;
}

void AudioOut::paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    Component::paint( p, option, widget );

    //p->drawRect( -10.5, -12, 12, 24 );
    static const QPointF points[7] = {
        QPointF(-10,-12 ),
        QPointF(-10, 4 ),
        QPointF( 0, 4 ),
        QPointF( 10, 16 ),
        QPointF( 10, -24 ),
        QPointF( 0, -12 ),
        QPointF(-10, -12 )
        };

    p->drawPolygon(points, 7);
    
    if( m_deviceinfo.isNull() )
    {
        p->drawLine(0,-8, 7, 0 );
        p->drawLine( 7,-8,0, 0 );
    }
}

#include "moc_audio_out.cpp"
