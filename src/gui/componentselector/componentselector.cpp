/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QApplication>
#include <QDomDocument>
#include <QMessageBox>
#include <QXmlStreamReader>
#include <QMimeData>
#include <QSettings>
#include <QDebug>
#include <QDrag>
#include <QMenu>

#include "componentselector.h"
#include "mainwindow.h"

ComponentSelector* ComponentSelector::m_pSelf = NULL;

ComponentSelector::ComponentSelector( QWidget* parent )
                 : QTreeWidget( parent )
                 , m_mcDialog( this )
                 , m_itemLibrary()
{
    m_pSelf = this;

    m_mcDialog.setVisible( false );

    setDragEnabled(true);
    setDragDropMode( QAbstractItemView::DragOnly );
    //setAlternatingRowColors(true);
    setIndentation( 12 );
    setRootIsDecorated( true );
    setCursor( Qt::OpenHandCursor );
    headerItem()->setHidden( true );

    float scale = MainWindow::self()->fontScale();
    setIconSize( QSize( 30*scale, 24*scale ));
    //setIconSize( QSize( 36, 24 ));

    LoadLibraryItems();

    setContextMenuPolicy( Qt::CustomContextMenu );

    connect( this, &ComponentSelector::customContextMenuRequested,
             this, &ComponentSelector::slotContextMenu, Qt::UniqueConnection );

    connect( this, &ComponentSelector::itemPressed,
             this, &ComponentSelector::slotItemClicked, Qt::UniqueConnection );
}
ComponentSelector::~ComponentSelector(){}

void ComponentSelector::LoadLibraryItems()
{
    for( LibraryItem* item : m_itemLibrary.items() )
    {
        QString category = item->category();

        QString icon = ":/"+item->iconfile();
        if( item->createItemFnPtr() )
        {
            QTreeWidgetItem* catItem = getCategory( category );
            if( catItem ) addItem( item->name(), catItem, icon, item->type() );
        }
        else addCategory( item->name(), item->type(), category, icon );
    }
}

void ComponentSelector::LoadCompSetAt( QDir compSetDir )
{
    compSetDir.setNameFilters( QStringList( "*.xml" ) );

    QStringList xmlList = compSetDir.entryList( QDir::Files );

    if( xmlList.isEmpty() ) return;                  // No comp sets to load

    qDebug() << "\n" << tr("    Loading Component sets at:")<< "\n" << compSetDir.absolutePath()<<"\n";

    for( QString compSetName : xmlList )
    {
        QString compSetFilePath = compSetDir.absoluteFilePath( compSetName );
        if( !compSetFilePath.isEmpty() ) loadXml( compSetFilePath );
    }
    if( compSetDir.cd("test") )
    {
        QStringList dirList = compSetDir.entryList( {"*"}, QDir::Dirs );
        if( !dirList.isEmpty() )
        {
            QTreeWidgetItem* catItem = getCategory("test");
            if( !catItem ) catItem = addCategory("test","test","","" );

            for( QString compName : dirList )
            {
                QString path = compName+"/"+compName;
                QString type;
                if     ( compSetDir.exists( path+".sim1") ) type = "Subcircuit";
                else if( compSetDir.exists( path+".mcu")  ) type = "MCU";
                if( !type.isEmpty() )
                {
                    addItem( compName, catItem, "subc", type );
                    m_dirFileList[ compName ] = compSetDir.absoluteFilePath( compName );
                }
            }
        }
    }
    qDebug() << "\n";
}

