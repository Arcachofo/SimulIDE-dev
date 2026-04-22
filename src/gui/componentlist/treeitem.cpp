/***************************************************************************
 *   Copyright (C) 2024 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "treeitem.h"
#include "mainwindow.h"
#include "thememanager.h"

TreeItem::TreeItem( TreeItem* parent, QString name, QString nameTr, QString compType, treItemType_t itemType, QPixmap &pixmap, bool custom  )
        : QTreeWidgetItem()
{
    m_parent   = parent;
    m_name     = name;
    m_nameTr   = nameTr;
    m_compType = compType;
    m_isCustom = custom;
    m_itemType = type_NONE;

    m_shortcut = "";
    m_expanded = false;
    //m_hidden   = false;

    m_pixmap = pixmap;

    setIcon( 0, QIcon(pixmap) );
    setItemType( itemType );
}
TreeItem::~TreeItem(){}

void TreeItem::setItemType( treItemType_t itemType )
{
    if( m_itemType == itemType ) return;
    m_itemType = itemType;

    float scale = MainWindow::self()->fontScale();
    QFont font;
    font.setFamily( MainWindow::self()->defaultFontName() );
    font.setBold( true );

    if( itemType == component )
    {
        setFlags( QFlag( Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled ) );

        if( icon( 0 ).isNull() ) setSizeHint( 0, QSize( 100, 14*scale ) );
        font.setPixelSize( 11*scale );

        if( m_isCustom ) m_foreColor = 0xFF505A6E;
        else             m_foreColor = 0xFF645A3C;
    }
    else   // Is Category
    {
        setChildIndicatorPolicy( TreeItem::ShowIndicator );
        setFlags( QFlag( Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsDragEnabled ) );

        if( itemType == categ_MAIN )
        {
            setSizeHint( 0, QSize(100, 30*scale) );
            font.setPixelSize( 13*scale );

            if( m_isCustom ){
                m_foreColor = 0xFF323C50;
                m_backColor = 0xFFDCEBF0;
            }else{
                m_foreColor = 0xFF4B460A;
                m_backColor = 0xFFDCF0EB;
            }
        }
        else if( itemType == categ_CHILD )
        {
            if( icon( 0 ).isNull() ) setSizeHint( 0, QSize(100, 16*scale) );
            else                     setSizeHint( 0, QSize(100, 20*scale) );
            font.setPixelSize( 12*scale );

            if( m_isCustom ){
                m_foreColor = 0xFF465064;
                m_backColor = 0xFFE6F5FA;
            }else{
                m_foreColor = 0xFF5A5032;
                m_backColor = 0xFFE6FAF5;
            }
        }
    }
    setFont( 0, font );

    updateColors();
}

void TreeItem::setItemExpanded( bool e )
{
    m_expanded = e;
    setExpanded( e );
}

//void TreeItem::setItemHidden( bool h )
//{
//    m_hidden = h;
//    setHidden( h );
//}

void TreeItem::setTheme( bool dark )
{
    if( dark ) setIcon( 0, QIcon( ThemeManager::self()->invertPixmap(m_pixmap)) );
    else       setIcon( 0, QIcon( m_pixmap ) );

    updateColors( dark );
}

void TreeItem::updateColors( bool dark )
{
    setForeground( 0, dark ? QColor(m_foreColor).lighter(170) : QColor(m_foreColor) );
    if( m_itemType != component )
        setBackground( 0, dark ? QColor(m_backColor).darker(500)  : QColor(m_backColor) );
}

QString TreeItem::toString( QString indent )
{
    QString catStr;

    if( m_itemType > component ) catStr = indent+"<category ";
    else                         catStr = indent+"<component ";
    catStr += "name=\""    +m_name+"\" ";
    //catStr += "nametr=\""  +m_nameTr+"\" ";
    //catStr += "comptype=\""+m_compType+"\" ";
    catStr += "hidden=\""  +QString::number( isHidden() ? 1 : 0 )+"\" ";

    if( m_itemType > component )
        catStr += "expanded=\""+QString::number( isExpanded() ? 1 : 0 )+"\" > \n";
    else catStr += "shortcut=\""+m_shortcut+"\" />\n";

    for( int i=0; i<childCount(); ++i )
    {
        TreeItem* childItem = (TreeItem*)child( i );
        catStr += childItem->toString( indent+"  " );
    }
    if( m_itemType > component ) catStr += indent+"</category>\n";

    return catStr;
}
