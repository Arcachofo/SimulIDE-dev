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

#include <QTranslator>
#include <QFileDialog>
#include <QMessageBox>

#include "memdata.h"
#include "memtable.h"
#include "simulator.h"
#include "circuitwidget.h"
#include "circuit.h"
#include "utils.h"

MemData::MemData()
{
    m_memTable = NULL;
}
MemData::~MemData()
{
    if( !m_memTable ) return;

    m_memTable->setParent( NULL );
    m_memTable->deleteLater();
}

void MemData::showTable( int dataSize, int wordBytes )
{
    if( !m_memTable )
    {
        m_memTable = new MemTable( CircuitWidget::self(), dataSize, wordBytes );
        m_memTable->setWindowFlags( Qt::Window | Qt::WindowTitleHint | Qt::Tool
                                  | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint );
    }
    m_memTable->show();
}

bool MemData::loadData( QVector<int>* toData, bool resize, int bits )
{
    Simulator::self()->pauseSim();

    QString dir = changeExt( Circuit::self()->getFilePath(), ".data" );
    QString fileName = QFileDialog::getOpenFileName( NULL,
                                                    "MemData::loadData", dir,
                       QCoreApplication::translate( "MemData", "All files (*.*);;.data (*.data);;.bin (*.bin)"));

    if( fileName.isEmpty() ) return false; // User cancels loading

    if( resize ) toData->resize( 1 );

    QString ext = getFileExt( fileName );

    bool ok = false;
    if     ( ext == ".data" ) ok = loadDat( toData, fileName, resize );
    else if( ext == ".hex"
          || ext == ".ihx" )  ok = loadHex( toData, fileName, resize, bits ); // Intel Hex Format
    else                      ok = loadBin( toData, fileName, resize, bits ); // Binary Format
    Simulator::self()->resumeSim();

    return ok;
}

bool MemData::loadDat( QVector<int>* toData, QString file, bool resize )
{
    QStringList lines = fileToStringList( file, "MemData::loadTxt" );

    int addr = 0;
    int dataEnd = toData->size()-1;

    for( QString line : lines )
    {
        line = line.replace("\t", "").replace(" ", "");
        if( line.isEmpty() ) continue;

        QStringList words = line.split( "," );
        words.removeAll(QString(""));

        while( !words.isEmpty() )
        {
            QString sdata = words.takeFirst();
            bool ok = false;
            int data = sdata.toInt( &ok, 10 );
            if( !ok ) continue;

            if( resize ){
                dataEnd++;
                toData->resize( dataEnd+1 );
            }
            if( addr > dataEnd ) break;
            toData->replace( addr, data );
            addr++;
        }
        if( !words.isEmpty() && ( addr > dataEnd ) )
        {
            qDebug() << "\nMemData::loadDat: Data doesn't fit in Memory"<<dataEnd<<"\n";
            return false;
    }   }
    return true;
}

bool MemData::loadHex( QVector<int>* toData, QString file, bool resize, int bits )
{
    QStringList lineList = fileToStringList( file, "MemData::loadHex" );

    int nLine = 0;
    int addr;
    int nBytes;
    int type;
    int checksum;
    int hiByte;
    uint16_t data;
    int dataEnd = toData->size()-1;
    int WordSize = bits/8;

    bool ok;

    for( QString line : lineList )
    {
        checksum = 0;
        line = line.remove( " " );
        if( line.isEmpty() ) continue;

        if( !(line.left(1) == ":") ){
            qDebug() << "Error: Wrong Start code at line "+QString::number(nLine);
            return false;
        }
        line = line.remove( 0, 1 );

        nBytes = line.left( 2 ).toInt( &ok, 16 );
        int lineSize = 2+4+2+nBytes*2+2;
        if( line.size() != lineSize ){
            qDebug() << "Error: Wrong line size at line "+QString::number(nLine);
            return false;
        }
        checksum += nBytes;

        addr = line.mid( 2, 4 ).toInt( &ok, 16 );
        addr /= WordSize;
        checksum += line.mid( 2, 2 ).toInt( &ok, 16 );
        checksum += line.mid( 4, 2 ).toInt( &ok, 16 );

        type = line.mid( 6, 2 ).toInt( &ok, 16 );
        if     ( type == 1 ) return true; // Reached End Of File
        else if( type != 0 )
        {
            qDebug() <<"Warning: Not supported Record type:"<<type<<"at line "+QString::number(nLine);
            continue;
        }
        checksum += type;

        int i;
        for( i=8; i<8+nBytes*2; i+=2*WordSize )
        {
            data = line.mid( i, 2 ).toInt( &ok, 16 );
            checksum += data;

            if( WordSize == 2 ){
                hiByte = line.mid( i+2, 2 ).toInt( &ok, 16 );
                data += (hiByte<<8);
                checksum += hiByte;
            }
            if( resize ){
                dataEnd++;
                toData->resize( dataEnd+1 );
            }
            if( addr > dataEnd ){
                qDebug() << "Warning: PGM End reached at Line"+QString::number(nLine)
                         <<"\nAddress:"<<addr<<"is > PMG End:"<<dataEnd
                         <<"\nTODO: Config word ??";
                return true;
            }
            toData->replace( addr, data );
            addr++;
        }
        checksum += line.mid( i, 2 ).toInt( &ok, 16 );
        if( checksum & 0xFF ){
            qDebug() << "Error: CheckSum Error at line "+QString::number(nLine);
            return false;
        }
        nLine++;
    }
    qDebug() << "Error: No End Of File reached";
    return false;
}

