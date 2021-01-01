/***************************************************************************
 *   Copyright (C) 2018 by santiago González                               *
 *   santigoro@gmail.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.  *
 *                                                                         *
 ***************************************************************************/

#include "propertieswidget.h"
#include "mainwindow.h"


PropertiesWidget::PropertiesWidget( QWidget* parent )
                : QWidget( parent )
{
    setWindowFlags( Qt::Window | Qt::WindowTitleHint | Qt::Tool
                  | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint );

    createWidgets();
}

PropertiesWidget::~PropertiesWidget()
{
}

void PropertiesWidget::createWidgets()
{
    QGridLayout* widgetLayout = new QGridLayout( this );
    widgetLayout->setSpacing(0);
    widgetLayout->setContentsMargins(0, 0, 0, 0);
    widgetLayout->setObjectName( "widgetLayout" );
    
    QSplitter* splitter0 = new QSplitter( this );
    splitter0->setObjectName("splitter0");
    splitter0->setOrientation( Qt::Vertical );
    //splitter0->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    widgetLayout->addWidget( splitter0 );

    m_properties = new QPropertyEditorWidget( this );
    m_properties->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    m_help = new QPlainTextEdit( this );
    
    splitter0->addWidget( m_properties );
    splitter0->addWidget( m_help );
    QList<int> sizes;
    sizes<<180<<120;
    splitter0->setSizes( sizes );
    this->resize( 270, 450 );

    m_help->setPlainText( "" );
    m_help->setReadOnly( true );

    //this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);


    //QObject::connect(m_help, &QTextBrowser::anchorClicked, this, &PropertiesWidget::openUrl );
    //[](const QUrl & link) { QDesktopServices::openUrl(link); });
}

/*void PropertiesWidget::openUrl( const QUrl &link )
{
    qDebug() << "PropertiesWidget::openUrl"<<link;
    QDesktopServices::openUrl(link);
}*/

void PropertiesWidget::setHelpText( QString* text )
{
    if( !text ) return;
    m_help->clear();
    m_help->setPlainText( *text );
}

#include  "moc_propertieswidget.cpp"

