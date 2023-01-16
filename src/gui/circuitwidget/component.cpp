/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <math.h>

#include "component.h"
#include "label.h"
#include "mainwindow.h"
#include "connector.h"
#include "connectorline.h"
#include "circuitwidget.h"
#include "circuit.h"
#include "utils.h"
#include "propdialog.h"

#include "doubleprop.h"
#include "boolprop.h"
#include "intprop.h"
#include "stringprop.h"
#include "pointprop.h"

int  Component::m_error = 0;
bool Component::m_selMainCo = false;

Component::Component( QObject* parent, QString type, QString id )
         : CompBase( parent, type, id )
         , QGraphicsItem()
{
    //setCacheMode(QGraphicsItem::DeviceCoordinateCache);

    m_help = "";
    m_Hflip  = 1;
    m_Vflip  = 1;
    m_color  = QColor( Qt::white );

    m_showId     = false;
    m_showVal    = false;
    m_moving     = false;
    m_printable  = false;
    m_isMainComp = false;
    m_hidden     = false;
    m_crashed    = false;
    m_warning    = false;
    m_graphical  = false;
    m_background = "";
    m_showProperty = "";

    m_boardPos = QPointF( -1e6, -1e6 );
    m_boardRot = -1e6;
    m_circPos  = QPointF(0, 0);
    m_circRot  = 0;

    QFont f;
    f.setFamily("Ubuntu");
    f.setPixelSize( 10 );
    
    m_idLabel = new Label();
    m_idLabel->setComponent( this );
    m_idLabel->setDefaultTextColor( Qt::darkBlue );
    m_idLabel->setFont( f );
    setLabelPos(-16,-24, 0 );
    setShowId( false );
    
    m_valLabel = new Label();
    m_valLabel->setComponent( this );
    m_valLabel->setDefaultTextColor( Qt::darkRed );
    setValLabelPos(-16, 20, 0 );
    f.setPixelSize( 8 );
    m_valLabel->setFont( f );
    m_valLabel->setVisible( false );
    
    setObjectName( id );
    if( !id.contains("-") ) id.prepend( type+"-" );
    setIdLabel( id );

    setCursor( Qt::OpenHandCursor );
    setFlag( QGraphicsItem::ItemIsSelectable, true );

    addPropGroup( { "CompBase", {
new StringProp<Component>( "itemtype","","", this, &Component::itemType,  &Component::setItemType ),
new StringProp<Component>( "CircId"  ,"","", this, &Component::getUid,    &Component::setUid ),
new BoolProp  <Component>( "mainComp","","", this, &Component::isMainComp,&Component::setMainComp ),// Related to Subcircuit:
    }} );
    addPropGroup( { "CompGraphic", {
new StringProp<Component>( "ShowProp","","", this, &Component::showProp,  &Component::setShowProp ),
new BoolProp  <Component>( "Show_id" ,"","", this, &Component::showId,    &Component::setShowId ),
new BoolProp<Component>  ( "Show_Val","","", this, &Component::showVal,   &Component::setShowVal ),
//new DoubProp<Component>  ( "Value"   ,"","",this, &Component::getValue,  &Component::setValue ),
//new StringProp<Component>( "Unit"    ,"","",this, &Component::unit,      &Component::setUnit ),
new PointProp <Component>( "Pos"     ,"","", this, &Component::position,  &Component::setPosition ),
new DoubProp  <Component>( "rotation","","", this, &Component::getAngle,  &Component::setAngle ),
new IntProp   <Component>( "hflip"   ,"","", this, &Component::hflip,     &Component::setHflip ),
new IntProp   <Component>( "vflip"   ,"","", this, &Component::vflip,     &Component::setVflip ),
new StringProp<Component>( "label"   ,"","", this, &Component::idLabel,   &Component::setIdLabel ),
new PointProp<Component>( "idLabPos" ,"","", this, &Component::getIdPos,  &Component::setIdPos ),
new IntProp  <Component>( "labelrot" ,"","", this, &Component::getIdRot,  &Component::setIdRot ),
new PointProp<Component>( "valLabPos","","", this, &Component::getValPos, &Component::setValPos ),
new IntProp  <Component>( "valLabRot","","", this, &Component::getValRot, &Component::setValRot ),
    }} );

    addPropGroup( { "Board", {                   // Board properties
new PointProp<Component>( "boardPos", "","", this, &Component::boardPos, &Component::setBoardPos ),
new PointProp<Component>( "circPos" , "","", this, &Component::circPos,  &Component::setCircPos ),
new DoubProp <Component>( "boardRot", "","", this, &Component::boardRot, &Component::setBoardRot ),
new DoubProp <Component>( "circRot" , "","", this, &Component::circRot,  &Component::setCircRot )
    }} );
}
Component::~Component(){}

