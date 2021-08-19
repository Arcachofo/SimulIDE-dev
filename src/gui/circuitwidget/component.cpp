/***************************************************************************
 *   Copyright (C) 2012 by santiago González                               *
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

#include <math.h>

#include "component.h"
#include "mainwindow.h"
#include "connector.h"
#include "connectorline.h"
#include "circuitwidget.h"
#include "itemlibrary.h"
#include "circuit.h"
#include "utils.h"
#include "simuapi_apppath.h"
#include "propdialog.h"

int Component::m_error = 0;
bool Component::m_selMainCo = false;

static const char* Component_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","id"),
    QT_TRANSLATE_NOOP("App::Property","Show id"),
    QT_TRANSLATE_NOOP("App::Property","Unit"),
    QT_TRANSLATE_NOOP("App::Property","Color")
};

Component::Component( QObject* parent, QString type, QString id )
         : QObject( parent )
         , QGraphicsItem()
         , multUnits( "TGMk mµnp" )
{
    Q_UNUSED( Component_properties );
    //setCacheMode(QGraphicsItem::DeviceCoordinateCache);

    m_help = "";
    m_value    = 0;
    m_unitMult = 1;
    m_Hflip  = 1;
    m_Vflip  = 1;
    m_mult   = " ";
    m_unit   = " ";
    m_type   = type;
    m_color  = QColor( Qt::white );

    m_showId     = false;
    m_moving     = false;
    m_printable  = false;
    m_properties = false;
    m_hidden     = false;
    m_graphical  = false;
    m_mainComp   = false;
    m_BackGround = "";

    m_propDialog = NULL;

    m_boardPos = QPointF( -1e6, -1e6 );
    m_boardRot = -1e6;

    QFont f;
    f.setPixelSize(10);
    
    m_idLabel = new Label();
    m_idLabel->setComponent( this );
    m_idLabel->setDefaultTextColor( Qt::darkBlue );
    m_idLabel->setFont(f);
    setLabelPos(-16,-24, 0);
    setShowId( false );
    
    m_valLabel = new Label();
    m_valLabel->setComponent( this );
    m_valLabel->setDefaultTextColor( Qt::black );
    setValLabelPos( 0, 0, 0);
    f.setPixelSize(9);
    m_valLabel->setFont(f);
    setShowVal( false );
    
    setObjectName( id );
    setIdLabel( id );
    setId(id);

    setCursor( Qt::OpenHandCursor );
    this->setFlag( QGraphicsItem::ItemIsSelectable, true );

    if     ( type == "Connector" )  Circuit::self()->conList()->append( this );
    else if( type == "SerialPort" ) Circuit::self()->compList()->append( this );
    else if( type == "SerialTerm" ) Circuit::self()->compList()->append( this );
    else                            Circuit::self()->compList()->prepend( this );
}
Component::~Component(){}

QStringList Component::userProperties()
{
    QStringList userProps;
    QList<propGroup_t> pgs = propGroups();
    for( propGroup_t pg : pgs )
    {
        for( property_t prop : pg.propList ) userProps.append( prop.name );
    }
    return userProps;
}

void Component::mousePressEvent( QGraphicsSceneMouseEvent* event )
{
    if( this->parentItem() )
    {
        Component* parentComp = static_cast<Component*>( this->parentItem() );
        parentComp->mousePressEvent( event );
        return;
    }
    if( event->button() == Qt::LeftButton )
    {
        event->accept();
        if( m_selMainCo )  // Used when creating Boards to set this as main component
        {
            if( m_mainComp ) m_mainComp = false;
            else
            {
                QList<Component*>* compList = Circuit::self()->compList();
                for( Component* comp : *compList ) comp->m_mainComp = false;
                m_selMainCo = false;
                m_mainComp  = true;
            }
            update();
            return;
        }
        if( event->modifiers() == Qt::ControlModifier ) setSelected( !isSelected() );
        else
        {
            QList<QGraphicsItem*> itemlist = Circuit::self()->selectedItems();
            if( !isSelected() )     // Unselect everything and select this
            {
                for( QGraphicsItem* item : itemlist ) item->setSelected( false );
                setSelected( true );
            }
            else                    // Deselect childs
            {
                for( QGraphicsItem* item : itemlist )
                {
                    QList<QGraphicsItem*> childs = item->childItems();
                    for( QGraphicsItem* child : childs ) child->setSelected( false );
            }   }
            setCursor( Qt::ClosedHandCursor );
        }
        QApplication::focusWidget()->clearFocus();
        CircuitView::self()->setFocus();
}   }

void Component::mouseMoveEvent( QGraphicsSceneMouseEvent* event )
{
    if( this->parentItem() )
    {
        Component* parentComp = static_cast<Component*>( this->parentItem() );
        parentComp->mouseMoveEvent( event );
        emit moved();
        return;
    }
    event->accept();
    
    QPointF delta = togrid(event->scenePos()) - togrid(event->lastScenePos());
    
    bool deltaH  = fabs( delta.x() )> 0;
    bool deltaV  = fabs( delta.y() )> 0;
    
    if( !deltaH && !deltaV ) return;

    QList<QGraphicsItem*> itemlist = Circuit::self()->selectedItems();

    if( !m_moving )
    {
        Circuit::self()->saveState();
        m_moving = true;
    }

    if( itemlist.size() > 1 )
    {
        for( QGraphicsItem* item : itemlist )
        {
            ConnectorLine* line =  qgraphicsitem_cast<ConnectorLine* >( item );
            if( line->objectName() == "" ) 
            {
                //line->move( delta );
                line->moveSimple( delta );
        }   }
        for( QGraphicsItem* item : itemlist )
        {
            Component* comp =  qgraphicsitem_cast<Component*>( item );
            if(comp && (comp->objectName() != "") && (!comp->objectName().contains("Connector")) )
            {
                comp->move( delta );
        }   }
        for( Component* comp : *(Circuit::self()->conList()) )
        {
            Connector* con = static_cast<Connector*>( comp );
            con->startPin()->isMoved();
            con->endPin()->isMoved();
    }   }
    else this->move( delta );
}

void Component::mouseReleaseEvent( QGraphicsSceneMouseEvent* event )
{
    if( this->parentItem() )
    {
        Component* parentComp = static_cast<Component*>( this->parentItem() );
        parentComp->mouseReleaseEvent( event );
        return;
    }
    event->accept();
    setCursor( Qt::OpenHandCursor );

    m_moving = false;
    Circuit::self()->update();
}

void Component::contextMenuEvent( QGraphicsSceneContextMenuEvent* event )
{
    if( this->parentItem() )
    {
        Component* parentComp = static_cast<Component*>( this->parentItem() );
        parentComp->contextMenuEvent( event );
        return;
    }
    if( !acceptedMouseButtons() ) event->ignore();
    else
    {
        event->accept();
        QMenu* menu = new QMenu();
        contextMenu( event, menu );
        menu->deleteLater();
}   }

void Component::contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu )
{
    m_eventpoint = mapToScene( togrid(event->pos()) );

    QAction* copyAction = menu->addAction(QIcon(":/copy.png"),tr("Copy")+"\tCtrl+C");
    connect( copyAction, SIGNAL( triggered()),
                   this, SLOT(slotCopy()), Qt::UniqueConnection );

    QAction* removeAction = menu->addAction( QIcon( ":/remove.png"),tr("Remove")+"\tDel" );
    connect( removeAction, SIGNAL( triggered()),
                     this, SLOT(slotRemove()), Qt::UniqueConnection );
    
    QAction* propertiesAction = menu->addAction( QIcon( ":/properties.png"),tr("Properties") );
    connect( propertiesAction, SIGNAL( triggered()),
                         this, SLOT(slotProperties()), Qt::UniqueConnection );
    menu->addSeparator();

    QAction* rotateCWAction = menu->addAction( QIcon( ":/rotateCW.png"),tr("Rotate CW") );
    connect( rotateCWAction, SIGNAL( triggered()),
                       this, SLOT(rotateCW()), Qt::UniqueConnection );

    QAction* rotateCCWAction = menu->addAction(QIcon( ":/rotateCCW.png"),tr("Rotate CCW") );
    connect( rotateCCWAction, SIGNAL( triggered()),
                        this, SLOT(rotateCCW()), Qt::UniqueConnection );

    QAction* rotateHalfAction = menu->addAction(QIcon(":/rotate180.png"),tr("Rotate 180") );
    connect( rotateHalfAction, SIGNAL( triggered()),
                         this, SLOT(rotateHalf()), Qt::UniqueConnection );
    
    QAction* H_flipAction = menu->addAction(QIcon(":/hflip.png"),tr("Horizontal Flip") );
    connect( H_flipAction, SIGNAL( triggered()),
                     this, SLOT(H_flip()), Qt::UniqueConnection );
    
    QAction* V_flipAction = menu->addAction(QIcon(":/vflip.png"),tr("Vertical Flip") );
    connect( V_flipAction, SIGNAL( triggered()),
                     this, SLOT(V_flip()), Qt::UniqueConnection );

    menu->exec(event->screenPos());
}

void Component::slotCopy()
{
    if( !isSelected() ) Circuit::self()->clearSelection();
    setSelected( true );
    Circuit::self()->copy( m_eventpoint );
}

void Component::slotRemove()
{
    if( !isSelected() )
    {
        Circuit::self()->clearSelection();
        setSelected( true );
    }
    Circuit::self()->removeItems();
}

void Component::remove()
{
    if( m_propDialog )
    {
        m_propDialog->setParent( NULL );
        m_propDialog->close();
        delete m_propDialog;
    }
    for( uint i=0; i<m_pin.size(); i++ )
        if( m_pin[i] ) m_pin[i]->removeConnector();

    Circuit::self()->compRemoved( true );
}

void Component::slotProperties()
{
    if( !m_propDialog )
    {
        if(( m_help == "" )&&( m_type != "Connector" )&&( m_type != "Node" ))
        {
            QString name = m_type;

            if( ( m_type == "Subcircuit" )
              ||( m_type == "MCU" )
              ||( m_type == "PIC" ))
            {
                name = m_id.split("-").first();
            }
            m_help = MainWindow::self()->getHelpFile( name );
        }
        m_propDialog = new PropDialog( CircuitWidget::self(), m_help );
        m_propDialog->setComponent( this );
    }
    m_propDialog->show();
}

void Component::H_flip()
{
    Circuit::self()->saveState();
    m_Hflip = -m_Hflip;
    setflip();
}

void Component::V_flip()
{
    Circuit::self()->saveState();
    m_Vflip = -m_Vflip;
    setflip();
}

void Component::rotateCW()
{
    Circuit::self()->saveState();
    setRotation( rotation() + 90 );
    emit moved();
}

void Component::rotateCCW()
{
    Circuit::self()->saveState();
    setRotation( rotation() - 90 );
    emit moved();
}

void Component::rotateHalf()
{
    Circuit::self()->saveState();
    setRotation( rotation() - 180);
    emit moved();
}

void Component::updateLabel( Label* label, QString txt )
{
    if     ( label == m_idLabel ) m_id = txt;
    else if( label == m_valLabel )
    {
        QString value = "";
        int x;
        for( x=0; x<txt.length(); ++x ) 
        {
            QChar atx = txt.at(x);
            if( atx.isDigit() ) value.append( atx );
            else break;
        }
        QString unit = txt.mid( x, txt.length() );
        
        setUnit( unit );
        setValue( value.toDouble() );
}   }

void Component::setLabelPos( int x, int y, int rot )
{
    m_idLabel->m_labelx = x;
    m_idLabel->m_labely = y;
    m_idLabel->m_labelrot = rot;
    m_idLabel->setLabelPos();
}

void Component::setValLabelPos( int x, int y, int rot )
{
    m_valLabel->m_labelx = x;
    m_valLabel->m_labely = y;
    m_valLabel->m_labelrot = rot;
    m_valLabel->setLabelPos();
}

void Component::setValue( double val )
{ 
    m_value = val;
    /*if( fabs(val) < 1e-12 )
    {
        m_value = 0;
        m_mult = " ";
    }
    else
    {
        val = val*m_unitMult;
        
        int index = 4;   // We are in bare units "TGMK munp"
        m_unitMult = 1;
        while( fabs(val) >= 1000 )
        {
            if( --index < 0 ) { index = 0; break; }

            m_unitMult = m_unitMult*1000;
            val = val/1000;
        }
        while( fabs(val) < 1 )
        {
            if( ++index > 8 ) { index = 8; break; }

            m_unitMult = m_unitMult/1000;
            val = val*1000;
        }
        m_mult = multUnits.at( index );
        if( m_mult != " " ) m_mult.prepend( " " );
        m_value = val;
    }*/
    QString valStr = QString::number(m_value);
    //m_valLabel->setPlainText( valStr.left(5)+m_mult+m_unit );
    m_valLabel->setPlainText( valStr+m_mult+m_unit );
}

