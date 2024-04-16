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
#include "circuit.h"    /// TODELETE
#include "circuitwidget.h"
#include "utils.h"

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
    //setStyleSheet("QTreeWidget::item { padding: 2px }");
    //setStyleSheet("background-color: #FDFDF8;");

    float scale = MainWindow::self()->fontScale();
    setIconSize( QSize( 30*scale, 24*scale ));
    //setIconSize( QSize( 36, 24 ));

    m_customComp = false;
    LoadLibraryItems();
    m_customComp = true;

    QString userDir = MainWindow::self()->userPath();
    if( !userDir.isEmpty() && QDir( userDir ).exists() ) LoadCompSetAt( userDir );

    for( QTreeWidgetItem* it : m_categories )
    {
        if( it->childCount() ) continue;
        QTreeWidgetItem* pa = it->parent();
        if( pa ) pa->removeChild( it  );
    }

    setContextMenuPolicy( Qt::CustomContextMenu );

    connect( this, &ComponentSelector::customContextMenuRequested,
             this, &ComponentSelector::slotContextMenu );

    connect( this, &ComponentSelector::itemPressed,
             this, &ComponentSelector::slotItemClicked );
}
ComponentSelector::~ComponentSelector(){}

void ComponentSelector::LoadLibraryItems()
{
    for( LibraryItem* item : m_itemLibrary.items() )
    {
        QString category = item->category();

        QString icon = item->iconfile();
        QString iconFile = MainWindow::self()->getDataFilePath("images/"+icon );
        if( !QFile::exists( iconFile ) ) iconFile = ":/"+icon; // Image not in simulide data folder, use hardcoded image

        if( item->createItemFnPtr() )
        {
            QTreeWidgetItem* catItem = getCategory( category );
            if( catItem ) addItem( item->name(), catItem, iconFile, item->type() );
        }
        else addCategory( item->name(), item->type(), category, iconFile );
    }
}

void ComponentSelector::LoadCompSetAt( QDir compSetDir )
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
            QTreeWidgetItem* catItem = getCategory("test");
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
                    m_components.append( compName );
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

void ComponentSelector::loadComps( QDir compSetDir )
{
    QStringList compList = compSetDir.entryList( {"*.comp"}, QDir::Files );

    for( QString compFile : compList )
    {
        compFile = compSetDir.absoluteFilePath( compFile );
        if( !compSetDir.exists( compFile ) ) continue;

        QFile file( compFile );
        if( !file.open(QFile::ReadOnly | QFile::Text) ){
              qDebug() << "ComponentSelector::loadComps Cannot read file"<< endl << compFile << endl << file.errorString();
              continue;
        }
        QFileInfo fi( compFile );
        QString compName = fi.baseName();

        QXmlStreamReader reader( &file );
        if( !reader.readNextStartElement() || reader.name() != "libitem" ){
            qDebug() << "ComponentSelector::loadComps Error parsing file (itemlib):"<< endl << compFile;
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
            if( !icon.isEmpty() ) ba = fileToByteArray( icon, "ComponentSelector::loadComps");
        }

        QPixmap ic;
        ic.loadFromData( ba );
        QIcon ico( ic );

        if( reader.attributes().hasAttribute("compname") )
            compName = reader.attributes().value("compname").toString();

        /// TODO: reuse get category from catPath
        QString category = reader.attributes().value("category").toString();
        QStringList catPath = category.split("/");

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
                catItem = addCategory( catTr, category, parent, "" );
            }
        }
        QString type = reader.attributes().value("itemtype").toString();

        if( !type.isEmpty() && !m_components.contains( compName ) )
        {
            m_components.append( compName );
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

void ComponentSelector::loadXml( QString setFile, bool convert )
{
    QFile file( setFile );
    if( !file.open(QFile::ReadOnly | QFile::Text) ){
          qDebug() << "ComponentSelector::loadXml Cannot read file"<< endl << setFile << endl << file.errorString();
          return;
    }
    QXmlStreamReader reader( &file );
    if( reader.readNextStartElement() )
    {
        if( reader.name() != "itemlib" ){
            qDebug() <<  "ComponentSelector::loadXml Error parsing file (itemlib):"<< endl << setFile;
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

            QTreeWidgetItem* catItem = NULL;
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
                        if( type == "Subcircuit" || type == "MCU" )
                        {
                            if( reader.attributes().hasAttribute("info") )
                                name += "???"+reader.attributes().value("info").toString();

                            if( type == "MCU" )
                            {
                                QString data = reader.attributes().value("data").toString();
                                if( !data.isEmpty() ) folder = data;
                            }

                            convertItem( folder, setFile, name, catFull, icon, type );
                        }
                    }
                    else if( catItem && !m_components.contains( name ) )
                    {
                        m_components.append( name );

                        m_dataFileList[ name ] = setFile;   // Save xml File used to create this item
                        if( reader.attributes().hasAttribute("info") )
                            name += "???"+reader.attributes().value("info").toString();

                        if( !convert ) addItem( name, catItem, icon, type );
                    }
                    reader.skipCurrentElement();
    }   }   }   }
    QString compSetName = setFile.split( "/").last();

    qDebug() << tr("        Loaded Component set:           ") << compSetName;
}

