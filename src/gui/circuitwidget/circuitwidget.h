/***************************************************************************
 *   Copyright (C) 2010 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef CIRCUITWIDGET_H
#define CIRCUITWIDGET_H

#include <QMenu>
#include <QToolBar>
#include <QVBoxLayout>

#include "circuitview.h"
#include "outpaneltext.h"

class QSplitter;
class QLabel;
class AboutDialog;
class AppDialog;

class MAINMODULE_EXPORT CircuitWidget : public QWidget
{
    Q_OBJECT

    public:
        CircuitWidget( QWidget *parent );
        ~CircuitWidget();

 static CircuitWidget* self() { return m_pSelf; }

        void clear();

        void setRate( double rate , int load );
        void setError( QString error );
        void setMsg(QString msg , int type);
        void powerCircOn();
        void powerCircOff();
        void powerCircDebug( bool paused );

        void simDebug( QString msg ) { m_outPane.appendLine( msg.remove("\"") ); }

        QSplitter* splitter() { return m_splitter; }
        
    public slots:
        bool newCircuit();
        void openRecentFile();
        void openCirc();
        void loadCirc( QString path );
        void saveCirc( QString file );
        void saveCirc();
        void saveCircAs();
        void powerCirc();
        void pauseSim();
        void settApp();
        void openInfo();
        void about();

    signals:
        void dataAvailable( int uart, const QByteArray &data );
        void saving();
        
    private:
        void createActions();
        void createToolBars();

 static CircuitWidget*  m_pSelf;

        enum { MaxRecentFiles = 10 };
        void updateRecentFiles();
        void updateRecentFileActions();

        bool m_blocked;

        QVBoxLayout  m_verticalLayout;
        CircuitView  m_circView;
        OutPanelText m_outPane;
        
        QToolBar m_circToolBar;
        QLabel*  m_rateLabel;
        QLabel*  m_msgLabel;
        QSplitter* m_splitter;

        QAction* recentFileActs[MaxRecentFiles];
        QAction* newCircAct;
        QAction* openCircAct;
        QAction* saveCircAct;
        QAction* saveCircAsAct;
        QAction* powerCircAct;
        QAction* pauseSimAct;
        QAction* settAppAct;
        QAction* infoAct;
        QAction* aboutAct;
        QAction* aboutQtAct;
        
        QMenu m_fileMenu;
        QMenu m_infoMenu;
        
        QString m_curCirc;
        QString m_lastCircDir;

        AppDialog*   m_appPropW;
        AboutDialog* m_about;
};

#endif
