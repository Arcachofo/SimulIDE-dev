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

#include "componentlist.h"
#include "treeitem.h"
#include "mainwindow.h"
#include "circuit.h"    /// TODELETE
#include "circuitwidget.h"
#include "chip.h"
#include "utils.h"

ComponentList* ComponentList::m_pSelf = NULL;

ComponentList::ComponentList( QWidget* parent )
             : QTreeWidget( parent )
             , m_mcDialog( this )
             , m_itemLibrary()
{
    m_pSelf = this;

    m_mcDialog.setVisible( false );

    setSelectionMode( QAbstractItemView::SingleSelection );
    setDragEnabled( true );
    //setAcceptDrops( true );
    viewport()->setAcceptDrops( true );
    setDropIndicatorShown( true );

    //setDragDropMode( QAbstractItemView::InternalMove );
    //setDragDropMode( QAbstractItemView::DragOnly );
    //setAlternatingRowColors(true);
    setIndentation( 12 );
    setRootIsDecorated( true );
    setCursor( Qt::OpenHandCursor );
    headerItem()->setHidden( true );

    float scale = MainWindow::self()->fontScale();
    setIconSize( QSize( 30*scale, 24*scale ));

    QString userDir = MainWindow::self()->userPath();
    m_listFile = userDir+"compList.xml";
    //m_listFile = MainWindow::self()->getConfigPath("compList.xml");
    m_insertItems = !QFile::exists( m_listFile ); // xml file doesn't exist: Insert items when created

    m_customComp = false;
    LoadLibraryItems();
    m_customComp = true;

    if( !userDir.isEmpty() && QDir( userDir ).exists() ) LoadCompSetAt( userDir );

    if( !m_insertItems ) insertItems(); // Add items to tree widget from xml file

    /*for( TreeItem* it : m_categories ) // Remove empty categories
    {
        if( it->childCount() ) continue;
        QTreeWidgetItem* pa = it->parent();
        if( pa ) pa->removeChild( it  );
    }*/

    setContextMenuPolicy( Qt::CustomContextMenu );

    connect( this, &ComponentList::customContextMenuRequested,
             this, &ComponentList::slotContextMenu );

    connect( this, &ComponentList::itemPressed,
             this, &ComponentList::slotItemClicked );
}
ComponentList::~ComponentList(){}

void ComponentList::LoadLibraryItems()
{
    for( LibraryItem* item : m_itemLibrary.items() )
    {
        QString category = item->category();

        QString icon = item->iconfile();
        QString iconFile = MainWindow::self()->getDataFilePath("images/"+icon );
        if( !QFile::exists( iconFile ) ) iconFile = ":/"+icon; // Image not in simulide data folder, use hardcoded image

        if( item->createItemFnPtr() )
        {
            TreeItem* catItem = getCategory( category );
            if( catItem ) addItem( item->name(), catItem, iconFile, item->type() );
        }
        else addCategory( item->name(), item->type(), category, iconFile );
    }
}

void ComponentList::LoadCompSetAt( QDir compSetDir )
{
    m_compSetDir = compSetDir;

    if( compSetDir.cd("components") )
    {
        qDebug() << "\n" << tr("    Loading User Components at:")<< "\n" << compSetDir.absolutePath()+"/components"<<"\n";
        loadComps( compSetDir );
        compSetDir.cd("..");
    }
    if( compSetDir.cd("test") )
    {
        QStringList dirList = compSetDir.entryList( {"*"}, QDir::Dirs );
        if( !dirList.isEmpty() )
        {
            TreeItem* catItem = getCategory("test");
            if( !catItem ) catItem = addCategory("test","test","","" );

            for( QString compName : dirList )
            {
                QString path = compName+"/"+compName;
                QString icon = getIcon( "test", compName );
                QString type;

                if( compSetDir.exists( path+".sim1") )
                {
                    if( icon.isEmpty() ) icon = ":/subc.png";
                    type = "Subcircuit";
                }
                else if( compSetDir.exists( path+".mcu") )
                {
                    if( icon.isEmpty() ) icon = ":/ic2.png";
                    type = "MCU";
                }
                if( !type.isEmpty() && !m_components.contains( compName ) )
                {
                    addItem( compName, catItem, icon, type );
                    m_dirFileList[ compName ] = compSetDir.absoluteFilePath( compName );
                }
            }
        }
        compSetDir.cd("..");
    }
    compSetDir.setNameFilters( QStringList( "*.xml" ) );

    QStringList xmlList = compSetDir.entryList( QDir::Files );

    //if( xmlList.isEmpty() ) return;                  // No comp sets to load

    qDebug() << "\n" << tr("    Loading Component sets at:")<< "\n" << compSetDir.absolutePath()<<"\n";

    for( QString compSetName : xmlList )
    {
        QString compSetFilePath = compSetDir.absoluteFilePath( compSetName );
        if( !compSetFilePath.isEmpty() ) loadXml( compSetFilePath );
    }

    qDebug() << "\n";
}

