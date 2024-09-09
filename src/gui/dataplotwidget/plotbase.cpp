/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "plotbase.h"
#include "plotdisplay.h"
#include "simulator.h"
#include "circuit.h"
#include "circuitwidget.h"
#include "propdialog.h"
#include "iopin.h"
#include "batchtest.h"
#include "utils.h"

#include "stringprop.h"
#include "doubleprop.h"
#include "boolprop.h"
#include "intprop.h"

#define tr(str) simulideTr("PlotBase",str)

PlotBase::PlotBase( QString type, QString id )
        : Component( type, id )
        , ScriptBase( id )
{
    m_graphical = true;
    m_bufferSize = 600000;

    m_connectGnd = true;
    m_inputAdmit = 1e-7;

    m_doTest = false;
    m_testTime = 0;

    m_color[0] = QColor( 240, 240, 100 );
    m_color[1] = QColor( 220, 220, 255 );
    m_color[2] = QColor( 255, 210, 90  );
    m_color[3] = QColor( 000, 245, 160 );
    m_color[4] = QColor( 255, 255, 255 );

    m_baSizeX = 135;
    m_baSizeY = 135;

    m_pauseFunc = NULL;
    m_aEngine->RegisterObjectType("PlotBase",0, asOBJ_REF | asOBJ_NOCOUNT );
    m_aEngine->RegisterGlobalProperty("PlotBase pb", this );
    m_aEngine->RegisterObjectProperty("PlotBase", "bool m_pause", asOFFSET(PlotBase,m_pause));
    m_aEngine->RegisterObjectProperty("PlotBase", "int ch1", asOFFSET(PlotBase,m_condCh1));
    m_aEngine->RegisterObjectProperty("PlotBase", "int ch2", asOFFSET(PlotBase,m_condCh2));
    m_aEngine->RegisterObjectProperty("PlotBase", "int ch3", asOFFSET(PlotBase,m_condCh3));
    m_aEngine->RegisterObjectProperty("PlotBase", "int ch4", asOFFSET(PlotBase,m_condCh4));
    m_aEngine->RegisterObjectProperty("PlotBase", "int ch5", asOFFSET(PlotBase,m_condCh5));
    m_aEngine->RegisterObjectProperty("PlotBase", "int ch6", asOFFSET(PlotBase,m_condCh6));
    m_aEngine->RegisterObjectProperty("PlotBase", "int ch7", asOFFSET(PlotBase,m_condCh7));
    m_aEngine->RegisterObjectProperty("PlotBase", "int ch8", asOFFSET(PlotBase,m_condCh8));

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
        new IntProp <PlotBase>("Basic_X",tr("Screen Width"), "_px"
                              , this, &PlotBase::baSizeX, &PlotBase::setBaSizeX,0,"uint" ),

        new IntProp <PlotBase>("Basic_Y",tr("Screen Height"),"_px"
                              , this, &PlotBase::baSizeY, &PlotBase::setBaSizeY,0,"uint" ),

        new IntProp <PlotBase>("BufferSize",tr("Buffer Size"),""
                              , this, &PlotBase::bufferSize, &PlotBase::setBufferSize,0,"uint" ),

        new BoolProp<PlotBase>("connectGnd",tr("Connect to ground"),""
                              , this, &PlotBase::connectGnd, &PlotBase::setConnectGnd,0 ),

        new DoubProp<PlotBase>("InputImped",tr("Impedance"),"MΩ"
                              , this, &PlotBase::inputImped, &PlotBase::setInputImped )
    }, groupNoCopy} );

    addPropGroup( { tr("Test"), {
        new DoubProp<PlotBase>("TestTime",tr("Test Time"),"ns"
                              , this, &PlotBase::testTime, &PlotBase::setTestTime,0  ),

        new BoolProp<PlotBase>("DoTest",tr("Do Test"),""
                              , this, &PlotBase::doTest, &PlotBase::setDoTest,0 ),
    }, 0 } );

    addPropGroup( {"Hidden", {
        new StrProp<PlotBase>("TimDiv","",""
                             , this, &PlotBase::timDiv, &PlotBase::setTimDiv ),

        new StrProp<PlotBase>("TimPos","",""
                             , this, &PlotBase::timPos, &PlotBase::setTimPos ),

        new StrProp<PlotBase>("VolDiv","",""
                             , this, &PlotBase::volDiv, &PlotBase::setVolDiv ),

        new StrProp<PlotBase>("Conds","",""
                             , this, &PlotBase::conds, &PlotBase::setConds ),

        new StrProp<PlotBase>("Tunnels","",""
                             , this, &PlotBase::tunnels, &PlotBase::setTunnels ),

        new IntProp<PlotBase>("Trigger","",""
                             , this, &PlotBase::trigger, &PlotBase::setTrigger ),

       new StrProp<PlotBase>("TestData","",""
                            , this, &PlotBase::testData, &PlotBase::setTestData ),
    }, groupHidden } );
}
PlotBase::~PlotBase()
{
    for( int i=0; i<m_numChannels; i++ ) delete m_channel[i];
}

