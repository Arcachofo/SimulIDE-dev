/***************************************************************************
 *   Copyright (C) 2019 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <math.h>

#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>
#include <QLineEdit>
#include <QTextStream>
#include <QLabel>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QComboBox>
#include <QDebug>

#include "subpackage.h"
#include "packagepin.h"
#include "itemlibrary.h"
#include "circuitwidget.h"
#include "circuit.h"
#include "node.h"
#include "utils.h"

#include "stringprop.h"
#include "boolprop.h"
#include "intprop.h"

QString SubPackage::m_lastPkg = "";

Component* SubPackage::construct( QObject* parent, QString type, QString id )
{ return new SubPackage( parent, type, id ); }

LibraryItem* SubPackage::libraryItem()
{
    return new LibraryItem(
        tr( "Package" ),
        "Other",
        "resistordip.png",
        "Package",
        SubPackage::construct );
}

SubPackage::SubPackage( QObject* parent, QString type, QString id )
          : Chip( parent, type, id )
{
    m_subcType = Chip::None;
    m_width  = 4;
    m_height = 8;
    m_area = QRect(0, 0, m_width*8, m_height*8);

    m_changed = false;
    m_fakePin = false;
    m_isLS    = true;
    m_graphical = true;
    m_boardMode = false;
    m_name = m_id.split("-").first();

    m_lsColor = QColor( 210, 210, 255 );
    m_icColor = QColor( 40, 40, 120 );
    m_color = m_lsColor;

    m_boardModeAction = new QAction( tr("Board Mode"), this );
    m_boardModeAction->setCheckable( true );
    m_boardMode = false;
    
    setAcceptHoverEvents( true );
    setZValue(-3 );
    
    m_pkgeFile = "";

    addPropGroup( { tr("Main"), {
new StrProp <SubPackage>("SubcType"    ,tr("Type")  ,""      , this, &SubPackage::subcTypeStr,&SubPackage::setSubcTypeStr,0,"enum" ),
new IntProp <SubPackage>("Width"       ,tr("Width") ,"_Cells", this, &SubPackage::width,      &SubPackage::setWidth ,0,"uint" ),
new IntProp <SubPackage>("Height"      ,tr("Height"),"_Cells", this, &SubPackage::height,     &SubPackage::setHeight,0,"uint"  ),
new StrProp <SubPackage>("Name"        ,tr("Name")        ,"", this, &SubPackage::name,       &SubPackage::setName ),
new StrProp <SubPackage>("Package_File",tr("Package File"),"", this, &SubPackage::package,    &SubPackage::setPackage),
new StrProp <SubPackage>("Background"  ,tr("Background")  ,"", this, &SubPackage::background, &SubPackage::setBackground ),
new BoolProp<SubPackage>("Logic_Symbol",tr("Logic Symbol"),"", this, &SubPackage::logicSymbol,&SubPackage::setLogicSymbol ),
    }, 0} );
}
SubPackage::~SubPackage(){}

void SubPackage::setSubcTypeStr( QString s )
{
    int index = getEnumIndex( s.remove("subc") );
    subcType_t type = (subcType_t)index;
    if( m_subcType == type ) return;

    if( type >= Board )
    {
        if( Circuit::self()->getBoard() ) // Only one board Package can be in the circuit
        {
            qDebug() << "SubPackage::setSubcTypeStr: ERROR: Only one Board allowed";
            return;
        }
        Circuit::self()->setBoard( this );
    }
    else if( Circuit::self()->getBoard() == this ) Circuit::self()->setBoard( NULL );

    m_subcType = type;

    if( m_showVal && (m_showProperty == "SubcType") )
        setValLabelText( m_enumNames.at( index ) );
}

void SubPackage::hoverMoveEvent( QGraphicsSceneHoverEvent* event ) 
{
    if( event->modifiers() & Qt::ShiftModifier) 
    {
        m_fakePin = true;
        
        int xPos = snapToGrid( (int)event->pos().x() );
        int yPos = snapToGrid( (int)event->pos().y() );
        
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
        else //m_fakePin = false;
        {
            m_angle = 180;
            m_p1X = xPos;
            m_p2X = xPos+1;
            m_p1Y = m_p2Y = yPos;
        }
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
    if( m_selecComp ) return; // Used when linking or creating Boards to set this as main component

    if( m_fakePin )
    {
        event->accept();
        m_fakePin = false;

        QColor color = m_isLS ? Qt::black : QColor( 250, 250, 200 );

        m_eventPin = new PackagePin( m_angle, QPoint(m_p1X,m_p1Y ), "name", 0, this );
        ///m_eventPin->setEnabled( false );
        m_eventPin->setFlag( QGraphicsItem::ItemStacksBehindParent, false );
        m_eventPin->setPinId( "Id" );
        m_eventPin->setLabelColor( color );
        m_eventPin->setLabelPos();
        if( m_p2X == m_p1X+1) m_eventPin->setLength( 1 );
        else                  m_eventPin->setLabelText( "Name" );
        m_pkgePins.append( m_eventPin );

        editPin();
        Circuit::self()->update();
    }
    else Component::mousePressEvent( event );
}

void SubPackage::contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu )
{
    QAction* loadAction = menu->addAction( QIcon(":/open.png"),tr("Load Package") );
    connect( loadAction, &QAction::triggered, this, &SubPackage::loadPackage, Qt::UniqueConnection );

    QAction* saveAction = menu->addAction( QIcon(":/save.png"),tr("Save Package") );
    connect( saveAction, &QAction::triggered, this, &SubPackage::slotSave, Qt::UniqueConnection );

    menu->addSeparator();

    if( m_subcType >= Board )
    {
        m_boardModeAction->setChecked( m_boardMode );
        menu->addAction( m_boardModeAction );
        connect( m_boardModeAction, &QAction::triggered,
                              this, &SubPackage::boardModeSlot, Qt::UniqueConnection );
    }
    QAction* mainCompAction = menu->addAction( QIcon(":/subcl.png"),tr("Select Main Component") );
    connect( mainCompAction, &QAction::triggered,
                       this, &SubPackage::mainComp, Qt::UniqueConnection );

    Component::contextMenu( event, menu );
}

void SubPackage::compSelected( Component* comp )
{
    if( comp) comp->setMainComp( !comp->isMainComp() );
}

void SubPackage::boardModeSlot()
{
    m_boardMode = m_boardModeAction->isChecked();
    setBoardMode( m_boardMode );
    Component::m_boardMode = m_boardMode;
    Circuit::self()->update();
}

void SubPackage::setBoardMode( bool mode )
{
    for( Connector* con : *Circuit::self()->conList() )
    {
        if( con ) con->setVisib( !mode );
    }
    for( Node* nod : *Circuit::self()->nodeList() ) nod->setHidden( mode );

    for( Component* comp : *Circuit::self()->compList() )
    {
        if( comp->itemType() == "Package" ) continue;
        if( mode )
        {
            comp->setCircPos( comp->pos() );
            comp->setCircRot( comp->rotation() );
            if( comp->boardRot() != -1e+6 )  // Board Position already defined
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
        comp->setHidden( mode );
    }
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
    Component::remove();
}

void SubPackage::setWidth( int width )
{
    if( width < 1 ) width = 1;
    if( m_width == width ) return;
    m_changed = true;

    m_width = width;
    m_area = QRect(0, 0, m_width*8, m_height*8);
    
    Circuit::self()->update();
}

void SubPackage::setHeight( int height )
{
    if( height < 1 ) height = 1;
    if( m_height == height ) return;
    m_changed = true;
    
    m_height = height;
    m_area = QRect( 0, 0, m_width*8, m_height*8 );
    
    Circuit::self()->update();
}

Pin* SubPackage::addPin( QString id, QString type, QString label, int pos, int xpos, int ypos, int angle, int length  )
{
    PackagePin* pin = new PackagePin( angle, QPoint(xpos, ypos), m_id+"-"+id, pos-1, this ); // pos in package starts at 1
    //pin->setEnabled( false );

    QColor color = m_isLS ? Qt::black : QColor( 250, 250, 200 );

    pin->setLabelColor( color );
    pin->setPos( QPoint( xpos, ypos ) );
    pin->setPinAngle( angle );
    pin->setLength( length );
    pin->setLabelText( label );
    pin->setInverted( type == "inverted" || type == "inv" );
    //pin->setFlag( QGraphicsItem::ItemStacksBehindParent, false );

    m_pkgePins.append( pin );
    return pin;
}

void SubPackage::editPin()
{
    if( !m_eventPin ) return;
    m_angle = m_eventPin->pinAngle();

    EditDialog* editDialog = new EditDialog( this, m_eventPin, NULL );
    connect( editDialog, &EditDialog::finished,
                   this, &SubPackage::editFinished, Qt::UniqueConnection );

    editDialog->exec();
    editDialog->deleteLater();
}

void SubPackage::editFinished( int )
{ if( m_changed ) Circuit::self()->saveState(); }

void SubPackage::deleteEventPin()
{
    if( !m_eventPin ) return;
    m_changed = true;

    m_pkgePins.removeOne( m_eventPin );
    m_signalPin.removeOne( m_eventPin );
    delete m_eventPin;
    m_eventPin = NULL;
    
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

void SubPackage::setPinAngle( int i )
{
    int angle = 0;
    if     ( i == 1 ) angle = 180;
    else if( i == 2 ) angle = 90;
    else if( i == 3 ) angle = 270;
    m_eventPin->setPinAngle( angle );
    m_eventPin->setLabelPos();
    update();
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

    m_pkgePins.clear();
    for( Pin* pin : m_pin ) deletePin( pin );

    Chip::initChip();

    m_pkgePins += m_unusedPins;

    //m_name = QFileInfo( m_pkgeFile ).baseName().remove(".package").remove("_LS");
    m_label.setPlainText( m_name );
    
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
    for( Pin* pin : m_pkgePins ) pin->setLabelColor( labelColor );

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

void SubPackage::loadPackage()
{
    Circuit::self()->addCompState( this, "Package_File" );

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

    QString subcType = subcTypeStr();

    out << "<!DOCTYPE SimulIDE>\n\n";
    out << "<!-- This file was generated by SimulIDE -->\n\n";
    out << "<packageB name=\""+m_name
           +"\" width=\"" +QString::number( m_width )
           +"\" height=\"" +QString::number( m_height )
           +"\" background=\"" +m_background
           +"\" type=\"" +subcType
           +"\" >\n\n";
    
    int pP = 1;
    for( Pin* pin : m_pkgePins ) { out << pinEntry( pin ); pP++; }

    out << "    \n";
    out << "</packageB>\n";

    file.close();
    QApplication::restoreOverrideCursor();

    QDir dir = QFileInfo( Circuit::self()->getFilePath() ).absoluteDir();

    m_pkgeFile = dir.relativeFilePath( fileName );
    m_lastPkg = fileName;
    m_changed = false;
}

QString SubPackage::pinEntry( Pin* pin )
{
    QString xpos   = "xpos=\""+QString::number( pin->x() )+"\"";
    QString ypos   = "ypos=\""+QString::number( pin->y() )+"\"";
    QString angle  = "angle=\""+QString::number( pin->pinAngle() )+"\"";
    QString length = "length=\""+QString::number( pin->length() )+"\"";
    QString id     = "id=\""+pin->pinId().split( "-" ).last().replace( " ", "" )+"\"";
    QString label  = "label=\""+pin->getLabelText().replace("<","&#x3C;").replace("=","&#x3D;").replace(">","&#x3E;")+"\"";
    QString type;
    if( pin->inverted() ) type = "inv";
    if( pin->unused()   ) type = "nc";
    type = "type=\""+type+"\"";

    return "    <pin "
            +adjustSize( type, 11 )
            +adjustSize( xpos, 12 )
            +adjustSize( ypos, 12 )
            +adjustSize( angle, 12 )
            +adjustSize( length, 12 )
            +adjustSize( id, 10 )
            +adjustSize( label, 12 )
            +"/>\n";
}

QString SubPackage::adjustSize( QString str, int size )
{
    while( str.length() < size ) str.append(" ");
    return str;
}
void SubPackage::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Chip::paint( p, option, widget );

    if( m_background != "" ) p->setBrush( Qt::transparent );
    p->drawRoundedRect( m_area, 1, 1);

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

    m_idLabel = new QLabel( tr("Pin Id:   ") );
    m_idLineEdit = new QLineEdit;
    m_idLineEdit->setText( eventPin->pinId().split( "-" ).last() );
    m_idLabel->setBuddy( m_idLineEdit );
    QHBoxLayout* idLayout = new QHBoxLayout;
    idLayout->addWidget( m_idLabel );
    idLayout->addWidget( m_idLineEdit );

    int angle = eventPin->pinAngle();
    int i = 0;
    if     ( angle == 180 ) i = 1;
    else if( angle == 90  ) i = 2;
    else if( angle == 270 ) i = 3;
    m_angleLabel = new QLabel( tr("Pin Angle:") );
    m_angleBox = new QComboBox();
    m_angleBox->addItems( {tr("Right"), tr("Left"), tr("Top"), tr("Bottom")} );
    m_angleBox->setCurrentIndex( i );
    QHBoxLayout* angleLayout = new QHBoxLayout;
    angleLayout->addWidget( m_angleLabel );
    angleLayout->addWidget( m_angleBox );

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
    layout->addLayout( angleLayout );
    layout->addWidget( m_invertCheckBox );
    layout->addWidget( m_unuseCheckBox );
    layout->addWidget( m_pointCheckBox );
    layout->addWidget( bb );

    setLayout( layout );
    setWindowTitle( tr("Edit Pin ")+eventPin->getLabelText() );

    connect( bb, &QDialogButtonBox::accepted,
           this, &EditDialog::accept, Qt::UniqueConnection);

    connect( m_nameLineEdit, &QLineEdit::textChanged,
                       pack, &SubPackage::setPinName, Qt::UniqueConnection );

    connect( m_idLineEdit, &QLineEdit::textEdited,
                     pack, &SubPackage::setPinId, Qt::UniqueConnection );

    connect( m_angleBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
                   pack, &SubPackage::setPinAngle, Qt::UniqueConnection );

    connect( m_invertCheckBox, &QCheckBox::toggled,
                         this, &EditDialog::invertPin, Qt::UniqueConnection );

    connect( m_unuseCheckBox,  &QCheckBox::toggled,
                        pack,  &SubPackage::unusePin, Qt::UniqueConnection );

    connect( m_pointCheckBox,  &QCheckBox::toggled,
                        pack,  &SubPackage::pointPin, Qt::UniqueConnection );
}

void EditDialog::invertPin( bool invert )
{
    QString id = m_idLineEdit->text();
    if( invert && !id.startsWith("!") ) id.prepend("!");
    else if( !invert && id.startsWith("!") ) id.remove( 0, 1 );

    m_idLineEdit->setText( id );
    m_package->invertPin( invert );
}
