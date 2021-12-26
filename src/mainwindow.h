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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDir>

class ComponentSelector;
class CircuitWidget;
class EditorWindow;
class QPluginLoader;
class FileWidget;
class QVBoxLayout;
class QLineEdit;
class QSplitter;
class QSettings;

enum Langs {
    English = 0,
    Czech,
    Dutch,
    French,
    German,
    Italian,
    Russian,
    Spanish,
    Portuguese,
    Pt_Brasil,
    Turkish
};

class MAINMODULE_EXPORT MainWindow : public QMainWindow
{
    Q_OBJECT

    public:
        MainWindow();
        ~MainWindow();

 static MainWindow* self() { return m_pSelf; }

        QSettings* settings();

        void loadPlugins();
        void unLoadPugin( QString pluginName );
        
        void readSettings();
        
        void setTitle( QString title );

        QString loc();
        void setLoc( QString loc );

        Langs lang() { return m_lang; }
        void setLang( Langs lang );
        
        float fontScale() { return m_fontScale; }
        void setFontScale( float scale );

        int autoBck() { return m_autoBck; }
        void setAutoBck( int secs ) { m_autoBck = secs; }

        QString getFilePath( QString file );
        QString getConfigPath( QString file );

        QString getHelpFile( QString name );
        
        QTabWidget*  m_sidepanel;

    protected:
        void closeEvent( QCloseEvent* event );

    private slots:
        void searchChanged();

    private:
 static MainWindow* m_pSelf;
 
        void loadPluginsAt( QDir pluginsDir );

        Langs m_lang;

        bool m_blocked;

        void createWidgets();
        void createMenus();
        void createToolBars();
        void writeSettings();
        
        float m_fontScale;
        int m_autoBck;
        
        QDir m_filesDir;
        QDir m_configDir;

        QSettings* m_settings;
        
        QString m_version;
        QString m_styleSheet;

        QHash<QString, QPluginLoader*>  m_plugins;
        QHash<QString, QString>  m_help;
        
        CircuitWidget* m_circuit;
        ComponentSelector*  m_components;
        QWidget*      m_componentWidget;
        QVBoxLayout*  m_componentWidgetLayout;
        QLineEdit*    m_searchComponent;
        EditorWindow* m_editor;
        
        QSplitter*  m_Centralsplitter;
        FileWidget* m_fileSystemTree;
};

#endif