bool PlotBase::setPropStr( QString prop, QString val )
{
    if     ( prop =="hTick"  ) setTimeDiv( val.toLongLong()*1e3 ); // Old: TODELETE
    else if( prop =="vTick"  ) setVolDiv( val );
    else if( prop =="TimePos") setTimPos( val+"000" );
    else return Component::setPropStr( prop, val );
    return true;
}

void PlotBase::initialize()
{
    if( m_testTime )
    {
        Simulator::self()->addEvent( m_testTime*1e12, this );
        if( BatchTest::isRunning() ) BatchTest::addTestUnit( this );
    }
}

void PlotBase::runEvent() // Test time reached, make comparison
{
    bool testOk = true;
    for( int i=0; i<m_numChannels; ++i )
    {
        if( !m_channel[i]->doTest( m_doTest ) )
        {
            testOk = false;
            qDebug() << idLabel() << "Error: Test failed for Channel" << i;
        }
    }
    if( m_doTest && testOk ) qDebug() << idLabel() << "Test passed" ;

    if( m_doTest && BatchTest::isRunning() ) BatchTest::testCompleted( this, testOk );
}

QString PlotBase::testData()
{
    QString td;
    if( !m_testTime ) return td;

    for( int i=0; i<m_numChannels; ++i ) td += m_channel[i]->testData()+";";
    td.remove( td.size()-1, 1);
    return td;
}

void PlotBase::setTestData( QString td )
{
    if( td.isEmpty() ) return;

    QStringList chDataList = td.split(";");
    for( int i=0; i<chDataList.size(); ++i )
    {
        if( i == m_numChannels ) break;
        m_channel[i]->setTestData( chDataList.at(i) );
    }
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

void PlotBase::setConnectGnd( bool c )
{
    if( m_connectGnd == c ) return;
    m_connectGnd = c;
    updtProperties();
    m_changed = true;
}

void PlotBase::setInputImped( double i )
{
    double admit = 1/i;
    if( m_inputAdmit == admit ) return;
    if( admit  < 0 ) return;
    m_inputAdmit = admit;
    m_changed = true;
    if( !Simulator::self()->isRunning() ) updateStep();
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
    /// qDebug() << m_script <<endl;
    int r = ScriptBase::compileScript();
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
        if( m_autoExport ) QTimer::singleShot( 50, [=](){ dump(); } );
    }
}

void PlotBase::slotProperties()
{
    Component::slotProperties();
    updtProperties();
}

void PlotBase::updtProperties()
{
    if( !m_propDialog ) return;
    m_propDialog->showProp("InputImped", m_connectGnd );
    m_propDialog->adjustWidgets();
}

void PlotBase::remove()
{
    if( m_expand ) expand( false );
    Component::remove();
}

void PlotBase::paint( QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w )
{
    Component::paint( p, o, w );
    
    //p->setBrush( Qt::darkGray );
    p->setBrush(QColor( 230, 230, 230 ));
    p->drawRoundedRect( m_area, 4, 4 );
    
    p->setBrush( Qt::white );
    QPen pen = p->pen();
    pen.setWidth( 0 );
    pen.setColor( Qt::white );
    p->setPen(pen);

    Component::paintSelected( p );
}