bool Component::setPropStr( QString prop, QString val )
{
    if     ( prop =="id" )        m_idLabel->setPlainText( val );       // Old: TODELETE
    else if( prop =="objectName") return true;                          // Old: TODELETE

    else if( prop =="x" )         setX( val.toInt() );                  // Old: TODELETE
    else if( prop =="y" )         setY( val.toInt() );                  // Old: TODELETE

    else if( prop =="labelx" )    m_idLabel->m_labelx = val.toInt();    // Old: TODELETE
    else if( prop =="labely" )    m_idLabel->m_labely = val.toInt();    // Old: TODELETE

    else if( prop =="valLabelx" ) m_valLabel->m_labelx = val.toInt();   // Old: TODELETE
    else if( prop =="valLabely" ) m_valLabel->m_labely = val.toInt();   // Old: TODELETE

    //else if( prop =="Show_Val" ) { setShowVal( val=="true" );}          // Old: TODELETE
    else if( prop =="Unit" )                                            // Old: TODELETE
    {
        val = val.remove(" ").replace("u","µ");
        if( !val.isEmpty() )
        {
            ComProperty* p =  m_propHash.value( m_showProperty );
            if( !p ) return true;
            QString un = p->unit();
            if( val == un ) return true;
            QString vstr = p->getValStr().replace( un, val );
            p->setValStr( vstr );
        }
    }
    else return CompBase::setPropStr( prop, val );
    return true;
}

