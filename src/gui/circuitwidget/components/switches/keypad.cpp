/***************************************************************************
 *   Copyright (C) 2018 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "keypad.h"
#include "circuitwidget.h"
#include "simulator.h"
#include "circuit.h"
#include "itemlibrary.h"
#include "e-node.h"

#include "stringprop.h"
#include "intprop.h"

Component* KeyPad::construct( QObject* parent, QString type, QString id )
{ return new KeyPad( parent, type, id ); }

LibraryItem* KeyPad::libraryItem()
{
    return new LibraryItem(
        tr( "KeyPad" ),
        "Switches",
        "keypad.png",
        "KeyPad",
        KeyPad::construct);
}

KeyPad::KeyPad( QObject* parent, QString type, QString id )
      : Component( parent, type, id )
      , eElement( id )
{
    m_graphical = true;
    m_keyLabels = "123456789*0#";
    m_rows = 4;
    m_cols = 3;
    setupButtons();
    setLabelPos(-8,-16, 0);

    addPropGroup( { tr("Main"), {
new IntProp<KeyPad>("Rows"      , tr("Rows")      ,tr("_Buttons"), this, &KeyPad::rows,      &KeyPad::setRows , propNoCopy,"uint" ),
new IntProp<KeyPad>("Cols"      , tr("Columns")   ,tr("_Buttons"), this, &KeyPad::cols,      &KeyPad::setCols , propNoCopy,"uint" ),
new StrProp<KeyPad>("Key_Labels", tr("Key Labels"),""            , this, &KeyPad::keyLabels, &KeyPad::setKeyLabels )
    },0} );
}
KeyPad::~KeyPad(){}

void KeyPad::stamp()
{
    for( int row=0; row<m_rows; row++ )
    {
        Pin* rowPin = m_pin[row];
        eNode* rowNode = rowPin->getEnode();
        if( rowNode ) rowNode->setSwitched( true );
        
        for( int col=0; col<m_cols; col++ )
        {
            Pin* colPin = m_pin[m_rows+col];
            eNode* colNode = colPin->getEnode();
            if( colNode ) colNode->setSwitched( true );
            
            PushBase* button = m_buttons.at( row*m_cols+col );
            
            ePin* epin0 = button->getEpin( 0 );
            epin0->setEnode( rowNode );
            
            ePin* epin1 = button->getEpin( 1 );
            epin1->setEnode( colNode );
}   }   }

void KeyPad::setupButtons()
{
    if( Simulator::self()->isRunning() )  CircuitWidget::self()->powerCircOff();
    
    m_area = QRectF(-12,-4, 16*m_cols+8, 16*m_rows+8 );
    
    for( PushBase* button : m_buttons ) 
    {
       m_buttons.removeOne( button );
       Circuit::self()->removeComp( button );
    }
    for( Pin* pin : m_pin ) 
    {
        pin->removeConnector();
        if( pin->scene() ) Circuit::self()->removeItem( pin );
        m_signalPin.removeAll( pin );
        delete pin;
    }
    m_pin.resize( m_rows + m_cols );
    
    int labelMax = m_keyLabels.size()-1;
    
    for( int row=0; row<m_rows; row++ )
    {
        QPoint pinPos = QPoint( m_cols*16, 8+row*16 );
        Pin* pin = new Pin( 0, pinPos, m_id+"-Pin"+QString::number(row), 0, this);
        pin->setLength( 4 );
        m_pin[row] = pin;
        
        for( int col=0; col<m_cols; col++ )
        {
            QString butId = m_id+"button"+QString::number(row)+QString::number(col);
            PushBase* button = new PushBase( this, "PushBase", butId );
            button->SetupButton();
            button->setParentItem( this );
            button->setPos( QPointF(col*16+12, 16+row*16 ) );
            button->setFlag( QGraphicsItem::ItemIsSelectable, false );
            m_buttons.append( button );
            
            int pos = row*m_cols+col;
            QString buttonLabel = "";
            if( pos <= labelMax ) buttonLabel = m_keyLabels.mid( pos, 1 );
            button->setKey( buttonLabel );

            if( row == 0 )
            {
                int index = m_rows+col;
                QPoint pinPos = QPoint( col*16, m_rows*16+8);
                Pin* pin = new Pin( 270, pinPos, m_id+"-Pin"+QString::number(index), 0, this);
                pin->setLength( 4 );
                m_pin[index] = pin;
            }
            Circuit::self()->update();
    }   }
    setflip();
}

void KeyPad::setRows( int rows )
{
    if( rows < 1 ) rows = 1;
    m_rows = rows;
    setupButtons();
}

void KeyPad::setCols( int cols )
{
    if( cols < 1 ) cols = 1;
    m_cols = cols;
    setupButtons();
}

void KeyPad::setKeyLabels( QString keyLabels )
{
    m_keyLabels = keyLabels;
    int labelMax = m_keyLabels.size()-1;
    
    for( int row=0; row<m_rows; row++ ){
        for( int col=0; col<m_cols; col++ )
        {
            PushBase* button = m_buttons.at( row*m_cols+col );
            
            int pos = row*m_cols+col;
            QString buttonLabel = "";
            if( pos <= labelMax ) buttonLabel = m_keyLabels.mid( pos, 1 );
            button->setKey( buttonLabel );
}   }   }

void KeyPad::remove()
{
    for( PushBase* button : m_buttons ) 
        Circuit::self()->removeComp( button );

    Component::remove();
}

void KeyPad::setflip()
{
    int x = -16;
    int y = -8;
    int angH = 180;
    int angV = 90;

    if( m_Hflip > 0 )
    {
        x = m_cols*16;
        angH = 0;
    }
    if( m_Vflip > 0 )
    {
        y = m_rows*16+8;
        angV = 270;
    }
    for( int row=0; row<m_rows; row++ )
    {
        m_pin[row]->setPinAngle( angH );
        m_pin[row]->setPos( x, m_pin[row]->pos().y() );
    }
    for( int col=0; col<m_cols; col++ )
    {
        m_pin[m_rows+col]->setPinAngle( angV );
        m_pin[m_rows+col]->setPos( m_pin[m_rows+col]->pos().x(), y );
    }
}

void KeyPad::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );
    p->setBrush( QColor( 240, 240, 240 ) );
    p->drawRoundedRect( m_area,2,2 );
}
