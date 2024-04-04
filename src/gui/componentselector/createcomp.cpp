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
    categoryEdit->setText( Circuit::self()->category() );
    m_iconData = Circuit::self()->iconData();

    if( m_circuitPath.endsWith(".comp") ) buttonBox->button( QDialogButtonBox::Cancel )->hide();

    if( m_iconData.isEmpty() ){
        m_iconFile =":/subc_ico.png";
        embedIcon();
    }
    else updtIconData();
}

void creCompDialog::accept()
{
    QString comp = "<libitem";
    comp += " itemtype=\""+ typeBox->currentText()+"\"";
    comp += " category=\""+ categoryEdit->text()  +"\"";
    comp += " icondata=\""+ m_iconData            +"\"";
    comp += ">\n\n";

    comp += Circuit::self()->circuitToString();
    comp += "</libitem>";

    QFileInfo info( m_circuitPath );

    const QString dir = info.path()+"/"+info.baseName()+".comp";
    QString fileName = QFileDialog::getSaveFileName( this, tr("Save Copmponent"), dir,
                                                     tr("Components (*.comp);;All files (*.*)") );
    if( fileName.isEmpty() ) return;

    if( !fileName.endsWith(".comp") ) fileName.append(".comp");

    Circuit::self()->saveString( fileName, comp );
    //this->hide();
}

void creCompDialog::reject()
{
    Circuit::self()->cancelComp();
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
    QString iconData( ba.toHex() );
    m_iconData = iconData;

    QPixmap ic( m_iconFile );
    QIcon ico( ic );
    iconImage->setIcon( ico );
}

void creCompDialog::updtIconData()
{
    QByteArray ba;
    bool ok;
    for( int i=0; i<m_iconData.size(); i+=2 )
    {
        QString ch = m_iconData.mid( i, 2 );
        ba.append( ch.toInt( &ok, 16 ) );
    }
    QPixmap ic;
    ic.loadFromData( ba );
    QIcon ico( ic );
    iconImage->setIcon( ico );
}
