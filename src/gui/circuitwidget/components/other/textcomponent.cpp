/***************************************************************************
 *   Copyright (C) 2017 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QCursor>
#include <QPainter>
#include <QTextDocument>

#include "textcomponent.h"
#include "itemlibrary.h"
#include "circuit.h"

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
{
    m_graphical = true;
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
    //m_text->setTextInteractionFlags( Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard | Qt::TextEditable);
    //m_text->setTextInteractionFlags( Qt::TextEditorInteraction | Qt::TextBrowserInteraction );
    m_text->setTextInteractionFlags( 0 );
    m_text->setTextWidth( 90 );
    m_text->setFont( sansFont );
    m_text->setPlainText("... TEXT ...");
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
new StringProp<TextComponent>( "Text"       , tr("Text")        ,""       , this, &TextComponent::getText , &TextComponent::setText, "textEdit" )
    }} );
}
TextComponent::~TextComponent()
{
    delete m_text;
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
    return m_text->toPlainText();
}

void TextComponent::setText( QString text )
{
    m_text->document()->setPlainText( text );
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