void ComponentList::loadComps( QDir compSetDir )
{
    QStringList compList = compSetDir.entryList( {"*.comp"}, QDir::Files );

    for( QString compFile : compList )
    {
        compFile = compSetDir.absoluteFilePath( compFile );
        if( !compSetDir.exists( compFile ) ) continue;

        QFile file( compFile );
        if( !file.open(QFile::ReadOnly | QFile::Text) ){
              qDebug() << "ComponentList::loadComps Cannot read file"<< endl << compFile << endl << file.errorString();
              continue;
        }
        QFileInfo fi( compFile );
        QString compName = fi.baseName();

        QXmlStreamReader reader( &file );
        if( !reader.readNextStartElement() || reader.name() != "libitem" ){
            qDebug() << "ComponentList::loadComps Error parsing file (itemlib):"<< endl << compFile;
            file.close();
            continue;
        }
        QString icon = "";
        QByteArray ba;

        QXmlStreamAttributes at = reader.attributes();

        if( at.hasAttribute("icondata") )
        {
            QString icStr = at.value("icondata").toString();
            bool ok;
            for( int i=0; i<icStr.size(); i+=2 )
            {
                QString ch = icStr.mid( i, 2 );
                ba.append( ch.toInt( &ok, 16 ) );
            }
        }else{
            if( at.hasAttribute("icon") )
            {
                icon = at.value("icon").toString();
                if( !icon.startsWith(":/") )
                    icon = MainWindow::self()->getDataFilePath("images/"+icon);
            }
            else icon = getIcon("components", compName );
            if( !icon.isEmpty() ) ba = fileToByteArray( icon, "ComponentList::loadComps");
        }

        QPixmap ic;
        ic.loadFromData( ba );
        QIcon ico( ic );

        if( reader.attributes().hasAttribute("compname") )
            compName = reader.attributes().value("compname").toString();

        /// TODO: reuse get category from catPath
        QString category = reader.attributes().value("category").toString();
        QStringList catPath = category.split("/");

        TreeItem* catItem = NULL;
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
                catItem = addCategory( catTr, category, parent, "" );
            }
        }
        QString type = reader.attributes().value("itemtype").toString();

        if( !type.isEmpty() && !m_components.contains( compName ) )
        {
            m_dataFileList[ compName ] = compFile;   // Save comp File used to create this item

            if( reader.attributes().hasAttribute("compinfo") )
                compName += "???"+reader.attributes().value("compinfo").toString();

            addItem( compName, catItem, ico, type );
        }
    }

    QStringList dirList = compSetDir.entryList( {"*"}, QDir::Dirs );
    for( QString dir : dirList )
    {
        if( dir == "." || dir == "..") continue;
        if( !compSetDir.cd( dir )    ) continue;

        loadComps( compSetDir );
        compSetDir.cd( ".." );
    }
}

