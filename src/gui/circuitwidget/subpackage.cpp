/***************************************************************************
 *   Copyright (C) 2019 by santiago González                               *
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

#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>
#include <QLineEdit>
#include <QTextStream>
#include <QLabel>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QPushButton>

#include "subpackage.h"
#include "itemlibrary.h"
#include "circuitwidget.h"
#include "circuit.h"
#include "simuapi_apppath.h"
#include "utils.h"

#include "stringprop.h"
#include "intprop.h"

QString SubPackage::m_lastPkg = "";

Component* SubPackage::construct( QObject* parent, QString type, QString id )
{ return new SubPackage( parent, type, id ); }

LibraryItem* SubPackage::libraryItem()
{
    return new LibraryItem(
        tr( "Package" ),
        tr( "Other" ),
        "subc2.png",
        "Package",
        SubPackage::construct );
}

SubPackage::SubPackage( QObject* parent, QString type, QString id )
          : Chip( parent, type, id )
{
    m_width  = 4;
    m_height = 8;

    m_changed = false;
    m_fakePin = false;
    m_movePin = false;
    m_isLS    = true;
    m_graphical = true;
    m_boardMode = false;

    m_name = m_id.split("-").first();
    
    m_lsColor = QColor( 210, 210, 255 );
    m_icColor = QColor( 40, 40, 120 );
    m_color = m_lsColor;
    
    m_area  = QRect(0, 0, m_width*8, m_height*8);

    m_boardModeAction = new QAction( tr("Board Mode"), this );
    m_boardModeAction->setCheckable(true);
    m_boardMode = false;
    
    setAcceptHoverEvents( true );
    setZValue( -1 );
    
    m_pkgeFile = SIMUAPI_AppPath::self()->RODataFolder().absolutePath();
    //if( m_lastPkg == "" ) m_lastPkg = m_pkgeFile;

    connect( CircuitWidget::self(), SIGNAL( saving() ),
             this, SLOT( savingCirc() ), Qt::UniqueConnection );

    addPropGroup( { tr("Main"), {
new IntProp   <SubPackage>( "Width"       ,tr("Width")       ,"_Cells", this, &SubPackage::width,      &SubPackage::setWidth ,"uint" ),
new IntProp   <SubPackage>( "Height"      ,tr("Height")      ,"_Cells", this, &SubPackage::height,     &SubPackage::setHeight,"uint"  ),
new StringProp<SubPackage>( "Package_File",tr("Package File"),""      , this, &SubPackage::package,    &SubPackage::setPackage),
new StringProp<SubPackage>( "Background"  ,tr("Background")  ,""      , this, &SubPackage::background, &SubPackage::setBackground ),
new StringProp<SubPackage>( "SubcType"    ,tr("Type")        ,""      , this, &SubPackage::subcTypeStr,&SubPackage::setSubcTypeStr,"enum" )
    }} );
}
SubPackage::~SubPackage(){}

void SubPackage::hoverMoveEvent( QGraphicsSceneHoverEvent* event ) 
{
    if( event->modifiers() & Qt::ShiftModifier) 
    {
        m_fakePin = true;
        
        int xPos = snapToCompGrid( (int)event->pos().x() );
        int yPos = snapToCompGrid( (int)event->pos().y() );
        
        if( xPos == 0 && yPos >= 8 && yPos <= m_height*8-8 ) // Left
        {
            m_angle = 180;
            m_p1X = -8;
            m_p1Y = yPos;
            m_p2X = 0;
            m_p2Y = yPos;
        }
        else if( xPos == m_width*8 && yPos >= 8 && yPos <= m_height*8-8 ) // Right
        {
            m_angle = 0;
            m_p1X = m_width*8+8;
            m_p1Y = yPos;
            m_p2X = m_width*8;
            m_p2Y = yPos;
        }
        else if( yPos == 0 && xPos >= 8&& xPos <= m_width*8-8 ) // Top 
        {
            m_angle = 90;
            m_p1X = xPos;
            m_p1Y = -8;
            m_p2X = xPos;
            m_p2Y = 0;
        }
        else if( yPos == m_height*8 && xPos >= 8 && xPos <= m_width*8-8 ) // Bottom
        {
            m_angle = 270;
            m_p1X = xPos;
            m_p1Y = m_height*8+8;
            m_p2X = xPos;
            m_p2Y = m_height*8;
        }
        else m_fakePin = false;

        Circuit::self()->update();
    }
    else QGraphicsItem::hoverMoveEvent(event);
}

void SubPackage::hoverLeaveEvent( QGraphicsSceneHoverEvent* event ) 
{
    m_fakePin = false;
    Circuit::self()->update();
    QGraphicsItem::hoverLeaveEvent(event);
}

void SubPackage::mousePressEvent( QGraphicsSceneMouseEvent* event )
{
    if( m_selMainCo ) return; // Used when creating Boards to set this as main component

    if( m_fakePin )
    {
        event->accept();
        m_fakePin = false;

        m_eventPin = new Pin( m_angle, QPoint(m_p1X,m_p1Y ), "name", 0, this );
        m_eventPin->setFlag( QGraphicsItem::ItemStacksBehindParent, false );
        m_eventPin->setLabelText( "Name" );
        m_eventPin->setPinId( "Id" );
        m_eventPin->setLabelColor( QColor( Qt::black ) );
        m_eventPin->setLabelPos();
        m_pins.append( m_eventPin );

        editPin();
        Circuit::self()->update();
    }
    else if( m_movePin )
    {
        event->accept();
        ungrabMouse();
        setCursor( Qt::OpenHandCursor );
        
        m_changed = true;
        m_movePin = false;
        m_eventPin = 0l;
    }
    else Component::mousePressEvent( event );
}

void SubPackage::mouseMoveEvent( QGraphicsSceneMouseEvent* event )
{
    if( m_movePin && m_eventPin )
    {
        event->accept();

        int pinLenth = m_eventPin->length();
        int Xmax = m_area.width();
        int Ymax = m_area.height();
        int pinX = m_eventPin->pos().x();
        int pinY = m_eventPin->pos().y();
        QPointF delta = toCompGrid(event->scenePos()) - toCompGrid(event->lastScenePos());
        
        int deltaX = delta.x();
        int deltaY = delta.y();

        if( (m_angle == 0) || (m_angle == 180) )   // Right or Left
        {
            if( (pinY >= Ymax) && (deltaY>0) ) return;
            if( (pinY <= 0)    && (deltaY<0) ) return;
            else{
                if( m_angle == 0 )    // Right
                {
                    if( (pinX >= Xmax+pinLenth) && (deltaX>0) ) return;
                    if( (pinX <= 16+pinLenth) && (deltaX<0) ) return;
                }else                  // Left
                {
                    if( (pinX >= Xmax-16-pinLenth) && (deltaX>0) ) return;
                    if( (pinX <= -pinLenth) && (deltaX<0) ) return;
        }   }   }
        else if( (m_angle == 90) || (m_angle == 270) )  // Top or Bottom
        {
            if( (pinX >= Xmax) && (deltaX>0) ) return;
            if( (pinX <= 0)    && (deltaX<0) ) return;
            else{
                if( m_angle == 90 )   // Top
                {
                    if( (pinY >= Ymax-16-pinLenth) && (deltaY>0) ) return;
                    if( (pinY <= -pinLenth) && (deltaY<0) ) return;
                }else{                 // Bottom
                    if( (pinY >= Ymax+pinLenth) && (deltaY>0) ) return;
                    if( (pinY <= 16+pinLenth) && (deltaY<0) ) return;
        }   }   }
        m_eventPin->moveBy( deltaX, deltaY );
    }
    else Component::mouseMoveEvent( event );
}

void SubPackage::contextMenuEvent( QGraphicsSceneContextMenuEvent* event )
{
    int xPos = snapToCompGrid( (int)event->pos().x() );
    int yPos = snapToCompGrid( (int)event->pos().y() );

    m_eventPin = 0l;

    for( Pin* pin : m_pins )
    {
        int xPin = pin->x();
        int yPin = pin->y();
        int angle = pin->pinAngle();

        if     ( angle == 0 )   xPin -= 8; // Right
        else if( angle == 180 ) xPin += 8; // Left
        else if( angle == 90 )  yPin += 8; // Top
        else if( angle == 270 ) yPin -= 8; // Bottom

        if(( abs(yPin-yPos)<4 ) && ( abs(xPin-xPos)<4 ) )
        { m_eventPin = pin; break; }
    }
    event->accept();
    QMenu* menu = new QMenu();
    contextMenu( event, menu );
    menu->deleteLater();
}

void SubPackage::contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu )
{
    if( m_eventPin )
    {
        QAction* moveAction = menu->addAction( QIcon(":/hflip.png"),tr("Move Pin ")+m_eventPin->getLabelText() );
        connect( moveAction, SIGNAL( triggered()), this, SLOT( movePin() ), Qt::UniqueConnection );

        QAction* editAction = menu->addAction( QIcon(":/rename.png"),tr("Edit Pin ")+m_eventPin->getLabelText() );
        connect( editAction, SIGNAL( triggered()), this, SLOT( editPin() ), Qt::UniqueConnection );

        QAction* deleteAction = menu->addAction( QIcon(":/remove.png"),tr("Delete Pin ")+m_eventPin->getLabelText() );
        connect( deleteAction, SIGNAL( triggered()), this, SLOT( deleteEventPin() ), Qt::UniqueConnection );

        menu->exec( event->screenPos() );
    }else{
        QAction* loadAction = menu->addAction( QIcon(":/open.png"),tr("Load Package") );
        connect( loadAction, SIGNAL( triggered()), this, SLOT( loadPackage() ), Qt::UniqueConnection );

        QAction* saveAction = menu->addAction( QIcon(":/save.png"),tr("Save Package") );
        connect( saveAction, SIGNAL(triggered()), this, SLOT( slotSave() ), Qt::UniqueConnection );

        menu->addSeparator();

        m_boardModeAction->setChecked( m_boardMode );
        menu->addAction( m_boardModeAction );
        connect( m_boardModeAction, SIGNAL( triggered()),
                              this, SLOT( boardMode() ), Qt::UniqueConnection );

        QAction* mainCompAction = menu->addAction( QIcon(":/subcl.png"),tr("Select Main Component") );
        connect( mainCompAction, SIGNAL( triggered()),
                           this, SLOT( mainComp() ), Qt::UniqueConnection );

        Component::contextMenu( event, menu );
    }
}

void SubPackage::boardMode()
{
    m_boardMode = m_boardModeAction->isChecked();
    setBoardMode();
}

void SubPackage::setBoardMode()
{
    QList<Connector*>* conList = Circuit::self()->conList();
    for( Connector* con : *conList )
    {
        if( con ) con->setVisib( !m_boardMode );
    }
    QList<Component*>* compList = Circuit::self()->compList();
    for( Component* comp : *compList )
    {
        if( comp->itemType() == "Package" ) continue;
        if( m_boardMode )
        {
            comp->setCircPos( comp->pos() );
            comp->setCircRot(comp->rotation() );
            if( comp->boardRot() != -1e+6 )
            {
                comp->setPos( comp->boardPos() + this->pos() );
                comp->setRotation( comp->boardRot() );
        }   }
        else{
            comp->setBoardPos( comp->pos()-this->pos() );
            comp->setBoardRot( comp->rotation() );
            comp->setPos( comp->circPos() );
            comp->setRotation( comp->circRot() );
        }
        comp->setHidden( m_boardMode );
    }
    Circuit::self()->update();
}

void SubPackage::savingCirc()
{
    if( m_subcType >= Board ) m_saveBoard = true;
    if( m_boardMode ) { m_boardMode = false; setBoardMode(); }
}

void SubPackage::remove()
{
    if( m_changed )
    {
        const QMessageBox::StandardButton ret
        = QMessageBox::warning( 0l, "SubPackage::remove",
                               tr("\nPackage has been modified.\n"
                                  "Do you want to save your changes?\n"),
                               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
                               
        if     ( ret == QMessageBox::Save ) slotSave();
        else if( ret == QMessageBox::Cancel ) return;
    }
    for( Pin* pin : m_pins ) pin->removeConnector();
    Component::remove();
    //Circuit::self()->compRemoved( true );
}

void SubPackage::setWidth( int width )
{
    if( m_width == width ) return;
    m_changed = true;

    m_width = width;
    m_area = QRect(0, 0, m_width*8, m_height*8);
    
    Circuit::self()->update();
}

void SubPackage::setHeight( int height )
{
    if( m_height == height ) return;
    m_changed = true;
    
    m_height = height;
    m_area = QRect( 0, 0, m_width*8, m_height*8 );
    
    Circuit::self()->update();
}

void SubPackage::movePin()
{
    if( !m_eventPin ) return;
    
    m_changed = true;
    m_movePin = true;
    m_angle = m_eventPin->pinAngle();
    
    grabMouse();
}

void SubPackage::editPin()
{
    if( !m_eventPin ) return;
    m_angle = m_eventPin->pinAngle();

    EditDialog* editDialog = new EditDialog( this, m_eventPin, 0l );
    connect( editDialog, SIGNAL( finished(int) ),
                   this, SLOT( editFinished(int) ), Qt::UniqueConnection );

    editDialog->exec();
    editDialog->deleteLater();
}

void SubPackage::editFinished( int r )
{ if( m_changed ) Circuit::self()->saveState(); }

void SubPackage::deleteEventPin()
{
    if( !m_eventPin ) return;
    m_changed = true;

    m_pins.removeOne( m_eventPin );
    deletePin( m_eventPin );
    m_eventPin = 0l;
    
    Circuit::self()->update();
}

void SubPackage::setPinId( QString id )
{
    m_eventPin->setPinId( id.replace("-","") );
    m_changed = true;
}

void SubPackage::setPinName( QString name )
{
    m_eventPin->setLabelText( name );
    m_changed = true;
}

void SubPackage::invertPin( bool invert )
{
    m_eventPin->setInverted( invert );
    Circuit::self()->update();
    m_changed = true;
}

void SubPackage::unusePin( bool unuse )
{
    m_eventPin->setUnused( unuse );
    Circuit::self()->update();
    m_changed = true;
}

void SubPackage::pointPin( bool point )
{
    int length = 8;
    if( point ) length = 1;
    int oldLength = m_eventPin->length();
    if( length == oldLength ) return;

    int deltaL = 8;
    if( oldLength > 1 ) deltaL = -8;

    m_eventPin->setLength( length );

    if     ( m_angle == 180 ) m_eventPin->moveBy(-deltaL, 0 );// Left
    else if( m_angle == 0 )   m_eventPin->moveBy( deltaL, 0 );// Right
    else if( m_angle == 90 )  m_eventPin->moveBy( 0,-deltaL );// Top
    else if( m_angle == 270 ) m_eventPin->moveBy( 0, deltaL );// Bottom

    m_eventPin->setFlag( QGraphicsItem::ItemStacksBehindParent, false );
    Circuit::self()->update();
    m_changed = true;
}

QString SubPackage::package()
{
    return m_pkgeFile;
    Circuit::self()->update();
}

void SubPackage::setPackage( QString package )
{
    m_pkgeFile = package;

    m_name = QFileInfo( m_pkgeFile ).baseName().remove(".package").remove("_LS");
    m_label.setPlainText( m_name );

    Chip::initChip();
    
    setLogicSymbol( m_isLS );
    Circuit::self()->update();
    m_changed = false;
}

void SubPackage::setLogicSymbol( bool ls )
{
    if( ls == m_isLS ) return;
    m_isLS = ls;

    QColor labelColor = QColor( 0, 0, 0 );

    if( ls ) m_color = m_lsColor;
    else{
        m_color = m_icColor;
        labelColor = QColor( 250, 250, 200 );
    }
    for( Pin* pin : m_pins ) pin->setLabelColor( labelColor );

    Circuit::self()->update();
}

void SubPackage::slotSave()
{
    QDir pdir = QFileInfo( Circuit::self()->getFilePath() ).absoluteDir();
    QString pkgeFile = pdir.absoluteFilePath( m_pkgeFile );

    const QString dir = pkgeFile;
    QString fileName = QFileDialog::getSaveFileName( 0l, tr("Save Package"), dir,
                                                     tr("Packages (*.package);;All files (*.*)"));
    if( fileName.isEmpty() ) return;
    savePackage( fileName );
}

QString SubPackage::pinEntry( Pin* pin, int pP )
{
    QString label = pin->getLabelText();
    QString id    = pin->pinId().split( "-" ).last().replace( " ", "" );
    QString paPin = QString::number( pP );
    
    QString xpos   = QString::number( pin->x() );
    QString ypos   = QString::number( pin->y() );
    QString angle  = QString::number( pin->pinAngle() );
    QString length = QString::number( pin->length() );
    
    QString type = "";
    if     ( pin->inverted() ) type = "inverted";
    else if( pin->unused() )   type = "unused";
    
    return "    <pin type=\""+type
            +"\" xpos=\""+xpos
            +"\" ypos=\""+ypos
            +"\" angle=\""+angle
            +"\" length=\""+length
            +"\" id=\""+id
            +"\" label=\""+label
            +"\" /><!-- packagePin"+paPin+" -->\n";
}

void SubPackage::loadPackage()
{
    Circuit::self()->saveState();

    QDir pkgDir;
    QString dir;

    if( m_lastPkg == "" )
    {
        pkgDir = QFileInfo( Circuit::self()->getFilePath() ).absoluteDir();
        dir = pkgDir.absoluteFilePath( m_pkgeFile );
    }else{
        pkgDir = QFileInfo( m_lastPkg ).absoluteDir();
        dir = pkgDir.absolutePath();
    }
    QString fileName = QFileDialog::getOpenFileName( 0l, tr("Load Package File"), dir,
                       tr("Packages (*.package);;All files (*.*)"));

    if( fileName.isEmpty() ) return; // User cancels loading
    setPackage( fileName );

    QDir pdir = QFileInfo( Circuit::self()->getFilePath() ).absoluteDir();
    m_pkgeFile = pdir.relativeFilePath( fileName );
    m_lastPkg = fileName;
}

void SubPackage::savePackage( QString fileName )
{
    if( !fileName.endsWith(".package") ) fileName.append(".package");

    QFile file( fileName );

    if( !file.open(QFile::WriteOnly | QFile::Text) )
    {
          QMessageBox::warning(0l, "Circuit::saveCircuit",
          tr("Cannot write file %1:\n%2.").arg(fileName).arg(file.errorString()));
          return;
    }
    QTextStream out(&file);
    out.setCodec("UTF-8");

    QApplication::setOverrideCursor(Qt::WaitCursor);
    int pins = m_pins.size();

    QString subcType = subcTypeStr();

    out << "<!DOCTYPE SimulIDE>\n\n";
    out << "<!-- This file was generated by SimulIDE -->\n\n";
    out << "<packageB name=\""+m_name
           +"\" pins=\"" +QString::number( pins )
           +"\" width=\"" +QString::number( m_width )
           +"\" height=\"" +QString::number( m_height )
           +"\" background=\"" +m_background
           +"\" type=\"" +subcType
           +"\" >\n\n";
    
    int pP = 1;
    for( Pin* pin : m_pins ) { out << pinEntry( pin, pP); pP++; }

    out << "    \n";
    out << "</packageB>\n";

    file.close();
    QApplication::restoreOverrideCursor();

    QDir dir = QFileInfo( Circuit::self()->getFilePath() ).absoluteDir();

    m_pkgeFile = dir.relativeFilePath( fileName );
    m_lastPkg = fileName;
    m_changed = false;
}

void SubPackage::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Chip::paint( p, option, widget );

    if( m_fakePin )
    {
        QPen pen = p->pen();
        pen.setWidth( 2 );
        pen.setColor( Qt::gray );
        p->setPen(pen);
        p->drawLine( m_p1X, m_p1Y, m_p2X, m_p2Y);
}   }

//_____________________________________________________________
//_____________________________________________________________


EditDialog::EditDialog( SubPackage* pack, Pin* eventPin, QWidget* parent )
          : QDialog( parent, Qt::WindowCloseButtonHint )
{
    m_package = pack;

    m_nameLabel = new QLabel( tr("Pin Name:") );
    m_nameLineEdit = new QLineEdit;
    m_nameLineEdit->setText( eventPin->getLabelText() );
    m_nameLabel->setBuddy( m_nameLineEdit );
    QHBoxLayout* nameLayout = new QHBoxLayout;
    nameLayout->addWidget( m_nameLabel );
    nameLayout->addWidget( m_nameLineEdit );

    m_idLabel = new QLabel( tr("Pin Id:") );
    m_idLineEdit = new QLineEdit;
    m_idLineEdit->setText( eventPin->pinId().split( "-" ).last() );
    m_idLabel->setBuddy( m_idLineEdit );
    QHBoxLayout* idLayout = new QHBoxLayout;
    idLayout->addWidget( m_idLabel );
    idLayout->addWidget( m_idLineEdit );

    m_invertCheckBox = new QCheckBox(tr("Invert Pin"));
    m_invertCheckBox->setChecked( eventPin->inverted() );
    m_unuseCheckBox  = new QCheckBox(tr("Unused Pin"));
    m_unuseCheckBox->setChecked( eventPin->unused() );

    m_pointCheckBox = new QCheckBox(tr("Point Pin"));
    m_pointCheckBox->setChecked( (eventPin->length() < 7) );

    QDialogButtonBox* bb = new QDialogButtonBox( QDialogButtonBox::Ok );
    QPushButton* okBtn = bb->button(QDialogButtonBox::Ok);
    okBtn->setAutoDefault(true);
    okBtn->setDefault(true);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addLayout( nameLayout );
    layout->addLayout( idLayout );
    layout->addWidget( m_invertCheckBox );
    layout->addWidget( m_unuseCheckBox );
    layout->addWidget( m_pointCheckBox );
    layout->addWidget( bb );

    setLayout( layout );
    setWindowTitle( tr("Edit Pin ")+eventPin->getLabelText() );

    connect( bb, SIGNAL(accepted()),
           this, SLOT(accept()), Qt::UniqueConnection);

    connect( m_nameLineEdit, SIGNAL( textChanged( QString ) ),
                       pack, SLOT( setPinName( QString ) ), Qt::UniqueConnection );

    connect( m_idLineEdit, SIGNAL( textEdited( QString ) ),
                     pack, SLOT( setPinId( QString ) ), Qt::UniqueConnection );

    connect( m_idLineEdit, SIGNAL( textEdited( QString ) ),
                     pack, SLOT( setPinId( QString ) ), Qt::UniqueConnection );

    connect( m_invertCheckBox, SIGNAL( toggled( bool ) ),
                         this, SLOT( invertPin( bool ) ), Qt::UniqueConnection );

    connect( m_unuseCheckBox,  SIGNAL( toggled( bool ) ),
                        pack,  SLOT( unusePin( bool ) ), Qt::UniqueConnection );

    connect( m_pointCheckBox,  SIGNAL( toggled( bool ) ),
                        pack,  SLOT( pointPin( bool ) ), Qt::UniqueConnection );
}

void EditDialog::invertPin( bool invert )
{
    QString id = m_idLineEdit->text();
    if( invert && !id.startsWith("!") ) id.prepend("!");
    else if( !invert && id.startsWith("!") ) id.remove( 0, 1 );

    m_idLineEdit->setText( id );
    m_package->invertPin( invert );
}

#include "moc_subpackage.cpp"
