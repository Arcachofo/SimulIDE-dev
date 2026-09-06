/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "docpage.h"
#include "codeeditor.h"
#include "editorwindow.h"

DocPage::DocPage( CodeEditor* editor, QWidget* parent )
        : QWidget( parent )
        , m_editor( editor )
{
    m_changedBar = new QFrame( this );
    m_changedBar->setObjectName( "fileChangedBar" );
    m_changedBar->hide();
    m_changedBar->setStyleSheet(
        "QFrame#fileChangedBar { background-color: #C00000; }"
        "QFrame#fileChangedBar QLabel { color: #FFFF99; }" );

    QHBoxLayout* barLayout = new QHBoxLayout( m_changedBar );
    barLayout->setContentsMargins( 6, 2, 6, 2 );

    m_changedLabel = new QLabel( m_changedBar );
    m_changedLabel->setWordWrap( true );
    barLayout->addWidget( m_changedLabel, 1 );

    m_reloadBtn = new QPushButton( tr("Reload"), m_changedBar );
    m_reloadBtn->setToolTip( tr("Reload the file from disk") );
    barLayout->addWidget( m_reloadBtn );

    m_keepBtn = new QPushButton( tr("Keep Current"), m_changedBar );
    m_keepBtn->setToolTip( tr("Keep the current editor content") );
    barLayout->addWidget( m_keepBtn );

    m_closeBtn = new QPushButton( tr("Close Tab"), m_changedBar );
    m_closeBtn->setToolTip( tr("Close this tab") );
    barLayout->addWidget( m_closeBtn );

    m_keepOpenBtn = new QPushButton( tr("Keep Open"), m_changedBar );
    m_keepOpenBtn->setToolTip( tr("Keep the tab open without the file") );
    barLayout->addWidget( m_keepOpenBtn );

    connect( m_reloadBtn,    &QPushButton::clicked, this, &DocPage::reloadBar );
    connect( m_keepBtn,      &QPushButton::clicked, this, &DocPage::keepBar );
    connect( m_closeBtn,     &QPushButton::clicked, this, &DocPage::closeTabBar );
    connect( m_keepOpenBtn,  &QPushButton::clicked, this, &DocPage::keepOpenBar );

    QVBoxLayout* layout = new QVBoxLayout( this );
    layout->setContentsMargins( 0, 0, 0, 0 );
    layout->setSpacing( 0 );
    layout->addWidget( m_changedBar );
    layout->addWidget( m_editor, 1 );
}

void DocPage::showModifiedBar( QString path, bool docModified )
{
    QString msg = tr("File has been modified on disk:\n")+path+"\n";
    if( docModified ) msg += tr("The document has unsaved changes.\nReloading will discard them.\n");
    m_changedLabel->setText( msg );

    m_reloadBtn->show();
    m_keepBtn->show();
    m_closeBtn->hide();
    m_keepOpenBtn->hide();
    m_changedBar->show();
}

void DocPage::showDeletedBar( QString path )
{
    m_changedLabel->setText( tr("File no longer exists on disk:\n")+path );

    m_reloadBtn->hide();
    m_keepBtn->hide();
    m_closeBtn->show();
    m_keepOpenBtn->show();
    m_changedBar->show();
}

void DocPage::hideChangeBar()
{
    m_changedBar->hide();
}

void DocPage::reloadBar()
{
    EditorWindow::self()->reload();
    hideChangeBar();
}

void DocPage::keepBar()
{
    hideChangeBar();
}

void DocPage::closeTabBar()
{
    EditorWindow::self()->closePage( this );
}

void DocPage::keepOpenBar()
{
    hideChangeBar();
}
