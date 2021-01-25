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

#include "scripted.h"
#include "connector.h"
#include "circuit.h"
#include "itemlibrary.h"
#include "utils.h"

static const char* Scripted_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","Functions")
};

Component* Scripted::construct( QObject* parent, QString type, QString id )
{
    return new Scripted( parent, type, id );
}

LibraryItem* Scripted::libraryItem()
{
    return new LibraryItem(
    tr( "Scripted" ),
    tr( "Logic/Arithmetic" ),
    "subc.png",
    "Scripted",
    Scripted::construct );
}

Scripted::Scripted( QObject* parent, QString type, QString id )
        : LogicComponent( parent, type, id )
        , eLogicDevice( id )
{
    Q_UNUSED( Scripted_properties );

    m_lastDir = Circuit::self()->getFileName();
    
    setNumInps( 2 );                           // Create Input Pins
    setNumOuts( 1 );

    m_thisObject = m_engine.newQObject( this );
    m_engine.globalObject().setProperty( "component", m_thisObject );

    //setScript( "component->m_output[0]->setTimedOut( true ); return true;" );
}
Scripted::~Scripted(){}

QList<propGroup_t> Scripted::propGroups()
{
    propGroup_t mainGroup { tr("Main") };
    mainGroup.propList.append( {"Num_Inputs", tr("Input Size"),"Inputs"} );
    mainGroup.propList.append( {"Num_Outputs", tr("Output Size"),"Outputs"} );

    propGroup_t scriptGroup { tr("Script") };
    scriptGroup.propList.append( {"Script", tr("Script Editor"),"textSave"} );
    scriptGroup.propList.append( {"Evaluate", tr("Evaluate Script"),"textEvaluate"} );

    QList<propGroup_t> pg = LogicComponent::propGroups();
    pg.prepend( scriptGroup );
    pg.prepend( mainGroup );
    return pg;
}

void Scripted::stamp()
{
    m_canRun = false;
    eLogicDevice::stamp();

    QString excep = evaluate( m_script );
    if( excep != "" ) return;
    m_canRun = true;

    for( int i=0; i<m_numInputs; ++i )
    {
        eNode* enode = m_input[i]->getEpin(0)->getEnode();
        if( enode ) enode->voltChangedCallback( this );
    }
}

void Scripted::voltChanged()
{
    if( !m_canRun ) return;

    m_voltChanged.call( m_thisObject );
    //if( getExceptions() ) return;

    ///Simulator::self()->addEvent( m_propDelay, this );
}

QString Scripted::evaluate( QString script )
{
    if( !m_engine.canEvaluate( script ) ) return "Error canEvaluate returned false!";

    m_program = QScriptProgram( script );

    m_engine.evaluate( m_program );
    QString excep = getExceptions();
    if( excep != "" ) return excep;

    m_voltChanged = m_engine.evaluate( "voltChanged" );
    excep = getExceptions();
    if( excep != "" ) return excep;

    if (!m_voltChanged.isFunction()) return "voltChanged: Not a function:\n";

    m_voltChanged.call( m_thisObject );
    excep = getExceptions();
    if( excep != "" ) return excep;

    return "";
}

QString Scripted::getExceptions()
{
    if( m_engine.hasUncaughtException() )
    {
        QScriptValue exception = m_engine.uncaughtException();
        return exception.toString();
    }
    return "";
}

void Scripted::runEvent()
{
    /*for( int i=0; i<m_numOutputs; ++i )
    {
        if( i >= m_numOutputs ) break;
        QString text = m_funcList.at(i).toLower();

        //qDebug() << "eScripted::voltChanged()"<<text<<m_engine.evaluate( text ).toString();

        if( text.startsWith( "vo" ) )
        {
            float out = m_engine.evaluate( m_program.at(i) ).toNumber();
            m_output[i]->setVoltHigh( out );
            m_output[i]->setOut( true );
            m_output[i]->stampOutput();

        }
        else
        {
            bool out = m_engine.evaluate( m_program.at(i) ).toBool();
            m_output[i]->setTimedOut( out );
        }
        //qDebug()<<"Func:"<< i << text; //textLabel->setText(text);
        //qDebug() << ":" << out;
        //qDebug() << m_engine.globalObject().property("i0").toVariant() << m_engine.globalObject().property("i1").toVariant();
    }*/
}

