/***************************************************************************
 *   Copyright (C) 2012 by santiago González                               *
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

#include <QDomDocument>
#include <QString>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QTextStream>
#include <qpoint.h>
#include <QPointF>
#include <cmath>

#include "mainwindow.h"
#include "utils.h"
#include "pin.h"

QHash<QString, double> multipliers = {{"p",1e-12},{"n",1e-9},{"µ",1e-6},{"m",1e-3},{"k",1e3},{"M",1e6},{"G",1e9}};

double getMultiplier( QString mult )
{
    mult = mult.remove(" ");
    mult = mult.left(1);
    double multiplier = multipliers.value( mult );
    if( multiplier == 0 ) multiplier = 1;
    return multiplier;
}

QString multToValStr( double value, QString mult )
{
    return QString::number( value*getMultiplier( mult ) );
}

QString val2hex( int d )
{
    QString Hex="0123456789ABCDEF";
    QString h = Hex.mid(d&15,1);
    while(d>15)
    {
        d >>= 4;
        h = Hex.mid( d&15,1 ) + h;
    }
    return h;
}

QString decToBase( int value, int base, int digits )
{
    QString converted = "";
    for( int i=0; i<digits; i++ )
    {
        if( value >= base ) converted = val2hex( value%base ) + converted;
        else                converted = val2hex( value ) + converted;

        if( i+1 == 4 ) converted = " " + converted;
        //if( (i+1)%8 == 0 ) converted = " " + converted;

        value = floor( value/base );
    }
    return converted;
}

//---------------------------------------------------

void MessageBoxNB( const QString &title, const QString &message )
{
    QMessageBox* msgBox = new QMessageBox( MainWindow::self() );
    msgBox->setAttribute( Qt::WA_DeleteOnClose ); //makes sure the msgbox is deleted automatically when closed
    msgBox->setStandardButtons( QMessageBox::Ok );
    msgBox->setWindowTitle( title );
    msgBox->setText( message );
    msgBox->setModal( false ); 
    msgBox->open();
}

//---------------------------------------------------

QString addQuotes( const QString &string )
{
    return "\""+string+"\"";
}
QString remQuotes( QString string )
{
    string = string.remove( 0, 1 );
    string = string.remove( string.size()-1, 1);
    return string;
}

QString getBareName( const QString &filepath ) // File name without extension
{
    return QFileInfo( filepath ).completeBaseName();
}
QString getFileName( const QString &filepath ) // Filename with extension
{
    return QFileInfo( filepath ).fileName();
}
QString getFileDir( const QString &filepath ) // File directory with end "/"
{
    return QFileInfo( filepath ).absolutePath()+"/";
}
QString getFileExt( const QString &filepath ) // File extension with "."
{
    return "."+QFileInfo( filepath ).suffix();
}
QString changeExt( const QString &filepath, const QString &ext )
{
    return getFileDir( filepath )+getBareName( filepath )+ext;
}

//---------------------------------------------------

QDomDocument fileToDomDoc( const QString &fileName, const QString &caller )
{
    QDomDocument domDoc;

    QFile file( fileName );
    if( !file.open( QFile::ReadOnly | QFile::Text) )
    {
        MessageBoxNB( caller, "Cannot read file:\n"+fileName+"\n"+file.errorString() );
        return domDoc;
    }
    QString error;
    int errorLine=0;
    int errorColumn=0;
    if( !domDoc.setContent( &file, false, &error, &errorLine, &errorColumn ) )
    {
         MessageBoxNB( caller, "Cannot set file to DomDocument:\n\n"+fileName );
         qDebug() << caller << "Cannot set file to DomDocument:\n"<<fileName<<"\nLine"<<errorLine<<errorColumn+"\n";
         qDebug() << error;
         domDoc.clear();
    }
    file.close();
    return domDoc;
}

QString fileToString( const QString &fileName, const QString &caller )
{
    QFile file( fileName );
    if (!file.open( QFile::ReadOnly | QFile::Text) )
    {
        MessageBoxNB( caller, "Cannot read file "+fileName+"\n"+file.errorString() );
        return "";
    }
    QTextStream in(&file);
    in.setCodec("UTF-8");
    QString text = in.readAll();
    file.close();

    return text;
}

QStringList fileToStringList( const QString &fileName, const QString &caller )
{
    return fileToString( fileName, caller ).split('\n');
}

QByteArray fileToByteArray( const QString &fileName, const QString &caller )
{
    QByteArray ba;

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        MessageBoxNB( caller, "Cannot read file "+fileName+"\n"+file.errorString() );
        return ba;
    }
    ba = file.readAll();
    file.close();

    return ba;
}

//--------------------------------------------------------------

int roundDown( int x, int roundness )
{
    if( x < 0 ) return (x-roundness+1) / roundness;
    else        return (x / roundness);
}

int snapToGrid( int x )     { return roundDown( x+2, 4 )*4; }
int snapToCompGrid( int x ) { return roundDown( x+4, 8 )*8; }

QPointF toCompGrid( QPointF point )
{
    int valor;
    valor = snapToCompGrid( (int)point.x() );
    point.rx() = (float)valor;
    valor = snapToCompGrid( (int)point.y() );
    point.ry() = (float)valor;
    return point;
}
QPointF togrid( QPointF point )
{
    int valor;
    valor = snapToGrid( (int)point.x() );
    point.rx() = (float)valor;
    valor = snapToGrid( (int)point.y() );
    point.ry() = (float)valor;
    return point;
}
QPoint togrid( QPoint point )
{
    int valor;
    valor = snapToGrid( (int)point.x() );
    point.rx() = valor;
    valor = snapToGrid( (int)point.y() );
    point.ry() = valor;
    return point;
}

bool lessPinX( Pin* pinA, Pin* pinB )
{
    return pinA->x() < pinB->x();
}

bool lessPinY( Pin* pinA, Pin* pinB )
{
    return pinA->y() < pinB->y();
}

/*QPointF getPointF( QString p )
{
    QStringList plist = p.split(",");
    QPointF point( plist.first().toDouble(), plist.last().toDouble() );
    return point;
}
QString getStrPointF( QPointF p )
{
    return QString::number(p.x())+","+QString::number(p.y());
}*/
