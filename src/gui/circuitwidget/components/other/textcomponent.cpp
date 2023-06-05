/***************************************************************************
 *   Copyright (C) 2017 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QCursor>
#include <QPainter>
#include <QTextDocument>
#include <QMenu>
#include <QGuiApplication>

#include "textcomponent.h"
#include "itemlibrary.h"
#include "circuit.h"
#include "simulator.h"
#include "circuit.h"
#include "circuitwidget.h"

#include "stringprop.h"
#include "doubleprop.h"
#include "boolprop.h"
#include "intprop.h"

Component* TextComponent::construct( QObject* parent, QString type, QString id )
{ return new TextComponent( parent, type, id ); }

LibraryItem* TextComponent::libraryItem()
{
    return new LibraryItem(
        tr( "Text" ),
        "Graphical",
        "text.png",
        "TextComponent",
        TextComponent::construct );
}

TextComponent::TextComponent( QObject* parent, QString type, QString id )
             : Component( parent, type, id )
             , Linkable()
{
    m_graphical = true;
    m_Linkable  = true;
    m_opac = 1;
    m_color = QColor( 255, 255, 220 );
    m_font  = "Helvetica [Cronyx]";

    QFont sansFont( m_font, 10 );
    #if QT_VERSION >= QT_VERSION_CHECK(5, 9, 0)
    sansFont.setWeight( QFont::Medium );
    #else
    sansFont.setWeight( QFont::Normal);
    #endif
    sansFont.setFixedPitch(true);

    m_text = new QGraphicsTextItem( this );
    m_text->setTextInteractionFlags( 0 );
    m_text->setTextWidth( 90 );
    m_text->setFont( sansFont );
    m_textString = "... TEXT ...";
    m_text->setPlainText( m_textString );
    m_text->setPos( 0, 0 );
    m_text->document()->setTextWidth(-1);
    m_text->setDefaultTextColor( Qt::darkBlue );
    m_text->setCursor( Qt::OpenHandCursor );
    //m_text->installEventFilter( this );

    m_margin = 5;
    m_docMargin = m_text->document()->documentMargin();
    m_context = false;
    m_border = 1;
    updateGeometry( 0, 0, 0 );
    
    setFontSize( 10 );

    connect( m_text->document(), &QTextDocument::contentsChange,
                           this, &TextComponent::updateGeometry, Qt::UniqueConnection );

    addPropGroup( { tr("Main"), {
new IntProp <TextComponent>( "Margin" , tr("Margin") ,"_Pixels", this, &TextComponent::margin, &TextComponent::setMargin ),
new IntProp <TextComponent>( "Border" , tr("Border") ,"_Pixels", this, &TextComponent::border, &TextComponent::setBorder, "uint" ),
new DoubProp<TextComponent>( "Opacity", tr("Opacity"),""       , this, &TextComponent::opac,   &TextComponent::setOpac )
    }} );
    addPropGroup( { tr("Text"), {
new StringProp<TextComponent>( "Font"       , tr("Font")       ,""       , this, &TextComponent::getFont , &TextComponent::setFont ),
new IntProp   <TextComponent>( "Font_Size"  , tr("Font Size")  ,"_Pixels", this, &TextComponent::fontSize, &TextComponent::setFontSize ),
new BoolProp  <TextComponent>( "Fixed_Width", tr("Fixed_Width"),""       , this, &TextComponent::fixedW  , &TextComponent::setFixedW ),
new StringProp<TextComponent>( "Text"       , tr("Text")        ,""      , this, &TextComponent::getText , &TextComponent::setText, "textEdit" )
    }} );
    addPropGroup( { "Hidden", {
new StringProp<TextComponent>( "Links", "Links","", this, &TextComponent::getLinks , &TextComponent::setLinks )
    }} );
}
TextComponent::~TextComponent()
{
    delete m_text;
}

void TextComponent::updateStep()
{
    if( m_linkedComp.isEmpty() ) return;

    QString text = m_textString;

    for( int i=0; i<m_linkedComp.size(); ++i )
    {
        Component* comp = m_linkedComp.at( i );
        comp->update();
        QString data = comp->getValLabelText();
        text.replace( "$data"+QString::number(i), data );
    }
    m_text->document()->setPlainText( text );
}

void TextComponent::updateGeometry(int, int, int)
{
    m_text->document()->setTextWidth(-1);
    
    int margin = m_margin;
    if( margin < 0 ) margin = 0;
    if( m_hidden ) m_area = QRect( 0, 0, 0, 0 );
    else m_area = QRect( -margin, -margin, m_text->boundingRect().width()+margin*2, m_text->boundingRect().height()+margin*2 );
    
    Circuit::self()->update();
}

int TextComponent::margin() { return m_margin; }

void TextComponent::setMargin( int margin )
{
    //if( margin < 0 ) margin = 0;
    // Maintain compatibility with older versions:
    if( margin < 0 ) m_text->document()->setDocumentMargin( m_docMargin+margin );
    else             m_text->document()->setDocumentMargin( m_docMargin );
    m_margin = margin;
    updateGeometry( 0, 0, 0 );
}

void TextComponent::setOpac( qreal op )
{
    if     ( op > 1 ) op = 1;
    else if( op < 0 ) op = 0;
    m_opac = op;
    update();
}

void TextComponent::setFixedW( bool fixedW ) 
{ 
    m_fixedW = fixedW;
    
    QFont font = m_text->font();
    font.setFixedPitch( fixedW );
    m_text->setFont( font );
    updateGeometry( 0, 0, 0 );
}

QString TextComponent::getText()
{
    return m_textString;
}

void TextComponent::setText( QString text )
{
    m_textString = text;
    if( m_linkedComp.isEmpty() ) m_text->document()->setPlainText( text );
    else updateStep();
}

void TextComponent::setFont( QString font )
{
    if( font == "" ) return;
    m_font = font;
    
    QFont Tfont = m_text->font();
    Tfont.setFamily( font );
    m_text->setFont( Tfont );
    updateGeometry( 0, 0, 0 );
}

void TextComponent::setFontSize( int size )
{
    if( size < 1 ) return;
    m_fontSize = size;
    
    QFont font = m_text->font();
    font.setPixelSize( size );
    m_text->setFont( font );
    updateGeometry( 0, 0, 0 );
}

void TextComponent::createLinks( QList<Component*>* compList )
{
    Linkable::createLinks( compList );
    if( !m_linkedComp.isEmpty() ) Simulator::self()->addToUpdateList( this );
    updateStep();
}

void TextComponent::compSelected( Component* comp )
{
    Linkable::compSelected( comp );
    updateStep();
    Simulator::self()->addToUpdateList( this );
}

void TextComponent::contextMenuEvent( QGraphicsSceneContextMenuEvent* event )
{
    if( !acceptedMouseButtons() ) { event->ignore(); return; }

    event->accept();
    QMenu* menu = new QMenu();

    QAction* linkCompAction = menu->addAction( QIcon(":/subcl.png"),tr("Link to Component") );
    connect( linkCompAction, &QAction::triggered,
                       this, &TextComponent::slotLinkComp, Qt::UniqueConnection );

    menu->addSeparator();

    Component::contextMenu( event, menu );
    menu->deleteLater();
}

void TextComponent::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );

    QPen pen( QColor( 0, 0, 0 ), m_border, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    p->setPen( pen );

    p->setOpacity( m_opac );
    p->fillRect( m_area, p->brush() );
    p->setOpacity( 1 );

    if( m_border > 0 )
    {
        p->setBrush( Qt::transparent );
        p->drawRect( m_area );
    }
}
