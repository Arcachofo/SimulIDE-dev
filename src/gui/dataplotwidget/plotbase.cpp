/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "plotbase.h"
#include "plotdisplay.h"
#include "simulator.h"
#include "circuit.h"
#include "circuitwidget.h"
#include "utils.h"

#include "stringprop.h"
#include "doubleprop.h"
#include "intprop.h"

PlotBase::PlotBase( QObject* parent, QString type, QString id )
        : Component( parent, type, id )
        , ScriptModule( id )
{
    m_graphical = true;
    m_bufferSize = 600000;

    m_color[0] = QColor( 240, 240, 100 );
    m_color[1] = QColor( 220, 220, 255 );
    m_color[2] = QColor( 255, 210, 90  );
    m_color[3] = QColor( 000, 245, 160 );
    m_color[4] = QColor( 255, 255, 255 );

    m_baSizeX = 135;
    m_baSizeY = 135;

    int r;
    m_pauseFunc = NULL;
    m_aEngine->RegisterObjectType("PlotBase",0, asOBJ_REF | asOBJ_NOCOUNT );
    m_aEngine->RegisterGlobalProperty("PlotBase pb", this );
    r = m_aEngine->RegisterObjectProperty("PlotBase", "bool m_pause", asOFFSET(PlotBase,m_pause)); //assert( r >= 0 );
    r = m_aEngine->RegisterObjectProperty("PlotBase", "int ch1", asOFFSET(PlotBase,m_condCh1)); //assert( r >= 0 );
    r = m_aEngine->RegisterObjectProperty("PlotBase", "int ch2", asOFFSET(PlotBase,m_condCh2)); //assert( r >= 0 );
    r = m_aEngine->RegisterObjectProperty("PlotBase", "int ch3", asOFFSET(PlotBase,m_condCh3)); //assert( r >= 0 );
    r = m_aEngine->RegisterObjectProperty("PlotBase", "int ch4", asOFFSET(PlotBase,m_condCh4)); //assert( r >= 0 );
    r = m_aEngine->RegisterObjectProperty("PlotBase", "int ch5", asOFFSET(PlotBase,m_condCh5)); //assert( r >= 0 );
    r = m_aEngine->RegisterObjectProperty("PlotBase", "int ch6", asOFFSET(PlotBase,m_condCh6)); //assert( r >= 0 );
    r = m_aEngine->RegisterObjectProperty("PlotBase", "int ch7", asOFFSET(PlotBase,m_condCh7)); //assert( r >= 0 );
    r = m_aEngine->RegisterObjectProperty("PlotBase", "int ch8", asOFFSET(PlotBase,m_condCh8)); //assert( r >= 0 );

    QString n;
    for( int i=1; i<9; ++i ) // Condition simple to script: ch1l to (ch1==1)
    {
        n = QString::number(i);
        m_condTo["ch"+n+"l"] = "(pb.ch"+n+"==1)";
        m_condTo["ch"+n+"r"] = "(pb.ch"+n+"==2)";
        m_condTo["ch"+n+"h"] = "(pb.ch"+n+"==3)";
        m_condTo["ch"+n+"f"] = "(pb.ch"+n+"==4)";
    }
    Simulator::self()->addToUpdateList( this );

    m_timeStep = 1000;
    m_autoExport = false;
    m_exportFile = changeExt( Circuit::self()->getFilePath(), "_"+id+".vcd" );

    addPropGroup( { tr("Main"), {
new IntProp<PlotBase>( "Basic_X"    ,tr("Screen Size X"),tr("_Pixels"), this, &PlotBase::baSizeX,    &PlotBase::setBaSizeX   ,0, "uint" ),
new IntProp<PlotBase>( "Basic_Y"    ,tr("Screen Size Y"),tr("_Pixels"), this, &PlotBase::baSizeY,    &PlotBase::setBaSizeY   ,0, "uint" ),
new IntProp<PlotBase>( "BufferSize" ,tr("Buffer Size")  ,tr("Samples"), this, &PlotBase::bufferSize, &PlotBase::setBufferSize,0,"uint" ),
    }, groupNoCopy} );
    addPropGroup( {"Hidden", {
new StrProp<PlotBase>( "TimDiv"  ,"","", this, &PlotBase::timDiv,  &PlotBase::setTimDiv ),
new StrProp<PlotBase>( "TimPos"  ,"","", this, &PlotBase::timPos,  &PlotBase::setTimPos ),
new StrProp<PlotBase>( "VolDiv" ,"", "", this, &PlotBase::volDiv,  &PlotBase::setVolDiv ),
new StrProp<PlotBase>( "Conds"   ,"","", this, &PlotBase::conds,   &PlotBase::setConds ),
new StrProp<PlotBase>( "Tunnels" ,"","", this, &PlotBase::tunnels, &PlotBase::setTunnels ),
new IntProp<PlotBase>( "Trigger" ,"","", this, &PlotBase::trigger, &PlotBase::setTrigger ),
    }, groupHidden } );
}
PlotBase::~PlotBase()
{
    for( int i=0; i<m_numChannels; i++ ) delete m_channel[i];
}

