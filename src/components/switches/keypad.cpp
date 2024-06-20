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
#include "boolprop.h"

#define tr(str) simulideTr("KeyPad",str)

Component* KeyPad::construct( QString type, QString id )
{ return new KeyPad( type, id ); }

LibraryItem* KeyPad::libraryItem()
{
    return new LibraryItem(
        tr("KeyPad"),
        "Switches",
        "keypad.png",
        "KeyPad",
        KeyPad::construct);
}

KeyPad::KeyPad( QString type, QString id )
      : Component( type, id )
      , eElement( id )
{
    m_graphical = true;
    m_keyLabels = "123456789*0#";
    m_rows = 4;
    m_cols = 3;
	m_has_diodes=false;
	m_direction=false;
    setupButtons(0,0);
    setLabelPos(-8,-16, 0);

    addPropGroup( { tr("Main"), {
        new BoolProp<KeyPad>("Diodes", tr("Diodes"), ""
                          , this, &KeyPad::has_diodes, &KeyPad::set_has_diodes ),
        
		new BoolProp<KeyPad>("Dir", tr("Diodes Direction"), ""
                          , this, &KeyPad::direction, &KeyPad::setDirection ),
        
		new IntProp<KeyPad>("Rows", tr("Rows"), ""
                           , this, &KeyPad::rows, &KeyPad::setRows, propNoCopy,"uint" ),

        new IntProp<KeyPad>("Cols", tr("Columns"), ""
                           , this, &KeyPad::cols, &KeyPad::setCols, propNoCopy,"uint" ),

        new StrProp<KeyPad>("Key_Labels", tr("Key Labels"), ""
                           , this, &KeyPad::keyLabels, &KeyPad::setKeyLabels )
        
    },0} );
}
KeyPad::~KeyPad(){}
void KeyPad::initialize(){
		m_enodes.clear();
		
		for (int row = 0 ; row < m_rows; row ++){
			for (int col = 0 ; col < m_cols; col ++){

			QString nodId = m_id+"eNode"+QString::number(row)+QString::number(col);
           	eNode* enode = new eNode(nodId);	 
			m_enodes.append(enode);	
			}
		}

}
void KeyPad::stamp()
{
    for( int row=0; row<m_rows; row++ )
    {
        Pin* rowPin = m_pin[row];
        eNode* rowNode = rowPin->getEnode();
        
        for( int col=0; col<m_cols; col++ )
        {
            Pin* colPin = m_pin[m_rows+col];
            eNode* colNode = colPin->getEnode();
            
            PushBase* button = m_buttons.at( row*m_cols+col );
			eDiode* diode = m_diodes.at( row*m_cols+col );
           
		   	eNode* node = m_enodes.at(row*m_cols+col);	

			ePin* buttonEpin0 = button->getEpin( 0 );
			ePin* buttonEpin1 = button->getEpin( 1 );
			if (m_has_diodes)	{
					ePin* diodeEpin0 = diode->getEpin( 0 );
					ePin* diodeEpin1 = diode->getEpin( 1 );

					buttonEpin1->setEnode( node );
					diodeEpin0->setEnode( node );
					
					if (m_direction)	{
							buttonEpin0->setEnode( colNode );
							diodeEpin1->setEnode( rowNode);
					}else{
							buttonEpin0->setEnode( rowNode );
							diodeEpin1->setEnode( colNode);
					}
			} else 
			{
            	buttonEpin0->setEnode( rowNode );
            	buttonEpin1->setEnode( colNode );
			}
		}
	}
}

void KeyPad::set_has_diodes(bool d){
	m_has_diodes=d;
	setupButtons(m_rows,m_cols);
}