void Component::setUnit( QString un ) 
{
    QString mul = " ";
    un.replace( " ", "" );
    if( un.size() > 0 ) 
    {
        mul = un.at(0);
        if( mul == "u" ) mul = "µ";
        double unitMult = 1e12;        // We start in Tera units "TGMk munp"
        
        for( int x=0; x<9; x++ )
        {
            if( mul == multUnits.at(x) ) 
            {
                m_unitMult = unitMult;
                m_mult     = mul;
                if( m_mult != " " ) m_mult.prepend( " " );
                m_valLabel->setPlainText( QString::number(m_value)+m_mult+m_unit );
                return;
            }
            unitMult = unitMult/1000;
    }   }
    m_unitMult = 1;
    m_mult     = " ";
    m_valLabel->setPlainText( QString::number(m_value)+m_mult+m_unit );
}

void Component::setHflip( int hf )
{ 
    if(( hf != 1 )&( hf != -1 )) hf = 1;
    m_Hflip = hf;
    setflip();
}

void Component::setVflip( int vf )
{ 
    if(( vf != 1 )&( vf != -1 )) vf = 1;
    m_Vflip = vf; 
    setflip();
}

void Component::setflip()
{
    setTransform(QTransform::fromScale( m_Hflip, m_Vflip ));
    m_idLabel->setTransform(QTransform::fromScale( m_Hflip, m_Vflip ));
    m_valLabel->setTransform(QTransform::fromScale( m_Hflip, m_Vflip ));
    emit moved();
}

void Component::setHidden( bool hid, bool hidLabel )
{
    m_hidden = hid;

    if( m_graphical )
    {
        for( Pin* pin : m_pin ) pin->setVisible( !hid );
    }
    else this->setVisible( !hid );

    if( hidLabel )
    {
        setShowId( false );
        setShowVal( false );
}   }

QString Component::print()
{
    if( !m_printable ) return "";
    
    QString str = m_id+" : ";
    str += objectName().split("-").first()+" ";
    if( m_value > 0 ) str += QString::number( m_value );
    str += m_mult+m_unit+"\n";
    
    return str;
}

void Component::paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    QPen pen( Qt::black, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin );

    QColor color;

    if( isSelected() )
    {
        pen.setColor( Qt::darkGray);
        color = Qt::darkGray;
    }
    else color = m_color;

    if( m_mainComp )
    {
        painter->fillRect( boundingRect(), Qt::yellow  );
        painter->setOpacity( 0.5 );
    }
    painter->setBrush( color );
    painter->setPen( pen );
}

#include "moc_component.cpp"
