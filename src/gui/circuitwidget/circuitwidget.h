/***************************************************************************
 *   Copyright (C) 2010 by santiago González                               *
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

#ifndef CIRCUITWIDGET_H
#define CIRCUITWIDGET_H

#include <QtWidgets>

#include "circuitview.h"
#include "outpaneltext.h"

class AppProp;
class CircProp;
class SimuProp;
class AboutDialog;

class MAINMODULE_EXPORT CircuitWidget : public QWidget
{
    Q_OBJECT

    public:
        CircuitWidget( QWidget *parent );
        ~CircuitWidget();

 static CircuitWidget* self() { return m_pSelf; }

        void clear();
        
        void createActions();
        void createToolBars();
        
        void setRate( double rate , int load );
        void setError( QString error );
        void setMsg(QString msg , int type);
        void powerCircOn();
        void powerCircOff();
        void powerCircDebug( bool paused );

        void simDebug( QString msg );

        QSplitter* splitter() { return m_splitter; }
        
    public slots:
        bool newCircuit();
        void openRecentFile();
        void openCirc();
        void loadCirc( QString path );
        void saveCirc();
        bool saveCircAs();
        void powerCirc();
        void pauseSim();
        void settApp();
        void settCir();
        void settSim();
        void openInfo();
        void about();

    signals:
        void dataAvailable( int uart, const QByteArray &data );
        void saving();
        
    private:

 static CircuitWidget*  m_pSelf;

        enum { MaxRecentFiles = 10 };
        void updateRecentFileActions();

        QVBoxLayout  m_verticalLayout;
        //QHBoxLayout  m_horizontLayout;
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
        QAction* settCirAct;
        QAction* settSimAct;
        QAction* infoAct;
        QAction* aboutAct;
        QAction* aboutQtAct;
        
        QMenu m_fileMenu;
        QMenu m_settingsMenu;
        QMenu m_infoMenu;
        
        QString m_curCirc;
        QString m_lastCircDir;

        AppProp*  m_appPropW;
        CircProp* m_cirPropW;
        SimuProp* m_simPropW;
        AboutDialog* m_about;
};

#endif