bool PlotBase::setPropStr( QString prop, QString val )
{
    if     ( prop =="hTick" ) setTimeDiv( val.toLongLong()*1e3 ); // Old: TODELETE
    else if( prop =="vTick" ) setVolDiv( val );
    else if( prop =="TimePos" ) setTimPos( val+"000" );
    else return Component::setPropStr( prop, val );
    return true;
}

void PlotBase::setBaSizeX( int size )
{
    if( size < 135 ) size = 135;
    m_baSizeX = size;
    expand( m_expand );
}

void PlotBase::setBaSizeY( int size )
{
    if( size < 135 ) size = 135;
    m_baSizeY = size;
    expand( m_expand );
}

void PlotBase::setBufferSize( int bs )
{
    if( bs < 0 || bs > 10000000 ) bs = 10000000;
    else if( bs < 1000 ) bs = 1000;
    m_bufferSize = bs;
    for( int i=0; i<m_numChannels; i++ )
    {
        m_channel[i]->m_buffer.resize( m_bufferSize );
        m_channel[i]->m_time.resize( m_bufferSize );
    }
}

QString PlotBase::timDiv()
{
    return QString::number( m_timeDiv );
}

void PlotBase::setTimDiv( QString td )
{
    setTimeDiv( td.toLongLong() );
}

void PlotBase::setTimeDiv( uint64_t td )
{
    m_display->setTimeDiv( td );
    m_timeDiv = m_display->m_timeDiv;
}

QString PlotBase::tunnels()
{
    QString list;
    for( int i=0; i<m_numChannels; ++i ) list.append( m_channel[i]->m_chTunnel ).append(",");
    return list;
}

void PlotBase::updateConds( QString conds )
{
    m_pauseFunc = NULL;
    m_pause = false;
    m_conditions = conds;

    if( conds.isEmpty() ) return;
    conds = conds.toLower();

    QString n;
    for( int i=1; i<9; ++i ) // Condition simple to script: ch1l to (ch1==1)
    {
        n = QString::number(i);
        conds.replace( "ch"+n+"l", m_condTo.value("ch"+n+"l") );
        conds.replace( "ch"+n+"r", m_condTo.value("ch"+n+"r") );
        conds.replace( "ch"+n+"h", m_condTo.value("ch"+n+"h") );
        conds.replace( "ch"+n+"f", m_condTo.value("ch"+n+"f") );
    }
    m_script = "void pause() { pb.m_pause = "+conds+";}";
    qDebug() << m_script <<endl;
    int r = compileScript();
    if( r < 0 ) { qDebug() << "PlotBase::updateConds Failed to compile expression:"<<conds; return; }

    m_pauseFunc = m_aEngine->GetModule(0)->GetFunctionByDecl("void pause()");
}

void PlotBase::conditonMet( int ch, cond_t cond )
{
    if( !m_pauseFunc ) return;

    ch++;
    switch ( ch ) {
        case 1: m_condCh1 = (int)cond; break;
        case 2: m_condCh2 = (int)cond; break;
        case 3: m_condCh3 = (int)cond; break;
        case 4: m_condCh4 = (int)cond; break;
        case 5: m_condCh5 = (int)cond; break;
        case 6: m_condCh6 = (int)cond; break;
        case 7: m_condCh7 = (int)cond; break;
        case 8: m_condCh8 = (int)cond; break;
        default: break;
    }
    if( Simulator::self()->simState() <= SIM_PAUSED ) return;

    callFunction( m_pauseFunc ); // Check if condition met:

    if( m_pause )
    {
        m_risEdge = Simulator::self()->circTime();
        CircuitWidget::self()->pauseCirc();
        if( m_autoExport ) QTimer::singleShot( 50, this, &PlotBase::dump );
    }

    //if( m_trigger != 8 ) return;

    /*if( m_conditions == m_condTarget ) // All conditions met
    {                                  // Trigger Pause Simulation
        m_risEdge = Simulator::self()->circTime();
        CircuitWidget::self()->pauseSim();
    }*/
    /*else  // Rising will be High and Falling Low in next cycles
    {
        if     ( cond == C_RISING )  m_conditions[ch] = C_HIGH;
        else if( cond == C_FALLING ) m_conditions[ch] = C_LOW;
    }*/
}

void PlotBase::remove()
{
    if( m_expand ) expand( false );
    Component::remove();
}

void PlotBase::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );
    
    //p->setBrush( Qt::darkGray );
    p->setBrush(QColor( 230, 230, 230 ));
    p->drawRoundedRect( m_area, 4, 4 );
    
    p->setBrush( Qt::white );
    QPen pen = p->pen();
    pen.setWidth( 0 );
    pen.setColor( Qt::white );
    p->setPen(pen);
}