void ComponentSelector::loadXml( const QString &setFile )
{
    QFile file( setFile );
    if( !file.open(QFile::ReadOnly | QFile::Text) )
    {
          QMessageBox::warning( 0, "ComponentSelector::loadXml", tr("Cannot read file %1:\n%2.").arg(setFile).arg(file.errorString()));
          return;
    }
    QXmlStreamReader reader(&file);
    if( reader.readNextStartElement() )
    {
        if( reader.name() != "itemlib" )
        {
            QMessageBox::warning(0, "ComponentSelector::loadXml"
                                 , tr("Error parsing file (itemlib):\n%1.").arg(setFile) );
            file.close();
            return;
        }
        while( reader.readNextStartElement() )
        {
            if( reader.name() != "itemset" ) { reader.skipCurrentElement(); continue;}

            QString icon = "";
            if( reader.attributes().hasAttribute("icon") )
            {
                icon = reader.attributes().value("icon").toString();
                if( !icon.startsWith(":/") )
                    icon.prepend( MainWindow::self()->getFilePath("data/images") + "/");
            }

            QString category = reader.attributes().value("category").toString();
            QStringList catPath = category.split( "/" );

            QTreeWidgetItem* catItem = NULL;
            QString parent = "";
            category = "";
            while( !catPath.isEmpty() )
            {
                parent = category;
                category = catPath.takeFirst();
                catItem = getCategory( category );
                if( !catItem /*&& !parent.isEmpty()*/ )
                {
                    QString catTr = QObject::tr( category.toLocal8Bit() );
                    catItem = addCategory( catTr, category, parent, icon );
                }
            }

            QString type = reader.attributes().value("type").toString();

            while( reader.readNextStartElement() )
            {
                if( reader.name() == "item")
                {
                    icon = "";
                    if( reader.attributes().hasAttribute("icon") )
                    {
                        icon = reader.attributes().value("icon").toString();
                        if( !icon.startsWith(":/") )
                            icon.prepend( MainWindow::self()->getFilePath("data/images") + "/");
                    }
                    QString name = reader.attributes().value("name").toString();
                    if( m_components.contains( name ) ) continue;
                    m_components.append( name );

                    m_xmlFileList[ name ] = setFile;   // Save xml File used to create this item
                    if( reader.attributes().hasAttribute("info") )
                        name += "???"+reader.attributes().value("info").toString();

                    if( catItem ) addItem( name, catItem, icon, type );
                    reader.skipCurrentElement();
    }   }   }   }
    QString compSetName = setFile.split( "/").last();
    //m_compSetUnique.append( compSetName );
    qDebug() << tr("        Loaded Component set:           ") << compSetName;
}

void ComponentSelector::addItem( QString caption, QTreeWidgetItem* catItem, QString icon, QString type )
{
    QStringList nameFull = caption.split( "???" );
    QString       nameTr = nameFull.first();
    QString info = "";
    if( nameFull.size() > 1 ) info = "   "+nameFull.last();

    QTreeWidgetItem* item = new QTreeWidgetItem(0);
    float scale = MainWindow::self()->fontScale();
    QFont font;
    font.setFamily("Ubuntu");
    font.setBold( true );
    font.setPixelSize( 11*scale );

    item->setFlags( QFlag(32) );
    item->setFont( 0, font );
    item->setIcon( 0, QIcon( QPixmap( icon ) ) );
    item->setText( 0, nameTr+info );
    item->setData( 0, Qt::UserRole, type );

    if( type == "Subcircuit" || type == "MCU" )
         item->setData( 0, Qt::WhatsThisRole, nameTr );
    else item->setData( 0, Qt::WhatsThisRole, type );

    catItem->addChild( item );

    QString name = item->data( 0, Qt::WhatsThisRole ).toString();
    bool hidden = MainWindow::self()->compSettings()->value( name+"/hidden" ).toBool();
    hideFromList( item, hidden );

    QString shortCut = MainWindow::self()->compSettings()->value( name+"/shortcut" ).toString();
    if( !shortCut.isEmpty() ) m_shortCuts[name] = shortCut;
}

QTreeWidgetItem* ComponentSelector::getCategory( QString category )
{
    QTreeWidgetItem* catItem = NULL;
    if( m_categories.contains( category ) ) catItem = m_categories.value( category );
    else{
        category = m_catTr.value( category );
        if( !category.isEmpty() && m_categories.contains( category ) )
            catItem = m_categories.value( category );
    }
    return catItem;
}

