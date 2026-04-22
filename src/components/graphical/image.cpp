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
#include "mainwindow.h"
#include "utils.h"

#include "stringprop.h"
#include "boolprop.h"

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

    m_embeedBck = false;
    m_movie = nullptr;

    addProperty( tr("Main"),
        new BoolProp<Image>("Embeed_bck", tr("Embeed background"),""
                          , this, &Image::embeedBck, &Image::setEmbeedBck ) );

    addPropGroup( {"Hidden", {
        new StrProp<Image>("Image_File", tr("Image File"),""
                          , this, &Image::background, &Image::setBackground ),

        new StrProp<Image>("BckGndData", "",""
                          , this, &Image::bckGndData, &Image::setBckGndData )
    }, groupNoCopy | groupHidden} );
}
Image::~Image()
{
    if( m_movie ) delete m_movie;
}

void Image::contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu )
{
    QAction* loadAction = menu->addAction( m_theme->icon(":/load.svg"),tr("Load Image") );
    QObject::connect( loadAction, &QAction::triggered, [=](){ slotLoad(); } );

    if( !m_bckGndData.isEmpty() ){
        QAction* saveAction = menu->addAction( m_theme->icon(":/save.svg"),tr("Save Image") );
        QObject::connect( saveAction, &QAction::triggered, [=](){ slotSave(); } );
    }
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

void Image::slotSave()
{
    QString dir = Circuit::self()->getFilePath();
    QString fileName = QFileDialog::getSaveFileName( MainWindow::self(), tr("Save Image"), dir, "");
    if( fileName.isEmpty() ) return;

    m_image.save( fileName );
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

    if( bck.isEmpty() ) m_image = QPixmap( ":/saveimage.svg" );
    else{
        if( m_image.load( absPath ) ) m_background = absPath;
        else                          m_image = QPixmap( m_background );
        if( QFile::exists( m_background ) ){
            QByteArray ba = fileToByteArray( m_background, "Image::setBackground");
            QString bckData( ba.toHex() );
            m_bckGndData = bckData;
        }
    }
}

QString Image::background()
{
    QDir circuitDir = QFileInfo( Circuit::self()->getFilePath() ).absoluteDir();
    return circuitDir.relativeFilePath( m_background );
}

QString Image::bckGndData()
{
    if( !m_embeedBck ) return "";

    return m_bckGndData;
}

void Image::setBckGndData( QString data )
{
    m_bckGndData = data;

    if( data.isEmpty() ) return;

    QStringView dataRef{data};
    QByteArray ba;
    bool ok;
    for( int i=0; i<dataRef.size(); i+=2 )
    {
        QStringView ch = dataRef.mid( i, 2 );
        ba.append( ch.toInt( &ok, 16 ) );
    }
    m_image.loadFromData( ba );
    update();
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