void ComponentList::loadXml( QString xmlFile, bool convert )
{
    QFile file( xmlFile );
    if( !file.open(QFile::ReadOnly | QFile::Text) ){
          qDebug() << "ComponentList::loadXml Cannot read file"<< endl << xmlFile << endl << file.errorString();
          return;
    }
    QXmlStreamReader reader( &file );
    if( reader.readNextStartElement() )
    {
        if( reader.name() != "itemlib" ){
            qDebug() <<  "ComponentList::loadXml Error parsing file (itemlib):"<< endl << xmlFile;
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
                    icon = MainWindow::self()->getDataFilePath("images/"+icon);
            }

            QString catFull = reader.attributes().value("category").toString();
            catFull.replace( "IC 74", "Logic/IC 74");
            QStringList catPath = catFull.split("/");

            TreeItem* catItem = NULL;
            QString parent = "";
            QString category = "";
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
            QString folder = reader.attributes().value("folder").toString();

            while( reader.readNextStartElement() )
            {
                if( reader.name() == "item")
                {
                    QString name = reader.attributes().value("name").toString();

                    if( reader.attributes().hasAttribute("icon") )
                    {
                        icon = reader.attributes().value("icon").toString();
                        if( !icon.startsWith(":/") )
                            icon = MainWindow::self()->getDataFilePath("images/"+icon);
                    }
                    else icon = getIcon( folder, name );

                    if( convert )
                    {
                        qDebug() << "ComponentList::loadXml Corverting" << name;

                        QString info;
                        if( reader.attributes().hasAttribute("info") ) info = reader.attributes().value("info").toString();

                        QString iconData;
                        if( QFile::exists( icon ) )
                        {
                            QByteArray ba = fileToByteArray( icon, "ComponentList::loadXml");
                            QString data( ba.toHex() );
                            iconData = data;
                        }

                        QString compFolder = QFileInfo( xmlFile ).absolutePath()+"/"+folder;
                        QString compFile = compFolder+name+".comp";

                        QString compStr;

                        if( type == "Subcircuit" )
                        {
                            CircuitWidget::self()->loadCirc( compFolder+"/"+name+"/"+name+".sim1" );
                            compStr = Circuit::self()->circuitToString();


                        }
                        else if( type == "MCU" )
                        {
                            QString data = reader.attributes().value("data").toString();
                            QString pkge = reader.attributes().value("package").toString();

                            if( !category.startsWith("Micro") ) category = "Micro/"+category;

                            QString pkgFile = compFolder+pkge+".package";
                            if( !QFile::exists( pkgFile ) ) pkgFile = compFolder+pkge+"_LS.package";
                            compStr += Chip::convertPackage( fileToString( pkgFile, "ComponentList::convertItem" ) );

                            QString mcuFile = compFolder+data+".mcu";
                            compStr += convertMcuFile( mcuFile );
                        }
                        QString comp = "<libitem";
                        comp += " itemtype=\""+ type+"\"";
                        comp += " category=\""+ category  +"\"";
                        comp += " compname=\""+ name      +"\"";
                        comp += " compinfo=\""+ info      +"\"";
                        comp += " icondata=\""+ iconData  +"\"";
                        comp += ">\n\n";
                        comp += compStr;
                        comp += "</libitem>";

                        Circuit::self()->saveString( compFile, comp );

                    }
                    else if( catItem && !m_components.contains( name ) )
                    {
                        m_dataFileList[ name ] = xmlFile;   // Save xml File used to create this item
                        if( reader.attributes().hasAttribute("info") )
                            name += "???"+reader.attributes().value("info").toString();

                        addItem( name, catItem, icon, type );
                    }
                    reader.skipCurrentElement();
    }   }   }   }
    QString compSetName = xmlFile.split( "/").last();

    qDebug() << tr("        Loaded Component set:           ") << compSetName;
}

QString ComponentList::getIcon( QString folder, QString name )
{
    QString icon = folder+"/"+name+"/"+name+"_icon.png";
    if( m_compSetDir.exists( icon ) ) icon = m_compSetDir.absoluteFilePath( icon );
    else                              icon = "";
    return icon;
}

QString ComponentList::convertMcuFile( QString file )
{
    QString converted;
    QString folder = QFileInfo( file ).absolutePath();

    QStringList lines = fileToStringList( file, "ComponentList::convertItem" );
    for( QString line : lines )
    {
        if( line.contains("parts>") ) continue;
        if( line.startsWith("<!") ) continue;
        if( line.contains("<include") )
        {
            line = line.split("\"").at(1);
            line = convertMcuFile( folder+"/"+line );
        }
        converted.append( line+"\n" );
    }
    return converted;
}