void Component::substitution( QString &propName ) // static
{
    if     ( propName == "Volts"       ) propName = "Voltage";
    //else if( propName == "Current") propName = "Value";
    else if( propName == "id"          ) propName = "label";
    else if( propName == "Duty_Square" ) propName = "Duty";
    else if( propName == "S_R_Inverted") propName = "Reset_Inverted";
    else if( propName == "Show_res"
          || propName == "Show_Volt"
          || propName == "Show_volt"
          || propName == "Show_Amp"
          || propName == "Show_Ind"
          || propName == "Show_Ind"
          || propName == "Show_Cap" )    propName = "Show_Val";
   /// else if( propName == "Inverted")    propName = "InvertOuts";
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
            if( m_isMainComp ) m_isMainComp = false;
            else{
                QList<Component*>* compList = Circuit::self()->compList();
                for( Component* comp : *compList ) comp->m_isMainComp = false;
                m_selMainCo  = false;
                m_isMainComp = true;
            }
            update();
            return;
        }
        if( event->modifiers() == Qt::ControlModifier ) setSelected( !isSelected() );
        else{
            QList<QGraphicsItem*> itemlist = Circuit::self()->selectedItems();
            if( !isSelected() )     // Unselect everything and select this
            {
                for( QGraphicsItem* item : itemlist ) item->setSelected( false );
                setSelected( true );
            }
            else{                   // Deselect childs
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
        if( !m_hidden ) emit moved();
        return;
    }
    event->accept();
    
    QPointF delta = toGrid(event->scenePos()) - toGrid(event->lastScenePos());
    if( !(fabs( delta.x() )> 0) && !(fabs( delta.y() )> 0) ) return;

    if( !m_moving ) // Get lists of elements to move and save Undo state
    {
        Circuit::self()->addCompState( NULL, "", stateNew );
        m_linMoveList.clear();
        m_conMoveList.clear();
        m_compMoveList.clear();
        QList<QGraphicsItem*> itemlist = Circuit::self()->selectedItems();
        for( QGraphicsItem* item : itemlist )
        {
            if( item->type() == UserType+2 ) // ConnectorLine selected
            {
                ConnectorLine* line =  qgraphicsitem_cast<ConnectorLine*>( item );
                m_linMoveList.append( line );
                Connector* con = line->connector();
                if( !m_conMoveList.contains( con ) ) // Connectors selected
                {
                    m_conMoveList.append( con );
                    Circuit::self()->addCompState( con, "pointList", stateAdd );
                }
            }
            else if( item->type() == UserType+1 ) // Component selected
            {
                Component* comp =  qgraphicsitem_cast<Component*>( item );
                Circuit::self()->addCompState( comp, "Pos", stateAdd );
                m_compMoveList.append( comp );
                std::vector<Pin*> pins = comp->getPins();
                for( Pin* pin : pins )
                {
                    if( !pin ) continue;
                    Connector* con = pin->connector();
                    if( !con ) continue;
                    if( !m_conMoveList.contains( con ) ) // Connector attached to selected Component
                    {
                        m_conMoveList.append( con );
                        Circuit::self()->addCompState( con, "pointList", stateAdd );
                    }
                }
            }
        }
        m_moving = true;
    }
    for( ConnectorLine* line : m_linMoveList  ) line->moveSimple( delta ); // Move lines selected
    for( Component*     comp : m_compMoveList ) comp->move( delta );       // Move Components selected
    for( Connector*     con  : m_conMoveList )                             // Update Connectors
    {
        con->startPin()->isMoved();
        con->endPin()->isMoved();
    }
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

    if( m_moving )
    {
        m_moving = false;
        Circuit::self()->saveState();
    }
    Circuit::self()->update();
}

void Component::mouseDoubleClickEvent( QGraphicsSceneMouseEvent* event )
{
    if( this->parentItem() )
    {
        Component* parentComp = static_cast<Component*>( this->parentItem() );
        parentComp->mouseDoubleClickEvent( event );
        return;
    }
    if( event->button() == Qt::LeftButton ) slotProperties();
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
    else{
        event->accept();
        QMenu* menu = new QMenu();
        contextMenu( event, menu );
        menu->deleteLater();
}   }