QString ComponentSelector::getIcon( QString folder, QString name )
{
    QString icon = folder+"/"+name+"/"+name+"_icon.png";
    if( m_compSetDir.exists( icon ) ) icon = m_compSetDir.absoluteFilePath( icon );
    else                              icon = "";
    return icon;
}

void ComponentSelector::convertItem( QString folder, QString itemFile, QString name, QString category, QString icon, QString type )
{
    qDebug() << "ComponentSelector::convertItem Corverting" << name;
    QStringList nameFull = name.split( "???" );
    QString info = "";
    if( nameFull.size() > 1 ) info = "   "+nameFull.last();
    name = nameFull.first();

    QString simFile;
    QString compFile;
    QString destFolder = QFileInfo( itemFile ).absolutePath();
    if( type == "MCU" )
    {
        simFile = destFolder+"/"+folder+".sim1";
        compFile = destFolder+"/"+folder+".comp";
        if( !category.startsWith("Micro") ) category = "Micro/"+category;
    }
    else{
        simFile = destFolder+"/"+folder+"/"+name+"/"+name+".sim1";
        compFile = destFolder+"/"+folder+"/"+name+".comp";
    }
    //qDebug() << simFile;
    //            return;
    QString iconData;
    if( QFile::exists( icon ) )
    {
        QByteArray ba = fileToByteArray( icon, "ComponentSelector::convertItem");
        QString data( ba.toHex() );
        iconData = data;
    }

    QString comp = "<libitem";
    comp += " itemtype=\""+ type+"\"";
    comp += " category=\""+ category  +"\"";
    comp += " compname=\""+ name      +"\"";
    comp += " compinfo=\""+ info      +"\"";
    comp += " icondata=\""+ iconData  +"\"";
    comp += ">\n\n";

    CircuitWidget::self()->loadCirc( simFile );
    comp += Circuit::self()->circuitToString();

    if( type == "MCU" )
    {
        QString mcuFile = destFolder+"/"+folder+".mcu";
        comp += convertMcuFile( mcuFile );
    }
    comp += "</libitem>";

    Circuit::self()->saveString( compFile, comp );
}

QString ComponentSelector::convertMcuFile( QString file )
{
    QString converted;
    QString folder = QFileInfo( file ).absolutePath();

    QStringList lines = fileToStringList( file, "ComponentSelector::convertItem" );
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

void ComponentSelector::addItem( QString caption, QTreeWidgetItem* catItem, QString icon, QString type )
{
    QPixmap ic( icon );
    QIcon ico( ic );
    addItem( caption, catItem, ico, type );
}

void ComponentSelector::addItem( QString caption, QTreeWidgetItem* catItem, QIcon &icon, QString type )
{
    QStringList nameFull = caption.split( "???" );
    QString       nameTr = nameFull.first();
    QString info = "";
    if( nameFull.size() > 1 ) info = "   "+nameFull.last();

    QTreeWidgetItem* item = new QTreeWidgetItem(0);
    float scale = MainWindow::self()->fontScale();
    QFont font;
    font.setFamily( MainWindow::self()->defaultFontName() );
    font.setBold( true );
    font.setPixelSize( 11*scale );

    item->setFlags( QFlag(32) );
    item->setFont( 0, font );
    item->setIcon( 0, icon );
    item->setText( 0, nameTr+info );
    item->setData( 0, Qt::UserRole, type );
    if( icon.isNull() ) item->setSizeHint( 0, QSize(100, 14*scale) );
    //else                item->setSizeHint( 0, QSize(100, 24) );
    if( m_customComp ) item->setTextColor( 0, QColor( 80, 90, 110 ) );
    else               item->setTextColor( 0, QColor( 100, 90, 60 ) );

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
    font.setFamily( MainWindow::self()->defaultFontName() );
    font.setBold( true );
    float fontScale = MainWindow::self()->fontScale();
    bool expanded = false;

    if( parent.isEmpty() )                              // Is Main Category
    {
        catItem = new QTreeWidgetItem( this );
        catItem->setIcon( 0, QIcon(":/null-0.png") );
        if( m_customComp ){
            catItem->setTextColor( 0, QColor( 50, 60, 80 ) );
            catItem->setBackground( 0, QBrush(QColor(220, 235, 240)) );
        }else{
            catItem->setTextColor( 0, QColor( 75, 70, 10 ) );
            catItem->setBackground( 0, QBrush(QColor(220, 240, 235)) );
        }
        catItem->setSizeHint( 0, QSize(100, 30*fontScale) );
        font.setPixelSize( 13*fontScale );
        expanded = true;
    }else{
        catItem = new QTreeWidgetItem(0);
        catItem->setIcon( 0, QIcon( QPixmap( icon ) ) );
        if( m_customComp ){
            catItem->setTextColor( 0, QColor( 70, 80, 100 ) );
            catItem->setBackground( 0, QBrush(QColor( 230, 245, 250)) );
        }else{
            catItem->setTextColor( 0, QColor( 90, 80, 50 ) );
            catItem->setBackground( 0, QBrush(QColor( 230, 250, 245)) );
        }
        if( icon.isEmpty() ) catItem->setSizeHint( 0, QSize(100, 16*fontScale) );
        else                 catItem->setSizeHint( 0, QSize(100, 20*fontScale) );
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
