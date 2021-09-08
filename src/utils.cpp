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
#include <QFile>
#include <QMessageBox>
#include <QTextStream>
#include <qpoint.h>
#include <QPointF>
#include <cmath>

#include "utils.h"
#include "pin.h"

void MessageBoxNB( const QString &title, const QString &message )
{
    QMessageBox* msgBox = new QMessageBox( 0l );
    msgBox->setAttribute( Qt::WA_DeleteOnClose ); //makes sure the msgbox is deleted automatically when closed
    msgBox->setStandardButtons( QMessageBox::Ok );
    msgBox->setWindowTitle( title );
    msgBox->setText( message );
    msgBox->setModal( false ); 
    msgBox->open();
}

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

QDomDocument fileToDomDoc( const QString &fileName, const QString &caller )
{
    QDomDocument domDoc;

    QFile file( fileName );
    if( !file.open( QFile::ReadOnly | QFile::Text) )
    {
        MessageBoxNB( caller, "Cannot read file:\n"+fileName+":\n"+file.errorString() );
        return domDoc;
    }
    QString error;
    int errorLine;
    if( !domDoc.setContent( &file, &error, &errorLine ) )
    {
         MessageBoxNB( caller, "Cannot set file to DomDocument:\n\n"+fileName );
         qDebug() << caller << "Cannot set file to DomDocument:\n"<<fileName<<":\nLine"<<errorLine+"\n";
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
        MessageBoxNB( caller, "Cannot read file "+fileName+":\n"+file.errorString() );
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
        MessageBoxNB( caller, "Cannot read file "+fileName+":\n"+file.errorString() );
        return ba;
    }
    ba = file.readAll();
    file.close();

    return ba;
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
        if( value >= base ) converted = val2hex(value%base) + converted;
        else                converted = val2hex(value) + converted;

        if( i+1 == 4 ) converted = " " + converted;
        //if( (i+1)%8 == 0 ) converted = " " + converted;

        value = floor( value/base );
    }
    return converted;
}

int roundDown( int x, int roundness )
{
    if( x < 0 ) return (x-roundness+1) / roundness;
    else        return (x / roundness);
}

int roundDown( float x, int roundness ) { return roundDown( int(x), roundness ); }

QPoint roundDown( const QPoint & p, int roundness )
{
    return QPoint( roundDown( p.x(), roundness ), roundDown( p.y(), roundness ) );
}

int snapToGrid( int x ) { return roundDown( x+2, 4 )*4; }

int snapToCompGrid( int x ) { return roundDown( x+4, 8 )*8; }

QPointF togrid( QPointF point )
{
    int valor;
    valor = snapToGrid( (int)point.x() );
    point.rx() = (float)valor;
    valor = snapToGrid( (int)point.y() );
    point.ry() = (float)valor;
    return point;
}

QPointF toCompGrid( QPointF point )
{
    int valor;
    valor = snapToCompGrid( (int)point.x() );
    point.rx() = (float)valor;
    valor = snapToCompGrid( (int)point.y() );
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

int getAlignment( QPointF p1, QPointF p2 )
{
    int align = 0;
    if( p1.x() == p2.x() ) align += 2;           // Aligned in Y axis
    if( p1.y() == p2.y() ) align += 1;           // Aligned in X axis

    return align;
}

#include "pin.h"
bool lessPinX( Pin* pinA, Pin* pinB )
{
    return pinA->x() < pinB->x();
}

bool lessPinY( Pin* pinA, Pin* pinB )
{
    return pinA->y() < pinB->y();
}

