/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef INODEBUGGER_H
#define INODEBUGGER_H

#include "avrgccdebugger.h"

class InoDebugger : public AvrGccDebugger
{
    Q_OBJECT
    Q_PROPERTY( board_t Board        READ board       WRITE setBoard       DESIGNABLE true USER true )
    Q_PROPERTY( QString Custom_Board READ customBoard WRITE setCustomBoard DESIGNABLE true USER true )
    public:
        InoDebugger( CodeEditor* parent, OutPanelText* outPane );
        ~InoDebugger();
        
        enum board_t {
            Uno = 0,
            Mega,
            Nano,
            Duemilanove,
            Leonardo,
            Custom
        };
        Q_ENUM( board_t )
        
        QString customBoard() { return m_customBoard; }
        void setCustomBoard( QString b ){ m_customBoard = b; }
        
        board_t board() { return m_Ardboard; }
        void setBoard( board_t b ){ m_Ardboard = b; }

        virtual QString toolPath() { return m_arduinoPath; }
        virtual void setToolPath( QString path ) override;

        virtual bool upload() override;
        virtual int  compile( bool debug ) override;

    protected:
        virtual bool postProcess() override;
        
    private:
        QString getBoard();
        virtual void setBoardName( QString board ) override;

        int m_version;
        int m_lastInoLine;
        int m_loopInoLine;
        
        QString m_arduinoPath;
        QString m_builder;
        QString m_sketchBook;
        QString m_customBoard;
        board_t m_Ardboard;
        QStringList m_ArdboardList;
};

#endif