void Component::contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu )
{
    if( !event && m_isMainComp ) // Main Component in Subcircuit
    {
        QAction* propertiesAction = menu->addAction( QIcon( ":/properties.svg"),tr("Properties") );
        connect( propertiesAction, SIGNAL( triggered()),
                             this, SLOT(slotProperties()), Qt::UniqueConnection );
        menu->addSeparator();
        return;
    }
    m_eventpoint = mapToScene( toGrid(event->pos()) );

    QAction* copyAction = menu->addAction(QIcon(":/copy.svg"),tr("Copy")+"\tCtrl+C");
    connect( copyAction, SIGNAL( triggered()),
                   this, SLOT(slotCopy()), Qt::UniqueConnection );

    QAction* cutAction = menu->addAction(QIcon(":/cut.svg"),tr("Cut")+"\tCtrl+X");
    connect( cutAction, SIGNAL( triggered()),
                   this, SLOT(slotCut()), Qt::UniqueConnection );

    QAction* removeAction = menu->addAction( QIcon( ":/remove.svg"),tr("Remove")+"\tDel" );
    connect( removeAction, SIGNAL( triggered()),
                     this, SLOT(slotRemove()), Qt::UniqueConnection );

    /*QAction* groupAction = menu->addAction( QIcon( ":/group.png"),tr("Group") );
    connect( groupAction, SIGNAL( triggered()),
                     this, SLOT(slotGroup()), Qt::UniqueConnection );*/
    
    QAction* propertiesAction = menu->addAction( QIcon( ":/properties.svg"),tr("Properties") );
    connect( propertiesAction, SIGNAL( triggered()),
                         this, SLOT(slotProperties()), Qt::UniqueConnection );
    menu->addSeparator();

    QAction* rotateCWAction = menu->addAction( QIcon( ":/rotatecw.svg"),tr("Rotate CW") );
    connect( rotateCWAction, SIGNAL( triggered()),
                       this, SLOT(rotateCW()), Qt::UniqueConnection );

    QAction* rotateCCWAction = menu->addAction(QIcon( ":/rotateccw.svg"),tr("Rotate CCW") );
    connect( rotateCCWAction, SIGNAL( triggered()),
                        this, SLOT(rotateCCW()), Qt::UniqueConnection );

    QAction* rotateHalfAction = menu->addAction(QIcon(":/rotate180.svg"),tr("Rotate 180") );
    connect( rotateHalfAction, SIGNAL( triggered()),
                         this, SLOT(rotateHalf()), Qt::UniqueConnection );
    
    QAction* H_flipAction = menu->addAction(QIcon(":/hflip.svg"),tr("Horizontal Flip") );
    connect( H_flipAction, SIGNAL( triggered()),
                     this, SLOT(H_flip()), Qt::UniqueConnection );
    
    QAction* V_flipAction = menu->addAction(QIcon(":/vflip.svg"),tr("Vertical Flip") );
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

void Component::slotCut()
{
    if( !isSelected() ) Circuit::self()->clearSelection();
    setSelected( true );
    Circuit::self()->copy( m_eventpoint );
    Circuit::self()->removeItems();
}

void Component::slotRemove()
{
    if( !isSelected() ){
        Circuit::self()->clearSelection();
        setSelected( true );
    }
    Circuit::self()->removeItems();
}

void Component::slotGroup()
{
    /*if( !m_group )
    {
        m_group = Circuit::self()->createItemGroup(Circuit::self()->selectedItems());
    }*/
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
            { name = m_id.split("-").first(); }

            m_help = MainWindow::self()->getHelp( name );
        }
        m_propDialog = new PropDialog( CircuitWidget::self(), m_help );
        m_propDialog->setComponent( this );
    }
    m_propDialog->show();
}

void Component::H_flip()
{
    Circuit::self()->addCompState( this, "hflip" );
    m_Hflip = -m_Hflip;
    setflip();
}

void Component::V_flip()
{
    Circuit::self()->addCompState( this, "vflip" );
    m_Vflip = -m_Vflip;
    setflip();
}

void Component::rotateCW()
{
    if( m_hidden ) return;
    Circuit::self()->addCompState( this, "rotation" );
    setRotation( rotation() + 90 );
    emit moved();
}

void Component::rotateCCW()
{
    if( m_hidden ) return;
    Circuit::self()->addCompState( this, "rotation" );
    setRotation( rotation() - 90 );
    emit moved();
}

void Component::rotateHalf()
{
    if( m_hidden ) return;
    Circuit::self()->addCompState( this, "rotation" );
    setRotation( rotation() - 180);
    emit moved();
}

QString Component::idLabel() { return m_idLabel->toPlainText(); }
void Component::setIdLabel( QString id ) { m_idLabel->setPlainText( id ); }

QPointF Component::getIdPos() { return m_idLabel->getLabelPos(); }
void Component::setIdPos( QPointF p ) { m_idLabel->setLabelPos( p ); }

int Component::getIdRot() { return m_idLabel->getAngle(); }
void Component::setIdRot( int r ) { m_idLabel->setAngle(r); }

void Component::setLabelPos( int x, int y, int rot )
{
    m_idLabel->m_labelx = x;
    m_idLabel->m_labely = y;
    m_idLabel->m_labelrot = rot;
    m_idLabel->updtLabelPos();
}
void Component::updtLabelPos() { m_idLabel->updtLabelPos(); }