void Scripted::contextMenuEvent( QGraphicsSceneContextMenuEvent* event )
{
    if( !acceptedMouseButtons() ) event->ignore();
    else
    {
        event->accept();
        QMenu* menu = new QMenu();
        contextMenu( event, menu );
        Component::contextMenu( event, menu );
        menu->deleteLater();
    }
}

void Scripted::contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu )
{
    menu->addSeparator();
    QAction* loadDaAction = menu->addAction( QIcon(":/load.png"),tr("Load script") );
    connect( loadDaAction, SIGNAL(triggered()),
                     this, SLOT(loadScript()), Qt::UniqueConnection );

    QAction* saveDaAction = menu->addAction(QIcon(":/save.png"), tr("Save script") );
    connect( saveDaAction, SIGNAL(triggered()),
                     this, SLOT(saveScript()), Qt::UniqueConnection );
    menu->addSeparator();
}

void Scripted::loadScript()
{
    QString fileName = QFileDialog::getOpenFileName( 0l, "Scripted::loadData", m_lastDir, "" );

    if( fileName.isEmpty() ) return; // User cancels loading
    m_lastDir = fileName;
    m_script = fileToString( fileName, "Scripted::loadData" );
}

void Scripted::saveScript()
{
    QString fileName = QFileDialog::getSaveFileName( 0l, "Scripted::saveData", m_lastDir, "" );

    if( fileName.isEmpty() ) return; // User cancels saving
    m_lastDir = fileName;
    QFile outFile( fileName );

    if( !outFile.open( QFile::WriteOnly | QFile::Text ) )
    {
          QMessageBox::warning( 0l, "Scripted::saveData",
           "Scripted", tr("Cannot write file %1:\n%2.").arg(fileName).arg(outFile.errorString()));
    }else {
        QTextStream toFile( &outFile );
        toFile << m_script;
        outFile.close();
    }
}

void Scripted::setNumInps( int inputs )
{
    if( inputs == m_numInputs ) return;
    if( inputs < 1 ) return;
    
    if( inputs < m_numInputs ) 
    {
        int dif = m_numInputs-inputs;
        
        eLogicDevice::deleteInputs( dif );
        LogicComponent::deleteInputs( dif );
    }
    else{
        m_inPin.resize( inputs );
        m_numInPins = inputs;
    
        for( int i=m_numInputs; i<inputs; ++i )
        {
            QString num = QString::number(i);
            m_inPin[i] = new Pin( 180, QPoint(-24, i*8+8 ), m_id+"-in"+num, i, this );
            m_inPin[i]->setLabelText( " I"+num );
            m_inPin[i]->setLabelColor( QColor( 0, 0, 0 ) );

            eLogicDevice::createInput( m_inPin[i] );
        }
    }
    m_height = m_numOutputs;
    if( m_numInputs > m_height ) m_height = m_numInputs;
    m_area = QRect( -16, 0, 32, 8*m_height+8 );
    
    Circuit::self()->update();
}

void Scripted::setNumOuts( int outs )
{
    if( outs == m_numOutputs ) return;
    if( outs < 1 ) return;
    
    if( outs < m_numOutputs ) 
    {
        int dif = m_numOutputs-outs;
        
        eLogicDevice::deleteOutputs( dif );
        LogicComponent::deleteOutputs( dif );
    }
    else{
        m_outPin.resize( outs );
        m_numOutPins = outs;
        
        for( int i=m_numOutputs; i<outs; ++i )
        {
            QString num = QString::number(i);
            m_outPin[i] = new Pin( 0, QPoint(24, i*8+8 ), m_id+"-out"+num, i, this );
            m_outPin[i]->setLabelText( "O"+num+" " );
            m_outPin[i]->setLabelColor( QColor( 0, 0, 0 ) );

            eLogicDevice::createOutput( m_outPin[i] );
        }
    }
    m_height = m_numOutputs;
    if( m_numInputs > m_height ) m_height = m_numInputs;
    m_area = QRect( -16, 0, 32, 8*m_height+8 );

    Circuit::self()->update();
}


#include "moc_scripted.cpp"
