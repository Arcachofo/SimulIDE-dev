/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef INODEBUGGER_H
#define INODEBUGGER_H

#include <QMap>

#include "avrgccdebugger.h"

class InoDebugger : public AvrGccDebugger
{
    public:
        InoDebugger( CodeEditor* parent, OutPanelText* outPane );
        ~InoDebugger();
        
        QString getBoard() { return m_board; }
        void setBoard( QString board );

        QString customBoard() { return m_customBoard; }
        void setCustomBoard( QString b ){ m_customBoard = b; }

        virtual QString toolPath() override { return m_arduinoPath; }
        virtual void setToolPath( QString path ) override;

        virtual bool upload() override;
        virtual int  compile( bool debug ) override;

        virtual void compilerProps() override;

    protected:
        virtual bool postProcess() override;
        
    private:
        int m_version;
        int m_lastInoLine;
        int m_loopInoLine;
        
        QString m_arduinoPath;
        QString m_builder;
        QString m_sketchBook;
        QString m_customBoard;
        QString m_board;
        QString m_defaultBoards;
        QStringList m_userBoards;

        QMap<QString, QString> m_boardMap;
};

#endif
