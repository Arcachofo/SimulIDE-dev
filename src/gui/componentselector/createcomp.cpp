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

    m_iconList <<":/null_ico.png"<<":/subc_ico.png"<<":/subcl.png"<<":/subc2.png"<<":/ic2.png"
               <<":/perif.png"<<":/1to1.png"<<":/1to2.png"<<":/1to3.png"<<":/2to1.png"<<":/2to2.png"
               <<":/2to3.png"<<":/2to3g.png"<<":/3to1.png"<<":/3to2.png"<<":/3to2g.png"
               <<":/demux.png"<<":/mux.png"<<":/script_ico.png"<<":/shield.png"<<":/7segbcd.png";
    for( QString icon : m_iconList ) iconBox->addItem( QIcon( icon ), "");

    if( m_iconData.isEmpty() ) iconBox->setCurrentIndex( 1 );
    else                       addIcon();
}

void creCompDialog::accept()
{
    int index = iconBox->currentIndex();
    m_iconFile = m_iconList.at( index );
    updtIconData();

    QString comp = "<libitem";
    comp += " itemtype=\""+ typeBox->currentText()+"\"";
    comp += " category=\""+ categoryEdit->text()  +"\"";
    comp += " name=\""    + nameEdit->text()      +"\"";
    comp += " info=\""    + infoEdit->text()      +"\"";
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

    QString path = m_iconFile;
    if( path.isEmpty() ) path = m_circuitPath;

    QString iconFile = QFileDialog::getOpenFileName( 0l, tr("Select icon"), path,
                                          tr("png Files (*.png);;All files (*.*)"));

    if( iconFile.isEmpty() ) return;
    m_iconFile = iconFile;

    int index = 0;

    if( m_iconList.contains( m_iconFile ) )
    {
        index = m_iconList.indexOf( m_iconFile );
    }else{
        updtIconData();
        QPixmap ic( m_iconFile );
        QIcon ico( ic );

        iconBox->addItem( ico, "" );
        iconBox->setCurrentIndex( iconBox->count()-1 );
    }

    iconBox->setCurrentIndex( index );
}

void creCompDialog::addIcon()
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

    iconBox->addItem( ico, "" );
    iconBox->setCurrentIndex( iconBox->count()-1 );
}

void creCompDialog::updtIconData()
{
    QByteArray ba = fileToByteArray( m_iconFile, "creCompDialog::accept");
    QString iconData( ba.toHex() );
    m_iconData = iconData;
}