void ComponentList::addItem( QString caption, TreeItem* catItem, QString icon, QString type )
{
    QPixmap ic( icon );
    QIcon ico( ic );
    addItem( caption, catItem, ico, type );
}

void ComponentList::addItem( QString caption, TreeItem* catItem, QIcon &icon, QString type )
{
    QStringList nameFull = caption.split( "???" );
    QString       nameTr = nameFull.first();
    QString info = "";
    if( nameFull.size() > 1 ) info = "   "+nameFull.last();

    QString name = ( type == "Subcircuit" || type == "MCU" ) ? nameTr : type;

    TreeItem* item = new TreeItem( catItem, name, nameTr, type, component, icon, m_customComp );

    item->setText( 0, nameTr+info );

    m_components.insert( name, item );

    if( m_insertItems )
    {
        catItem->addChild( item );

        bool hidden = MainWindow::self()->compSettings()->value( name+"/hidden" ).toBool();
        item->setItemHidden( hidden );

        QString shortCut = MainWindow::self()->compSettings()->value( name+"/shortcut" ).toString();
        item->setShortCut( shortCut );
        if( !shortCut.isEmpty() ) m_shortCuts[name] = shortCut;
    }
}

TreeItem* ComponentList::getCategory( QString category )
{
    TreeItem* catItem = NULL;
    if( m_categories.contains( category ) ) catItem = m_categories.value( category );
    else{
        category = m_catNames.value( category );
        if( !category.isEmpty() && m_categories.contains( category ) )
            catItem = m_categories.value( category );
    }
    return catItem;
}

TreeItem* ComponentList::addCategory( QString nameTr, QString name, QString parent, QString icon )
{
    TreeItem* catItem = nullptr;
    TreeItem* catParent = nullptr;

    bool expanded = false;

    if( parent.isEmpty() )                              // Is Main Category
    {
        catItem = new TreeItem( nullptr, name, nameTr, "", categ_MAIN, QIcon( QPixmap( icon ) )/*QIcon(":/null-0.png")*/, m_customComp );
        catItem->setExpanded( true );
        expanded = true;
    }else{
        if( m_categories.contains( parent ) ) catParent = m_categories.value( parent );
        catItem = new TreeItem( catParent, name, nameTr, "", categ_CHILD, QIcon( QPixmap( icon ) ), m_customComp );
    }

    catItem->setText( 0, nameTr );
    /// catItem->setData( 0, Qt::WhatsThisRole, name );
    m_categories.insert( name, catItem );
    m_catNames.insert( nameTr, name );
    if( m_insertItems )
    {
        if( parent.isEmpty() ) addTopLevelItem( catItem ); // Is root category or root category doesn't exist
        else if( catParent )   catParent->addChild( catItem );

        if( MainWindow::self()->compSettings()->contains(name+"/collapsed") )
            expanded = !MainWindow::self()->compSettings()->value( name+"/collapsed" ).toBool();

        catItem->setExpanded( expanded );
        catItem->setData( 0, Qt::UserRole+2, expanded );

        bool hidden = MainWindow::self()->compSettings()->value( name+"/hidden" ).toBool();
        catItem->setItemHidden( hidden );
    }
    return catItem;
}

void ComponentList::mousePressEvent( QMouseEvent* event )
{
    if( event->modifiers() & Qt::ControlModifier ) setDragDropMode( QAbstractItemView::InternalMove );
    else                                           setDragDropMode( QAbstractItemView::DragOnly );

    for( QTreeWidgetItem* item : selectedItems() ) item->setSelected( false );
    QTreeWidget::mousePressEvent( event );
}

void ComponentList::slotItemClicked( QTreeWidgetItem* item, int  )
{
    if( !item ) return;
    if( dragDropMode() == QAbstractItemView::InternalMove ) return; // Moving items in the list

    TreeItem* treeItem = (TreeItem*)item;
    QMimeData* mimeData = new QMimeData;
    mimeData->setText( treeItem->name()+","+treeItem->compType() );

    QDrag* drag = new QDrag( this );
    drag->setMimeData( mimeData );
    drag->exec( Qt::MoveAction, Qt::MoveAction );
}

