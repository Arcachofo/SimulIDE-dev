/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QFileDialog>

#include "pathval.h"
#include "component.h"
#include "comproperty.h"
#include "propdialog.h"
#include "mainwindow.h"
#include "circuit.h"
#include "utils.h"

PathVal::PathVal( PropDialog* parent, CompBase* comp, ComProperty* prop, bool isFile )
       : PropVal( parent, comp, prop )
{
    setupUi(this);

    m_isFile = isFile;
}
PathVal::~PathVal() {}

void PathVal::setup( bool )
{
    valLabel->setText( m_property->capt() );
    updtValues();
    this->adjustSize();
}

void PathVal::on_value_editingFinished()
{
    QString path = value->text();

    if( !m_isFile && !path.isEmpty() && !path.endsWith("/") && !path.endsWith("\\") ) path.append("/");
    m_property->setValStr( path );
    updtValues();
    m_propDialog->changed();
}

void PathVal::on_setPathButton_clicked()
{
    QString filePath = value->text();
    QDir circuitDir  = QFileInfo( Circuit::self()->getFilePath() ).absoluteDir();
    QString absPath  = circuitDir.absoluteFilePath( filePath );

    QString path;
    if( m_isFile ) path = QFileDialog::getOpenFileName( 0l, tr("Select file"), absPath );
    else           path = getDirDialog( tr("Select directory"), absPath );

    if( path.isEmpty() ) return;
    value->setText( path );
    on_value_editingFinished();
}

void PathVal::updtValues()
{
    QString text = m_property->getValStr();
    value->setText( text );
}
