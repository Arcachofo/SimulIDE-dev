/***************************************************************************
 *   Copyright (C) 2019 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QMenu>
#include <QMovie>
#include <QFileDialog>
#include <QPainter>
#include <QDebug>

#include "image.h"
#include "circuit.h"
#include "itemlibrary.h"

#include "stringprop.h"

#define tr(str) simulideTr("Image",str)

Component* Image::construct( QString type, QString id )
{ return new Image( type, id ); }

LibraryItem* Image::libraryItem()
{
    return new LibraryItem(
        tr("Image"),
        "Graphical",
        "img.png",
        "Image",
        Image::construct);
}

Image::Image( QString type, QString id )
     : Shape( type, id )
{
    m_background = "";
    m_image = QPixmap( ":/saveimage.svg" );
    m_hSize = 80;
    m_vSize = 80;
    m_area = QRectF(-40,-40, 80, 80 );

    m_movie = nullptr;

    addPropGroup( {"Hidden", {
        new StrProp<Image>("Image_File", tr("Image File"),""
                          , this, &Image::background, &Image::setBackground )
    }, groupNoCopy | groupHidden} );
}
Image::~Image()
{
    if( m_movie ) delete m_movie;
}

void Image::contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu )
{
    QAction* loadAction = menu->addAction( QIcon(":/load.svg"),tr("Load Image") );
    QObject::connect( loadAction, &QAction::triggered, [=](){ slotLoad(); } );

    menu->addSeparator();
    Component::contextMenu( event, menu );
}

void Image::slotLoad()
{
    QString fil = m_background;
    if( fil.isEmpty() ) fil = Circuit::self()->getFilePath();

    const QString dir = fil;

    QString fileName = QFileDialog::getOpenFileName( 0l, tr("Load Image"), dir,
                       tr("All files (*.*)"));

    if( fileName.isEmpty() ) return; // User cancels loading

    setBackground( fileName );
    Shape::setHSize( m_image.width() );
    Shape::setVSize( m_image.height() );
}

void Image::updateGif( const QRect &rect )
{
    m_image = m_movie->currentPixmap();
    update();
}

void Image::setBackground( QString bck )
{
    if( bck.isEmpty() ) return;

    QDir circuitDir = QFileInfo( Circuit::self()->getFilePath() ).absoluteDir();
    QString absPath = circuitDir.absoluteFilePath( bck );

    if( bck.endsWith(".gif") )
    {
        if( m_movie ) delete m_movie;

        m_movie = new QMovie( bck );
        /// FIXME: m_movie->setParent( this );

        if( m_movie->isValid() )
        {
            m_movie->setCacheMode( QMovie::CacheAll );
            QObject::connect( m_movie, &QMovie::updated, [=](const QRect &rect){ updateGif(rect); } );
            m_movie->start();
        }
        else qDebug() << "Image::setBackground : not a valid Gif animation";
    }

    if( m_image.load( absPath ) )     m_background = absPath;
    else if( m_background.isEmpty() ) m_image = QPixmap( ":/saveimage.svg" );
    else                              m_image = QPixmap( m_background );
}

QString Image::background()
{
    QDir circuitDir = QFileInfo( Circuit::self()->getFilePath() ).absoluteDir();
    return circuitDir.relativeFilePath( m_background );
}

void Image::paint( QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w )
{
    Component::paint( p, o, w );

    QPen pen( Qt::black, m_border, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    p->setPen( pen );
    
    double opacity = p->opacity();
    p->setOpacity( opacity*m_opac );
    p->drawRect( m_area );
    p->setOpacity( opacity );

    p->drawPixmap( m_area, m_image, m_image.rect() );

    if( m_border > 0 )
    {
        p->setBrush( Qt::transparent );
        p->drawRect( m_area );
    }
    Component::paintSelected( p );
}