QTreeWidgetItem* ComponentSelector::addCategory( QString nameTr, QString name, QString parent, QString icon )
{
    QTreeWidgetItem* catItem = NULL;

    QFont font;
    font.setFamily("Ubuntu");
    font.setBold( true );
    float fontScale = MainWindow::self()->fontScale();
    bool expanded = false;

    if( parent.isEmpty() )                              // Is Main Category
    {
        catItem = new QTreeWidgetItem( this );
        catItem->setIcon( 0, QIcon(":/null-0.png") );
        catItem->setTextColor( 0, QColor( 110, 95, 50 ) );
        catItem->setBackground( 0, QBrush(QColor(240, 235, 245)) );
        font.setPixelSize( 13*fontScale );
        expanded = true;
    }else{
        catItem = new QTreeWidgetItem(0);
        catItem->setIcon( 0, QIcon( QPixmap( icon ) ) );
        font.setPixelSize( 12*fontScale );
    }
    catItem->setFlags( QFlag(32) );

    catItem->setFont( 0, font );
    catItem->setText( 0, nameTr );
    catItem->setChildIndicatorPolicy( QTreeWidgetItem::ShowIndicator );
    catItem->setData( 0, Qt::WhatsThisRole, name );
    m_categories.insert( name, catItem );
    m_catTr.insert( nameTr, name );

    if( parent.isEmpty() ) addTopLevelItem( catItem ); // Is root category or root category doesn't exist
    else if( m_categories.contains( parent ) )
        m_categories.value( parent )->addChild( catItem );

    if( MainWindow::self()->compSettings()->contains(name+"/collapsed") )
        expanded = !MainWindow::self()->compSettings()->value( name+"/collapsed" ).toBool();
    catItem->setExpanded( expanded );
    catItem->setData( 0, Qt::UserRole+2, expanded );

    bool hidden = MainWindow::self()->compSettings()->value( name+"/hidden" ).toBool();
    hideFromList( catItem, hidden );

    return catItem;
}

void ComponentSelector::slotItemClicked( QTreeWidgetItem* item, int  )
{
    if( !item ) return;

    QString type = item->data( 0, Qt::UserRole ).toString();
    if( type == "" ) return;

    QString name = item->data (0, Qt::WhatsThisRole ).toString(); //item->text(0);
    QMimeData* mimeData = new QMimeData;
    mimeData->setText( name );
    mimeData->setHtml( type );              // esto hay que revisarlo

    QDrag* drag = new QDrag( this );
    drag->setMimeData( mimeData );
    drag->exec( Qt::CopyAction | Qt::MoveAction, Qt::CopyAction );
}

void ComponentSelector::slotContextMenu( const QPoint& point )
{
    QMenu menu;

    QAction* manageComponents = menu.addAction( QIcon(":/fileopen.png"),tr("Manage Components") );
    connect( manageComponents, &QAction::triggered,
             this, &ComponentSelector::slotManageComponents, Qt::UniqueConnection );

    menu.exec( mapToGlobal(point) );
}

void ComponentSelector::slotManageComponents()
{
    m_mcDialog.initialize();
    m_mcDialog.setVisible( true );
}

void ComponentSelector::hideFromList( QTreeWidgetItem* item, bool hide )
{
    item->setData( 0, Qt::UserRole+1, hide );
    item->setHidden( hide );
}

void ComponentSelector::search( QString filter )
{
    QList<QTreeWidgetItem*>    cList = findItems( filter, Qt::MatchContains|Qt::MatchRecursive, 0 );
    QList<QTreeWidgetItem*> allItems = findItems( "", Qt::MatchContains|Qt::MatchRecursive, 0 );

    for( QTreeWidgetItem* item : allItems )
    {
        item->setHidden( true );

        if( item->childCount() > 0  )
        {
            item->setExpanded( item->data( 0, Qt::UserRole+2 ).toBool() );
            continue;
        }
        if( !cList.contains( item ) ) continue;

        bool hidden = item->data( 0, Qt::UserRole+1 ).toBool();
        while( item ){
            item->setHidden( hidden );
            if( item->childCount() > 0 && !hidden && !filter.isEmpty() ) item->setExpanded( true );
            item = item->parent();
        }
    }
}
