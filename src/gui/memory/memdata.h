/***************************************************************************
 *   Copyright (C) 2018 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

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

        static bool loadFile( QVector<int>* toData, QString file, bool resize, int bits, eMcu* eMcu=NULL );
        static bool loadDat( QVector<int>* toData, QString file, bool resize );
        static bool loadHex( QVector<int>* toData, QString file, bool resize, int bits );
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


