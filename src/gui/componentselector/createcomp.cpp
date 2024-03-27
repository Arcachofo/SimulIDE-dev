/***************************************************************************
 *   Copyright (C) 2024 by santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QDebug>
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>

#include "createcomp.h"
#include "circuit.h"
#include "utils.h"

creCompDialog::creCompDialog( QWidget* parent )
             : QDialog( parent )
{
    setupUi( this );

    m_circuitPath = Circuit::self()->getFilePath();

    m_iconFile =":/ic_comp.png";
    embedIcon();
}

void creCompDialog::accept()
{
    QString comp = "<item";
    comp += " type=\""    + typeBox->currentText()+"\"";
    comp += " category=\""+ categoryEdit->text()  +"\"";
    comp += " icondata=\""+ m_iconData            +"\"";
    comp += ">";

    comp += Circuit::self()->circuitToComp();

    QFileInfo info( m_circuitPath );
    QString fileName = info.path()+"/"+info.baseName()+".comp";
    Circuit::self()->saveString( fileName, comp );
    this->hide();
}

void creCompDialog::reject()
{
    this->hide();
}

/*void creCompDialog::on_typeBox_currentIndexChanged( int index )
{
    qDebug() << "on_typeBox_currentIndexChanged";
}

void creCompDialog::on_categoryEdit_editingFinished()
{
    qDebug() << "on_categoryEdit_editingFinished";
}*/

void creCompDialog::on_iconChoose_clicked()
{
    //qDebug() << "on_iconChoose_clicked";

    QString iconFile = QFileDialog::getOpenFileName( 0l, tr("Select icon"), m_circuitPath,
                                          tr("png Files (*.png);;All files (*.*)"));

    if( iconFile.isEmpty() ) return;
    m_iconFile = iconFile;

    embedIcon();
}

/*void creCompDialog::on_iconImage_clicked()
{
    qDebug() << "on_iconImage_clicked";
}*/

void creCompDialog::embedIcon()
{
    QByteArray ba = fileToByteArray( m_iconFile, "creCompDialog::accept");
    QString iconData( ba.toHex(':') );
    m_iconData = iconData;

    QPixmap ic( m_iconFile );
    QIcon ico( ic );
    iconImage->setIcon( ico );
}
