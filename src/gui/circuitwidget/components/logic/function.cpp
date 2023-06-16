/***************************************************************************
 *   Copyright (C) 2018 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QMenu>
#include <QFileDialog>
#include <QPushButton>
#include <QInputDialog>
#include <QGraphicsProxyWidget>

#include "function.h"
#include "connector.h"
#include "circuit.h"
#include "simulator.h"
#include "circuitwidget.h"
#include "itemlibrary.h"
#include "utils.h"
#include "iopin.h"

#include "stringprop.h"
#include "boolprop.h"
#include "intprop.h"

Component* Function::construct( QObject* parent, QString type, QString id )
{ return new Function( parent, type, id ); }

LibraryItem* Function::libraryItem()
{
    return new LibraryItem(
        QCoreApplication::translate("Function", "Function"),
        "Arithmetic",
        "subc.png",
        "Function",
        Function::construct );
}

Function::Function( QObject* parent, QString type, QString id )
        : IoComponent( parent, type, id )
        , ScriptModule( id )
{
    m_lastDir = Circuit::self()->getFilePath();
    m_width = 4;

    m_inHighV = 0;
    m_inLowV  = 0;
    m_ouHighV = 0;

    m_voltChanged = NULL;
    m_aEngine->RegisterObjectType("Function",0, asOBJ_REF | asOBJ_NOCOUNT );
    m_aEngine->RegisterGlobalProperty("Function fu", this );

    int r=0;
    r += m_aEngine->RegisterObjectMethod("Function", "bool getInputState(int pin)"
                                       , asMETHODPR( Function, getInputState, (int), bool)
                                       , asCALL_THISCALL );

    r += m_aEngine->RegisterObjectMethod("Function", "double getInputVoltage(int pin)"
                                       , asMETHODPR( Function, getInputVoltage, (int), double)
                                       , asCALL_THISCALL );

    r += m_aEngine->RegisterObjectMethod("Function", "void setOutputState(int pin, bool s)"
                                       , asMETHODPR( Function, setOutputState, (int,bool), void)
                                       , asCALL_THISCALL );

    r += m_aEngine->RegisterObjectMethod("Function", "void setOutputVoltage(int pin, double v)"
                                       , asMETHODPR( Function, setOutputVoltage, (int,double), void)
                                       , asCALL_THISCALL );

    r += m_aEngine->RegisterObjectMethod("Function", "double getOutputVoltage(int pin)"
                                       , asMETHODPR( Function, getOutputVoltage, (int), double)
                                       , asCALL_THISCALL );
    if( r < 0 ) qDebug() << "Function::Function error Registering Function";

    setNumInputs( 2 );                           // Create Input Pins
    setNumOutputs( 1 );
    setFunctions( "i0 | i1" );

    addPropGroup( { tr("Main"), {
new IntProp<Function>("Num_Inputs" , tr("Input Size") ,"_Pins", this, &Function::numInps,   &Function::setNumInputs ,0,"uint" ),
new IntProp<Function>("Num_Outputs", tr("Output Size"),"_Pins", this, &Function::numOuts,   &Function::setNumOutputs,0,"uint" ),
new StrProp<Function>("Functions"  , tr("Functions")  ,""     , this, &Function::functions, &Function::setFunctions ),
    },groupNoCopy} );
    addPropGroup( { tr("Electric"), IoComponent::inputProps()
+QList<ComProperty*>({new BoolProp<Function>( "Invert_Inputs", tr("Invert Inputs"),"", this, &Function::invertInps, &Function::setInvertInps, propNoCopy )})
                    +IoComponent::outputProps()+IoComponent::outputType(),0 } );
    addPropGroup( { tr("Edges"), IoComponent::edgeProps(),0 } );
}
Function::~Function(){}

void Function::stamp()
{
    IoComponent::initState();

    for( uint i=0; i<m_inPin.size(); ++i ) m_inPin[i]->changeCallBack( this );
}

void Function::voltChanged()
{
    if( !m_voltChanged ) return;
    m_nextOutVal = 0;

    callFunction( m_voltChanged );
    sheduleOutPuts( this );
}

bool Function::getInputState( int pin )
{
    if( (uint)pin >= m_inPin.size() ) return false;
    return m_inPin[pin]->getInpState();
}

double Function::getInputVoltage( int pin )
{
    if( (uint)pin >= m_inPin.size() ) return 0;
    return m_inPin[pin]->getVoltage();
}

void Function::setOutputState( int pin, bool s )
{
    if( (uint)pin >= m_outPin.size() ) return;
    if( s ) m_nextOutVal |= 1<<pin;
}

void Function::setOutputVoltage( int pin, double v )
{
    if( (uint)pin >= m_outPin.size() ) return;
    m_outPin[pin]->setOutHighV( v );
    m_nextOutVal |= 1<<pin;
    m_outValue   &= ~(1<<pin); // Force Pin update
    m_outPin[pin]->m_nextState = false; // Force Pin update
}

double Function::getOutputVoltage( int pin )
{
    if( (uint)pin >= m_outPin.size() ) return 0;
    return m_outPin[pin]->getVoltage();
}

void Function::setFunctions( QString f )
{
    m_funcList = f.split(",");
    updateFunctions();
}

void Function::updateFunctions()
{
    if( Simulator::self()->isRunning() ) CircuitWidget::self()->powerCircOff();

    m_script = "\n// "+m_id+" Script --------;\n";

    m_script += "\n// Declaring Variables:\n";
    for( uint i=0; i<m_inPin.size(); ++i )
    {
        QString n = QString::number(i);
        m_script += "double vi"+n+" = 0;\n";
        m_script += "bool   i"+n+"  = false;\n";
    }
    for( uint i=0; i<m_outPin.size(); ++i )
    {
        QString n = QString::number(i);
        m_script += "double vo"+n+" = 0;\n";
        m_script += "bool   o"+n+"  = false;\n";
    }
    m_script += "\nvoid voltChanged()\n{\n";
    m_script += "  // Getting data:\n";
    for( uint i=0; i<m_inPin.size(); ++i )
    {
        QString n = QString::number(i);
        m_script += "  vi"+n+" = fu.getInputVoltage("+n+");\n";
        m_script += "  i"+n+"  = fu.getInputState("+n+");\n";
        m_script += "\n";
    }
    for( uint i=0; i<m_outPin.size(); ++i )
    {
        QString n = QString::number(i);
        m_script += "  vo"+n+" = fu.getOutputVoltage("+n+");\n";
    }
    m_script += "\n  // Setting Outputs:\n";
    for( int i=0; i<m_funcList.size(); ++i )
    {
        if( i >= (int)m_outPin.size() ) break;
        QString n = QString::number(i);

        QString func = m_funcList.at( i );
        if( func.isEmpty() ) continue;

        func = func.replace("&","&&").replace("|","||").replace("^","^^");
        func = func.remove(" ").toLower();
        if( func.startsWith("vo") )
        {
            func = func.replace("vo=", "");
            m_script += "  vo"+n+" = "+func+";\n";
            m_script += "  fu.setOutputVoltage( "+n+", vo"+n+" );\n";
        }else{
            m_script += "  o"+n+" = "+func+";\n";
            m_script += "  fu.setOutputState( "+n+", o"+n+" );\n";
        }
        m_script += "\n";
    }
    m_script += "}\n";
    m_script += "//----------------------;\n";
    /// qDebug() << m_script.toLocal8Bit().data();

    int r = compileScript();
    if( r < 0 ) return;

    m_voltChanged = m_aEngine->GetModule(0)->GetFunctionByDecl("void voltChanged()");
}

void Function::contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu )
{
    menu->addSeparator();
    QAction* loadDaAction = menu->addAction( QIcon(":/load.svg"),tr("Load Functions") );
    connect( loadDaAction, &QAction::triggered,
                     this, &Function::loadData, Qt::UniqueConnection );

    QAction* saveDaAction = menu->addAction(QIcon(":/save.png"), tr("Save Functions") );
    connect( saveDaAction, &QAction::triggered,
                     this, &Function::saveData, Qt::UniqueConnection );
    menu->addSeparator();
    Component::contextMenu( event, menu );
}

void Function::loadData()
{
    QString fileName = QFileDialog::getOpenFileName( 0l, "Function::loadData", m_lastDir, "" );

    if( fileName.isEmpty() ) return; // User cancels loading
    m_lastDir = fileName;
    QStringList lines = fileToStringList( fileName, "MemData::loadData" );

    int i=0;
    for( QString line : lines )
    {
        if( line.remove(" ").isEmpty() ) continue;
        if( i >= m_funcList.size() ) break;
        m_funcList[i++] = line;
}   }

void Function::saveData()
{
    QString fileName = QFileDialog::getSaveFileName( 0l, "Function::saveData", m_lastDir, "" );

    if( fileName.isEmpty() ) return; // User cancels saving
    m_lastDir = fileName;
    QFile outFile( fileName );
    QString output = "";
    for( QString func : m_funcList ) output.append( func+"\n");

    if( !outFile.open( QFile::WriteOnly | QFile::Text ) )
    {
          MessageBoxNB( "MemData::saveData",
                        tr("Cannot write file %1:\n%2.").arg(fileName).arg(outFile.errorString()));
    }else {
        QTextStream toFile( &outFile );
        toFile << output;
        outFile.close();
}   }

void Function::remove()
{
    for( QPushButton* button : m_buttons ) 
    {
       m_buttons.removeOne( button );
       delete button;
    }
    IoComponent::remove();
}

void Function::updateArea( uint ins, uint outs )
{
    uint inSize = ins+1;
    m_height = outs*2;
    if( inSize > m_height ) m_height = inSize;
    int halfH = (m_height/2)*8;
    m_area = QRect(-16,-halfH, m_width*8, m_height*8 );
}

void Function::setNumInputs( int inputs )
{
    if( (uint)inputs == m_inPin.size() ) return;
    if( inputs < 1 ) return;

    m_height = m_outPin.size()*2-1;
    if((uint) inputs > m_height ) m_height = inputs;
    
    IoComponent::setNumInps( inputs, "I" );

    updateArea( inputs, m_outPin.size() );
}

void Function::setNumOutputs( int outs )
{
    if( (uint)outs == m_outPin.size() ) return;
    if( outs < 1 ) return;
    
    updateArea( m_inPin.size(), outs );
    int halfH = (m_height/2)*8;

    uint oldSize = m_outPin.size();
    if( (uint)outs < oldSize )
    {
        int dif = m_outPin.size()-outs;

        IoComponent::deletePins( &m_outPin, dif );

        for( uint i=0; i<oldSize; ++i )
        {
            if( i < (uint)outs )
            {
                m_outPin[i]->setY( -halfH+(int)i*16+8 );
                m_proxys.at(i)->setPos( QPoint( 0, -halfH+(int)i*16+1 ) );
            }else{
                QPushButton* button = m_buttons.takeLast();
                disconnect( button, &QPushButton::released, this, &Function::onbuttonclicked );
                delete button;

                m_proxys.removeLast();
                m_funcList.removeLast();
    }   }   }
    else{
        m_outPin.resize( outs );

        for( uint i=0; i<(uint)outs; ++i )
        {
            if( i<oldSize )
            {
                m_outPin[i]->setY( -halfH+(int)i*16+8 );
                m_proxys.at(i)->setPos( QPoint( 0, -halfH+i*16+1 ) );
            }else{
                QString num = QString::number(i);
                m_outPin[i] = new IoPin( 0, QPoint(24, -halfH+i*16+8 ), m_id+"-out"+num, i, this, output );

                QPushButton* button = new QPushButton( );
                button->setMaximumSize( 14,14 );
                button->setGeometry(-14,-14,14,14);
                QFont font = button->font();
                font.setPixelSize(7);
                button->setFont(font);
                button->setText( "O"+num );
                button->setCheckable( true );
                m_buttons.append( button );

                QGraphicsProxyWidget* proxy = Circuit::self()->addWidget( button );
                proxy->setParentItem( this );
                proxy->setPos( QPoint( 0, -halfH+i*16+1 ) );

                m_proxys.append( proxy );
                m_funcList.append( "" );

                connect( button, &QPushButton::released,
                           this, &Function::onbuttonclicked, Qt::UniqueConnection );
    }   }   }
    
    Circuit::self()->update();
}

void Function::onbuttonclicked()
{
    int i = 0;
    for( QPushButton* button : m_buttons ){
       if( button->isChecked()  ){ button->setChecked( false ); break; }
       ++i;
    }
    bool ok;
    QString text = QInputDialog::getText(0l, tr("Set Function"),
                                             "Output "+QString::number(i)+tr(" Function:"),
                                             QLineEdit::Normal,
                                             m_funcList[i], &ok);
    if( ok ){
        m_funcList[i] = text;
        updateFunctions();
}   }