void KeyPad::setupButtons(int prev_rows, int prev_cols)
{
    if( Simulator::self()->isRunning() )  CircuitWidget::self()->powerCircOff();
    
    m_area = QRectF(-12,-4, 16*m_cols+8, 16*m_rows+8 );
    
    for( PushBase* button : m_buttons ) 
    {
       m_buttons.removeOne( button );
       delete button;
    }
    for( eDiode* diode : m_diodes ) 
    {
       m_diodes.removeOne( diode );
       delete diode;
    }
	
	for (int col = m_cols; col < prev_cols; col ++ )
	{
		deletePin(m_pin.end()[-1]);
		m_pin.pop_back();
	}
	if (prev_rows > m_rows)
	{
		for (int row = m_rows; row < prev_rows; row ++ )deletePin(m_pin[row]);
		m_pin.erase(m_pin.begin()+m_rows,m_pin.begin()+prev_rows);
	}
    m_pin.resize( m_rows + m_cols );
	
	int n_added_rows=m_rows-prev_rows;
	if (n_added_rows > 0 )
			for (int i = m_pin.size()-1; i >= m_pin.size()-m_cols;i--)
	{
		m_pin[i] = m_pin[i-n_added_rows];
	}
    
    int labelMax = m_keyLabels.size()-1;
    
    for( int row=0; row<m_rows; row++ )
    {
		if (row >= prev_rows){
        	QPoint pinPos = QPoint( m_cols*16, 8+row*16 );
			Pin* pin = new Pin( 0, pinPos, m_id+"-Pin"+QString::number(row), 0, this);
			pin->setLength( 4 );
			m_pin[row] = pin;
		} 
        for( int col=0; col<m_cols; col++ )
        {
            QString butId = m_id+"button"+QString::number(row)+QString::number(col);
            PushBase* button = new PushBase( "PushBase", butId );
            
            button->SetupButton();
            button->setParentItem( this );
            button->setPos( QPointF(col*16+12, 16+row*16 ) );
            button->setFlag( QGraphicsItem::ItemIsSelectable, false );
            m_buttons.append( button );
            
			
			if (m_has_diodes){	
				QString dioId = m_id+"diode"+QString::number(row)+QString::number(col);
				eDiode* diode = new eDiode (dioId);
				diode->setModel( "Diode Default" );
				diode->setNumEpins( 2 );
				m_diodes.append( diode );
			}	
			
			
			
			int pos = row*m_cols+col;
            QString buttonLabel = "";
            if( pos <= labelMax ) buttonLabel = m_keyLabels.mid( pos, 1 );
            button->setKey( buttonLabel );

            if( row == 0 )
            {
				if (col >= prev_cols){
					int index = m_rows+col;
					QPoint pinPos = QPoint( col*16, m_rows*16+8);
					Pin* pin = new Pin( 270, pinPos, m_id+"-Pin"+QString::number(index), 0, this);
					pin->setLength( 4 );
					m_pin[index] = pin;
				}
            }
            Circuit::self()->update();
    }   }
    setflip();
}
void KeyPad::setDirection(bool dir){ 
	m_direction=dir; 
	setupButtons(m_rows,m_cols);
}

void KeyPad::setRows( int rows )
{
    if( rows < 1 ) rows = 1;
    int prev_rows = m_rows;
	m_rows = rows;
    setupButtons(prev_rows,m_cols);
}

void KeyPad::setCols( int cols )
{
    if( cols < 1 ) cols = 1;
	int prev_cols = m_cols;
    m_cols = cols;
    setupButtons(m_rows, prev_cols);
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
    for( PushBase* button : m_buttons ) delete button;
    for( eDiode* diode : m_diodes ) delete diode;
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
    p->setBrush( QColor(50, 70, 100) );
    p->drawRoundedRect( m_area,2,2 );

   	if (m_has_diodes){ 
		if (m_direction){
			int y_pos = (m_Vflip > 0) ? 8+m_rows*16 : -8;
			int y_pos2 = (m_Vflip > 0) ? y_pos-3 : y_pos+3;
			
			for (int col = 0 ; col < m_cols; col ++){	
				QPointF points[3] = {
				QPointF( col*16  -2, y_pos),
				QPointF( col*16  ,   y_pos2),
				QPointF( col*16  +2, y_pos)     
				};
				p->drawPolygon(points, 3);
			}
		} else {
			int x_pos = (m_Hflip > 0) ?  m_cols*16    : -16;
			int x_pos2 = (m_Hflip > 0) ? x_pos-3 : x_pos+3;
			for (int row = 0 ; row < m_rows; row ++){	
				QPointF points[3] = {
				QPointF( x_pos ,  8+row*16-2),
				QPointF( x_pos2,  8+row*16),
				QPointF( x_pos ,  8+row*16+2)     
				};
				p->drawPolygon(points, 3);
			}
		}
	}
    Component::paintSelected( p );
}
