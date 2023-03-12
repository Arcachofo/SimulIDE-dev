/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef EDITORWINDOW_H
#define EDITORWINDOW_H

#include "editorwidget.h"
#include "updatable.h"
#include "basedebugger.h"

enum bebugState_t{
    DBG_STOPPED = 0,
    DBG_PAUSED,
    DBG_STEPING,
    DBG_RUNNING
};

class Compiler;

class EditorWindow : public EditorWidget, public Updatable
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

        BaseDebugger* debugger() { return m_debugger; }

        virtual void updateStep();

        bool debugStarted() { return (m_state > DBG_STOPPED); }
        void lineReached( codeLine_t line );
        void pauseAt( codeLine_t line );

        bebugState_t debugState() { return m_state; }

        BaseDebugger* createDebugger(QString name, CodeEditor* ce, QString code="" );

        QStringList compilers() { return m_compilers.keys(); }
        QStringList assemblers() { return m_assemblers.keys(); }

        OutPanelText* outPane() { return &m_outPane; }

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

        void initDebbuger();

    private:
 static EditorWindow*  m_pSelf;
        bool uploadFirmware( bool debug );
        void stepDebug( bool over=false );
        void stopDebbuger();

        void loadCompilers();
        void loadCompilerSet( QString compilsPath, QMap<QString, compilData_t>* compList );

        CodeEditor*   m_debugDoc;
        BaseDebugger* m_debugger;
        bool m_stepOver;
        //bool m_driveCirc;
        bool m_updateScreen;
        bool m_jumpToBrk;
        int m_lastCycle;
        double m_lastTime;

        QString m_debugFile;
        int     m_debugLine;

        bebugState_t m_state;
        bebugState_t m_resume;

        QFont m_font;

        QMap<QString, compilData_t> m_compilers;
        QMap<QString, compilData_t> m_assemblers;
};

#endif