void ComponentList::dropEvent( QDropEvent* event )
{
    QTreeWidget::dropEvent( event );

    for( TreeItem* catItem : m_categories )
    {
        if( catItem->parent() ) catItem->setItemType( categ_CHILD );
        else                    catItem->setItemType( categ_MAIN );
    }
}

void ComponentList::slotContextMenu( const QPoint& point )
{
    QMenu menu;

    QAction* manageComponents = menu.addAction( QIcon(":/fileopen.png"),tr("Manage Components") );
    connect( manageComponents, &QAction::triggered,
             this, &ComponentList::slotManageComponents, Qt::UniqueConnection );

    menu.exec( mapToGlobal(point) );
}

void ComponentList::slotManageComponents()
{
    m_mcDialog.initialize();
    m_mcDialog.setVisible( true );
}

void ComponentList::search( QString filter )
{
    QList<QTreeWidgetItem*>    cList = findItems( filter, Qt::MatchContains|Qt::MatchRecursive, 0 );
    QList<QTreeWidgetItem*> allItems = findItems( "", Qt::MatchContains|Qt::MatchRecursive, 0 );

    for( QTreeWidgetItem* item : allItems )
    {
        TreeItem* treeItem = (TreeItem*)item;
        treeItem->setHidden( true );

        if( treeItem->childCount() > 0  )
        {
            treeItem->setExpanded( treeItem->isItemExpanded() );
            continue;
        }
        if( !cList.contains( item ) ) continue;

        bool hidden = treeItem->isItemHidden();
        while( item ){
            item->setHidden( hidden );
            if( item->childCount() > 0 && !hidden && !filter.isEmpty() ) item->setExpanded( true );
            item = item->parent();
        }
    }
}

void ComponentList::insertItems()
{
    QDomDocument domDoc = fileToDomDoc( m_listFile, "ComponentList::insertItems" );
    if( domDoc.isNull() ) return;

    QDomElement root = domDoc.documentElement();
    QDomNode    tree = root.firstChild();
    insertItem( &domDoc, nullptr );               // Insert items as stored in file

    for( TreeItem* item : m_categories.values() ) // Insert new categories
    {
        TreeItem* parent = item->parentItem();
        if( parent ) parent->addChild( item );
        else         addTopLevelItem( item );
    }

    for( TreeItem* item : m_components.values() ) // Insert new components
    {
        TreeItem* catItem = item->parentItem();
        if( catItem ) catItem->addChild( item );
    }
}

void ComponentList::insertItem( QDomNode* node, TreeItem* parent )
{
    TreeItem* item = nullptr;
    bool expanded = false;

    QDomElement element = node->toElement();
    QString name = element.attribute("name");

    if( element.tagName() == "category" )
    {
        item = m_categories.value( name );

        if( item ){
            m_categories.remove( name );
            expanded = element.attribute("expanded") == "1";

            treItemType_t itemType = parent ? categ_CHILD : categ_MAIN;
            item->setItemType( itemType );
        }
    }
    else if( element.tagName() == "component" )
    {
        item = m_components.value( name );

        if( item ){
            m_components.remove( name );
            QString shortcut = element.attribute("shortcut");
            item->setShortCut( shortcut );
            m_shortCuts.insert( shortcut, name );
        }
    }

    if( item ){
        if( parent ) parent->addChild( item );
        else         addTopLevelItem( item );

        bool hidden = element.attribute("hidden") == "1";
        item->setItemHidden( hidden );
    }

    QDomNode child = node->firstChild(); // Recursively add items
    while( !child.isNull() ){
        insertItem( &child, item );
        child = child.nextSibling();
    }
    if( item ) item->setItemExpanded( expanded );
}

void ComponentList::writeSettings()
{
    QString treeStr = "<comptree>\n";

    QList<QTreeWidgetItem*> topLevelList = ComponentList::self()->findItems("",Qt::MatchStartsWith);

    for( QTreeWidgetItem* catItem : topLevelList )
    {
        TreeItem* childItem = (TreeItem*)catItem;
        treeStr += childItem->toString("  ");
    }
    treeStr += "</comptree>\n";

    Circuit::self()->saveString( m_listFile, treeStr );
}
