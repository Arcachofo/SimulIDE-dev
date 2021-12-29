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

#include <QMenu>
#include <QFileDialog>
#include <QPushButton>
#include <QInputDialog>
#include <QGraphicsProxyWidget>

#include "function.h"
#include "connector.h"
#include "circuit.h"
#include "simulator.h"
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
        tr( "Function" ),
        tr( "Logic/Arithmetic" ),
        "subc.png",
        "Function",
        Function::construct );
}

Function::Function( QObject* parent, QString type, QString id )
        : LogicComponent( parent, type, id )
        , m_engine()
        , m_functions()
{
    m_lastDir = Circuit::self()->getFilePath();
    
    m_width = 4;
    setNumInps( 2 );                           // Create Input Pins
    setNumOuts( 1 );
    
    setFunctions( "i0 | i1" );

    addPropGroup( { tr("Main"), {
new IntProp   <Function>( "Num_Inputs" , tr("Input Size")     ,"_Pins", this, &Function::numInps,    &Function::setNumInps, "uint" ),
new IntProp   <Function>( "Num_Outputs", tr("Output Size")    ,"_Pins", this, &Function::numOuts,    &Function::setNumOuts, "uint" ),
new BoolProp  <Function>( "Inverted"   , tr("Invert Outputs") ,""     , this, &Function::invertOuts, &Function::setInvertOuts ),
new StringProp<Function>( "Functions"  , tr("Functions")      ,""     , this, &Function::functions,  &Function::setFunctions ),
    }} );
    addPropGroup( { tr("Electric"), IoComponent::inputProps()+IoComponent::outputProps() } );
    addPropGroup( { tr("Edges"), IoComponent::edgeProps() } );
}
Function::~Function(){}

void Function::stamp()
{
    LogicComponent::stamp();

    for( uint i=0; i<m_inPin.size(); ++i ) m_inPin[i]->changeCallBack( this );

    m_program.clear();
    for( uint i=0; i<m_outPin.size(); ++i )
    {
        m_program.append( QScriptProgram( m_funcList.at(i) ));
}   }

void Function::runEvent()
{
    for( uint i=0; i<m_outPin.size(); ++i )
    {
        bool state = m_nextOutVal & (1<<i);
        m_outPin[i]->setOutState( state );
    }
    m_outValue = m_nextOutVal;
}

void Function::voltChanged()
{
    uint bit = 0;
    //uint msb = (m_inPin.size()+m_outPin.size())*2-1;
    for( uint i=0; i<m_inPin.size(); ++i )
    {
        bit = m_inPin[i]->getInpState();
        //if( bit ) bits += 1 << (msb-(i*4));
        //else      bits += 1 << (msb-(i*4)-1);
        m_engine.globalObject().setProperty( "i"+QString::number(i), QScriptValue( bit ) );
        m_engine.globalObject().setProperty( "vi"+QString::number(i), QScriptValue( m_inPin[i]->getVolt()) );
    }
    //m_engine.globalObject().setProperty( "inBits", QScriptValue( bits ) );
    //m_engine.globalObject().setProperty( "inputs", QScriptValue( m_inPin.size() ) );

    for( uint i=0; i<m_outPin.size(); ++i )
    {
        bit = m_outPin[i]->getOutState();
        //if( bit ) bits += 1 << (msb-(i*4)-2);
        //else      bits += 1 << (msb-(i*4)-3);
        m_engine.globalObject().setProperty( "o"+QString::number(i), QScriptValue( bit ) );
        m_engine.globalObject().setProperty( "vo"+QString::number(i), QScriptValue( m_outPin[i]->getVolt()) );
    }
    //m_engine.globalObject().setProperty( "bits", QScriptValue( bits ) );
    //m_engine.globalObject().setProperty( "outputs", QScriptValue( m_outPin.size() ) );

    m_nextOutVal = 0;
    for( uint i=0; i<m_outPin.size(); ++i )
    {
        if( i >= m_outPin.size() ) break;
        QString text = m_funcList.at(i).toLower();

        if( text.startsWith( "vo" ) )
        {
            float out = m_engine.evaluate( m_program.at(i) ).toNumber();
            m_outPin[i]->setOutHighV( out );
            m_nextOutVal += 1<<i;
            Simulator::self()->addEvent( m_propDelay, this );
        }
        else {
            bool out = m_engine.evaluate( m_program.at(i) ).toBool();
            m_outPin[i]->setOutHighV( m_ouHighV );
            if( out ) m_nextOutVal += 1<<i;
            sheduleOutPuts( this );
}   }   }

void Function::setFunctions( QString f )
{
    if( f.isEmpty() ) return;
    m_functions = f;
    m_funcList = f.split(",");
}

void Function::contextMenuEvent( QGraphicsSceneContextMenuEvent* event )
{
    if( !acceptedMouseButtons() ) event->ignore();
    else
    {
        event->accept();
        QMenu* menu = new QMenu();
        contextMenu( event, menu );
        Component::contextMenu( event, menu );
        menu->deleteLater();
}   }

void Function::contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu )
{
    menu->addSeparator();
    QAction* loadDaAction = menu->addAction( QIcon(":/load.png"),tr("Load Functions") );
    connect( loadDaAction, SIGNAL(triggered()),
                     this, SLOT(loadData()), Qt::UniqueConnection );

    QAction* saveDaAction = menu->addAction(QIcon(":/save.png"), tr("Save Functions") );
    connect( saveDaAction, SIGNAL(triggered()),
                     this, SLOT(saveData()), Qt::UniqueConnection );
    menu->addSeparator();
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
        m_functions = m_funcList.join(",");
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
                         tr( "MemData", "Cannot write file %1:\n%2.").arg(fileName).arg(outFile.errorString()));
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
    LogicComponent::remove();
}

void Function::updateArea( uint ins, uint outs )
{
    uint inSize = ins+1;
    m_height = outs*2;
    if( inSize > m_height ) m_height = inSize;
    int halfH = (m_height/2)*8;
    m_area = QRect(-16,-halfH, m_width*8, m_height*8 );
}

void Function::setNumInps( int inputs )
{
    if( (uint)inputs == m_inPin.size() ) return;
    if( inputs < 1 ) return;

    m_height = m_outPin.size()*2-1;
    if((uint) inputs > m_height ) m_height = inputs;
    
    IoComponent::setNumInps( inputs, "I" );

    updateArea( inputs, m_outPin.size() );
}

void Function::setNumOuts( int outs )
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
            }
            else
            {
                QPushButton* button = m_buttons.takeLast();
                disconnect( button, SIGNAL( released() ), this, SLOT  ( onbuttonclicked() ));
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

                connect( button, SIGNAL( released() ),
                           this, SLOT  ( onbuttonclicked() ), Qt::UniqueConnection );
    }   }   }
    m_functions = m_funcList.join(",");
    
    Circuit::self()->update();
}

void Function::onbuttonclicked()
{
    int i = 0;
    for( QPushButton* button : m_buttons ){
       if( button->isChecked()  ){
           button->setChecked( false );
           break;
       }
       ++i;
    }
    bool ok;
    QString text = QInputDialog::getText(0l, tr("Set Function"),
                                             "Output "+QString::number(i)+tr(" Function:"), 
                                             QLineEdit::Normal,
                                             m_funcList[i], &ok);
    if( ok && !text.isEmpty() )
    {
        m_funcList[i] = text;
        m_functions = m_funcList.join(",");
}   }

#include "moc_function.cpp"
