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

#ifndef MEMDATA_H
#define MEMDATA_H

class MemTable;
class eMcu;

class MAINMODULE_EXPORT MemData
{
    public:
        MemData();
        ~MemData();

        static bool loadData( QVector<int>* toData, bool resize=false, int bits=8 );
        static void saveData( QVector<int>* data, int bits=8 );

        static bool loadDat( QVector<int>* toData, QString file, bool resize );
        static bool loadHex( QVector<int>* toData, QString file, bool resize, int bits );
        static bool loadHexMcu( QVector<int>* toData, QString file, int bits, eMcu* eMcu );
        static bool loadBin( QVector<int>* toData, QString fileName, bool resize, int bits );

        static QString getMem( QVector<int>* data );
        static void setMem( QVector<int>* data, QString m );

    public slots:
        virtual void showTable( int dataSize=256, int wordBytes=1 );

    protected:
        MemTable* m_memTable;
        static eMcu* m_eMcu;

        static void saveDat( QVector<int>* data, int bits );
        static void saveHex( QVector<int>* data, int bits ); /// TODO
        static void saveBin( QVector<int>* data, int bits );
};

#endif


