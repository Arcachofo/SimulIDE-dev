/***************************************************************************
 *   Copyright (C) 2024 by santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QDebug>
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>

#include "createcomp.h"
#include "mainwindow.h"
#include "circuit.h"
#include "utils.h"

creCompDialog::creCompDialog( QWidget* parent )
             : QDialog( parent )
{
    setupUi( this );

    m_circuitPath = Circuit::self()->getFilePath();
    categoryEdit->setText( Circuit::self()->category() );
    nameEdit->setText( Circuit::self()->compName() );
    infoEdit->setText( Circuit::self()->compInfo() );
    QString iconData = Circuit::self()->iconData();

    if( m_circuitPath.endsWith(".comp") ) buttonBox->button( QDialogButtonBox::Cancel )->hide();

    QStringList iconList;
    iconList <<":/null_ico.png"<<":/subc_ico.png"<<":/subcl.png"<<":/subc2.png"<<":/ic2.png"
             <<":/perif.png"<<":/1to1.png"<<":/1to2.png"<<":/1to3.png"<<":/2to1.png"<<":/2to2.png"
             <<":/2to3.png"<<":/2to3g.png"<<":/3to1.png"<<":/3to2.png"<<":/3to2g.png"
             <<":/demux.png"<<":/mux.png"<<":/script_ico.png"<<":/shield.png"<<":/7segbcd.png";

    for( QString icon : iconList ) addIconFile( icon ); // iconBox->addItem( QIcon( icon ), "");

    if( iconData.isEmpty() ) iconBox->setCurrentIndex( 0 );
    else{
        iconItem_t iconItem;
        iconItem.iconData = iconData;
        addIcon( iconItem );
    }
}

void creCompDialog::accept()
{
    QString name = nameEdit->text();
    if( name.isEmpty() ){
        qDebug() << "creCompDialog::accept Error: Component name is empty";
        return;
    }
    int index = iconBox->currentIndex();
    QString iconData;
    if( index > 0 ) iconData = m_itemList.at( index ).iconData;

    QString comp = "<libitem";
    comp += " itemtype=\""+ typeBox->currentText()+"\"";
    comp += " category=\""+ categoryEdit->text()  +"\"";
    comp += " compname=\""+ name                  +"\"";
    comp += " compinfo=\""+ infoEdit->text()      +"\"";
    comp += " icondata=\""+ iconData              +"\"";
    comp += ">\n\n";

    comp += Circuit::self()->circuitToString();
    comp += "</libitem>";

    QString dir = m_circuitPath;
    if( dir.isEmpty() ) dir = MainWindow::self()->userPath();

    QFileInfo info( dir );
    dir = info.path()+"/"+name+".comp";
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

void creCompDialog::on_iconChoose_clicked()
{
    QString path = m_iconFile;
    if( path.isEmpty() ) path = m_circuitPath;

    QString iconFile = QFileDialog::getOpenFileName( 0l, tr("Select icon file"), path,
                                          tr("png Files (*.png);;All files (*.*)"));

    m_iconFile = iconFile;
    if( !iconFile.isEmpty() ) addIconFile( iconFile );
}

void creCompDialog::addIconFile( QString iconFile )
{
    iconItem_t iconItem;
    iconItem.iconFile = iconFile;

    QByteArray ba = fileToByteArray( iconFile, "creCompDialog::addIconFile");
    QString iconData( ba.toHex() );
    iconItem.iconData = iconData;

    addIcon( iconItem );
}

void creCompDialog::addIcon( iconItem_t iconItem )
{
    m_itemList.append( iconItem );

    QByteArray ba;
    bool ok;
    for( int i=0; i<iconItem.iconData.size(); i+=2 )
    {
        QString ch = iconItem.iconData.mid( i, 2 );
        ba.append( ch.toInt( &ok, 16 ) );
    }
    QPixmap ic;
    ic.loadFromData( ba );
    QIcon ico( ic );

    iconBox->addItem( ico, "" );
    iconBox->setCurrentIndex( iconBox->count()-1 );
}
