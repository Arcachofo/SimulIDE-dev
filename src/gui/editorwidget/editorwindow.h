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

#ifndef EDITORWINDOW_H
#define EDITORWINDOW_H

#include "editorwidget.h"

enum bebugState_t{
    DBG_STOPPED = 0,
    DBG_PAUSED,
    DBG_STEPING,
    DBG_RUNNING
};

class BaseDebugger;
class Compiler;

class EditorWindow : public EditorWidget
{
    Q_OBJECT

    public:
        EditorWindow( QWidget *parent );
        ~EditorWindow();

 static EditorWindow* self() { return m_pSelf; }

        struct compilData_t
        {
            QString file;
            QString type;
        };

        bool driveCirc() { return m_driveCirc; }
        void setDriveCirc( bool drive );

        BaseDebugger* debugger() { return m_debugger; }

        bool debugStarted() { return (m_state > DBG_STOPPED); }
        void lineReached( int line );

        bebugState_t debugState() { return m_state; }

        BaseDebugger* createDebugger(QString name, CodeEditor* ce, QString code="" );

        QStringList compilers() { return m_compilers.keys(); }
        QStringList assemblers() { return m_assemblers.keys(); }

    public slots:
        virtual void pause() override;
        virtual void stop() override;
        virtual void run() override;
        
    protected slots:
        virtual void debug() override;
        virtual void step() override;
        virtual void stepOver() override;
        virtual void reset() override;

        virtual bool upload();

    private:
 static EditorWindow*  m_pSelf;
        bool uploadFirmware( bool debug );
        bool initDebbuger();
        void stepDebug( bool over=false );
        void stopDebbuger();

        void loadCompilers();
        void loadCompilerSet( QString compilsPath, QMap<QString, compilData_t>* compList );

        CodeEditor* m_debugDoc;
        BaseDebugger* m_debugger;
        bool m_stepOver;
        bool m_driveCirc;
        int m_lastCycle;

        bebugState_t m_state;
        bebugState_t m_resume;

        QFont m_font;

        QMap<QString, compilData_t> m_compilers;
        QMap<QString, compilData_t> m_assemblers;
};

#endif
