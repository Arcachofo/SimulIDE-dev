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
                 , m_pluginsdDialog( this )
                 , m_itemLibrary()
{
    m_pSelf = this;

    m_pluginsdDialog.setVisible( false );

    setDragEnabled(true);
    setDragDropMode( QAbstractItemView::DragOnly );
    //setAlternatingRowColors(true);
    setIndentation( 12 );
    setRootIsDecorated( true );
    setCursor( Qt::OpenHandCursor );
    headerItem()->setHidden( true );
    setIconSize( QSize( 36, 24 ));

    LoadLibraryItems();

    setContextMenuPolicy( Qt::CustomContextMenu );

    connect( this, SIGNAL(customContextMenuRequested(const QPoint&)),
             this, SLOT  (slotContextMenu(const QPoint&)), Qt::UniqueConnection );

    connect( this, SIGNAL(itemPressed(QTreeWidgetItem*, int)),
             this, SLOT  (slotItemClicked(QTreeWidgetItem*, int)), Qt::UniqueConnection );
}
ComponentSelector::~ComponentSelector(){}

void ComponentSelector::LoadLibraryItems()
{
    for( LibraryItem* item : m_itemLibrary.items() ) addItem( item );
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
        if( !compSetFilePath.isEmpty( ))  loadXml( compSetFilePath );
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
                if( !catItem && !parent.isEmpty() )
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

                    m_xmlFileList[ name ] = setFile;   // Save xml File used to create this item
                    if( reader.attributes().hasAttribute("info") )
                        name += "???"+reader.attributes().value("info").toString();

                    if( catItem ) addItem( name, catItem, icon, type );
                    reader.skipCurrentElement();
    }   }   }   }
    QString compSetName = setFile.split( "/").last();
    m_compSetUnique.append( compSetName );
    qDebug() << tr("        Loaded Component set:           ") << compSetName;
}

void ComponentSelector::addItem( LibraryItem* item )
{
    QString category = item->category();
    //if( category != "")
    {
        QString icon = ":/"+item->iconfile();
        if( item->createItemFnPtr() )
        {
            QTreeWidgetItem* catItem = getCategory( category );
            if( !catItem )
                return;
             addItem( item->name(), catItem, icon, item->type() );
        }
        else addCategory( item->name(), item->type(), category, icon );
}   }

void ComponentSelector::addItem( QString caption, QTreeWidgetItem* catItem, QString icon, QString type )
{
    QStringList nameFull = caption.split( "???" );
    QString         name = nameFull.first();
    QString info = "";
    if( nameFull.size() > 1 ) info = "   "+nameFull.last();

    bool hidden = MainWindow::self()->settings()->value( name+"/hidden" ).toBool();

    QTreeWidgetItem* item = new QTreeWidgetItem(0);
    QFont font;
    font.setFamily("Ubuntu");
    font.setBold( true );
    
    double fontScale = MainWindow::self()->fontScale();
    if( type == "" ) font.setPixelSize( 12*fontScale );
    else             font.setPixelSize( 11*fontScale );
    
    item->setFlags( QFlag(32) );
    item->setFont( 0, font );
    item->setIcon( 0, QIcon( QPixmap( icon ) ) );
    item->setText( 0, name+info );
    item->setData( 0, Qt::UserRole, type );

    if( ( type == "Subcircuit" )||( type == "MCU" ) )
         item->setData( 0, Qt::WhatsThisRole, name );
    else item->setData( 0, Qt::WhatsThisRole, type );

    catItem->addChild( item );
    item->setHidden( hidden );
    if( MainWindow::self()->settings()->contains( name+"/collapsed" ) )
    {
        bool expanded = !MainWindow::self()->settings()->value( name+"/collapsed" ).toBool();
        item->setExpanded( expanded );
}   }

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
    bool c_hidden = false;
    bool expanded = false;
    QTreeWidgetItem* catItem = NULL;

    if( parent.isEmpty() )                              // Is Main Category
    {
        catItem = new QTreeWidgetItem( this );
        catItem->setIcon( 0, QIcon(":/null-0.png") );
        catItem->setTextColor( 0, QColor( 110, 95, 50 ) );
        catItem->setBackground( 0, QBrush(QColor(240, 235, 245)) );
        expanded = true;
    }else{
        catItem = new QTreeWidgetItem(0);
        catItem->setIcon( 0, QIcon( QPixmap( icon ) ) );
    }
    catItem->setFlags( QFlag(32) );
    QFont font = catItem->font(0);
    font.setPixelSize( 13*MainWindow::self()->fontScale() );
    font.setWeight(75);
    catItem->setFont( 0, font );
    catItem->setText( 0, nameTr );
    catItem->setChildIndicatorPolicy( QTreeWidgetItem::ShowIndicator );
    m_categories.insert( name, catItem );
    m_catTr.insert( nameTr, name );

    if( parent.isEmpty() ) addTopLevelItem( catItem ); // Is root category or root category doesn't exist
    else if( m_categories.contains( parent ) )
        m_categories.value( parent )->addChild( catItem );

    if( MainWindow::self()->settings()->contains( name+"/hidden" ) )
        c_hidden =  MainWindow::self()->settings()->value( name+"/hidden" ).toBool();

    if( MainWindow::self()->settings()->contains( name+"/collapsed" ) )
        expanded = !MainWindow::self()->settings()->value( name+"/collapsed" ).toBool();

    catItem->setExpanded( expanded );
    catItem->setHidden( c_hidden );

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
    
    QAction* managePluginAction = menu.addAction( QIcon(":/fileopen.png"),tr("Manage Components") );
    connect( managePluginAction, SIGNAL(triggered()), this, SLOT(slotManageComponents()) );
    
    menu.exec( mapToGlobal(point) );
}

void ComponentSelector::slotManageComponents()
{
    m_pluginsdDialog.setPluginList();
    m_pluginsdDialog.setVisible( true );
}

void ComponentSelector::search( QString filter )
{
    QList<QTreeWidgetItem*>    cList = findItems( filter, Qt::MatchContains|Qt::MatchRecursive, 0 );
    QList<QTreeWidgetItem*> allItems = findItems( "", Qt::MatchContains|Qt::MatchRecursive, 0 );

    for( QTreeWidgetItem* item : allItems )
    {
        item->setHidden( true );

        if( item->childCount() > 0 ) continue;
        if( cList.contains( item ) )
        {
            item->setHidden( false );

            QTreeWidgetItem* parent = item->parent();
            while( parent )
            {
                parent->setHidden( false );
                parent = parent->parent();
}   }   }   }

#include "moc_componentselector.cpp"