void Component::setShowId( bool show ) { m_idLabel->setVisible( show ); m_showId = show; }
void Component::setShowVal( bool show ) { m_valLabel->setVisible( show ); m_showVal = show; }

QPointF Component::getValPos() { return m_valLabel->getLabelPos(); }
void Component::setValPos( QPointF p ) { m_valLabel->setLabelPos( p ); }

int Component::getValRot() { return m_valLabel->getAngle(); }
void Component::setValRot( int r ) { m_valLabel->setAngle(r); }

void Component::setValLabelPos( int x, int y, int rot )
{
    m_valLabel->m_labelx = x;
    m_valLabel->m_labely = y;
    m_valLabel->m_labelrot = rot;
    m_valLabel->updtLabelPos();
}

void Component::updtValLabelPos() { m_valLabel->updtLabelPos(); }

void Component::setValLabelText( QString t ) { m_valLabel->setPlainText( t ); }

QString Component::showProp()
{
    if( m_showVal ) return m_showProperty;
    else            return "";
}

void Component::setShowProp( QString prop )
{
    m_showProperty = prop;
    setShowVal( !(prop.isEmpty()) );
}

void Component::setHflip( int hf )
{ 
    if( ( hf != 1 ) && ( hf != -1 ) ) hf = 1;
    m_Hflip = hf;
    setflip();
}

void Component::setVflip( int vf )
{ 
    if( ( vf != 1 ) && ( vf != -1 ) ) vf = 1;
    m_Vflip = vf; 
    setflip();
}

void Component::setflip()
{
    if( m_hidden ) return;
    setTransform( QTransform::fromScale( m_Hflip, m_Vflip ) );
    m_idLabel->setTransform( QTransform::fromScale( m_Hflip, m_Vflip ) );
    m_valLabel->setTransform( QTransform::fromScale( m_Hflip, m_Vflip ) );
    emit flip( m_Hflip, m_Vflip );
    emit moved();
}

void Component::setHidden(bool hid, bool hidArea, bool hidLabel )
{
    m_hidden = hid;

    if( m_graphical ) { for( Pin* pin : m_pin ) pin->setVisible( !hid ); }
    else this->setVisible( !hid );

    if( hidLabel ){
        setShowId( false );
        m_showProperty = "";
}   }

QString Component::print()
{
    if( !m_printable ) return "";
    
    QString str = m_id+" : ";
    str += objectName().split("-").first()+" ";
    /// FIXME if( m_value > 0 ) str += QString::number( m_value )+" "+m_unit+"\n";
    
    return str;
}

void Component::paint( QPainter* p, const QStyleOptionGraphicsItem*, QWidget* )
{
    QPen pen( Qt::black, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin );
    QColor color;

    if( isSelected() )
    {
        //pen.setColor( Qt::darkGray);
        //color = Qt::darkGray;
        p->setOpacity( 0.5 );
        p->fillRect( boundingRect(), Qt::darkGray  );
    }
    color = m_color;

    if( m_warning || m_crashed )
    {
        double speed, opaci;
        if( m_crashed ){
            speed = 0.1; opaci = 0;
            p->fillRect( boundingRect(), QColor(255, 100, 0, 150) );
        }
        else if( m_warning ){
            speed = 0.05; opaci = 0.4;
            p->fillRect( boundingRect(), QColor(200, 200, 0, 150) );
        }
        m_opCount += speed;
        if( m_opCount > 0.6 ) m_opCount = 0.0;
        p->setOpacity( m_opCount+opaci );
    }
    else if( m_isMainComp & !m_hidden )
    {
        p->fillRect( boundingRect(), Qt::yellow  );
        p->setOpacity( 0.5 );
    }
    //p->drawPath( shape() );
    p->setBrush( color );
    p->setPen( pen );
}

#include "moc_component.cpp"