bool MemData::loadBin( QVector<int>* toData, QString fileName, bool resize, int bits )
{
    int bytes = (bits+7)/8;
    int dataEnd = toData->size()-1;

    QByteArray ba = fileToByteArray( fileName, "MemData::loadData" );
    int memSize = ba.size()/bytes;

    if( resize )
    {
        dataEnd = memSize-1;
        toData->resize( memSize );
    }
    for( int i=0; i<memSize; i++ )
    {
        if( i > dataEnd ){
            qDebug() << "\nMemData::loadBin: Data doesn't fit in Memory"<<dataEnd<<"\n";
            return false;
        }
        int data = 0;
        for( int by=0; by<bytes; by++ )  // Join bytes little-endian
        {
            int val = ba.at( i*bytes+by );
            if( by>0 ) val <<= 8*by;
            data += val;
        }
        toData->replace( i, data );
    }
    return true;
}

void MemData::saveData( QVector<int>* data, int bits )
{
     Simulator::self()->pauseSim();

    QString dir = changeExt( Circuit::self()->getFilePath(), ".data" );

    QString fileName = QFileDialog::getSaveFileName( NULL,
                       QCoreApplication::translate( "MemData", "Save Data" ), dir,
                       QCoreApplication::translate( "MemData", "All files (*.*);;.data (*.data);;.bin (*.bin)"));

    if( fileName.isEmpty() ) return; // User cancels saving

    QFile outFile( fileName );
    int bytes = (bits+7)/8;
    int i = 0;

    if( fileName.endsWith(".data") )
    {
        QString output = "";
        for( int val : *data )
        {
            QString sval = QString::number( val );
            while( sval.length() < 4) sval.prepend( " " );
            output += sval;

            if( i == 15 ) { output += "\n"; i = 0; }
            else          { output += ",";  i++;  }
        }
        if( !outFile.open( QFile::WriteOnly | QFile::Text ) )
        {
             QMessageBox::warning(NULL, "MemData::saveData",
             QCoreApplication::translate( "MemData", "Cannot write file %1:\n%2.").arg(fileName).arg(outFile.errorString()));
        }else{
            QTextStream toFile( &outFile );
            toFile << output;
            outFile.close();
    }   }
    else                          // Binary Format
    {
        if( !outFile.open( QFile::WriteOnly ) )
        {
              QMessageBox::warning(NULL, "MemData::saveData",
              QCoreApplication::translate( "MemData", "Cannot write file %1:\n%2.").arg(fileName).arg(outFile.errorString()));
        }else{
            for( int val : *data ){
                for( int by=0; by<bytes; by++ ) // Separate bytes little-endian
                {
                    char byte = val & 0xFF;
                    val >>= 8;
                    outFile.write( &byte, 1 );
            }   }
            outFile.close();
    }   }
    Simulator::self()->resumeSim();
}

void MemData::saveDat( QVector<int>* data, int bits )
{

}

void MemData::saveHex( QVector<int>* data, int bits ) /// TODO
{

}

void MemData::saveBin( QVector<int>* data, int bits )
{

}

QString MemData::getMem( QVector<int>* data )
{
    QString m;
    int size = data->size();
    if( size > 0 )
    {
        bool empty = true;
        for( int i=size-1; i>=0; --i )
        {
            int val = data->at( i );
            if( val > 0 ) empty = false;
            if( empty ) continue;
            m.prepend( QString::number( val )+"," );
    }   }
    return m;
}

void MemData::setMem( QVector<int>* data, QString m )
{
    if( m.isEmpty() ) return;

    QStringList list = m.split(",");
    int i = 0;
    for( QString val : list )
    {
        if( i >= data->size() ) break;
        data->replace( i, val.toUInt() );
        i++;
}   }
