/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDir>

class ComponentSelector;
class CircuitWidget;
class EditorWindow;
class FileWidget;
class QVBoxLayout;
class QLineEdit;
class QSplitter;
class QSettings;

enum Langs {
    English = 0,
    Chinese,
    Czech,
    Dutch,
    French,
    German,
    Italian,
    Russian,
    Spanish,
    Portuguese,
    Pt_Brasil,
    Slovak,
    Turkish
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

    public:
        MainWindow();
        ~MainWindow();

 static MainWindow* self() { return m_pSelf; }

        QSettings* settings();
        QSettings* compSettings();
        
        void readSettings();
        
        void setFile( QString file );
        void setState( QString state );

        QString loc();
        void setLoc( QString loc );

        Langs lang() { return m_lang; }
        void setLang( Langs lang );

        void setDefaultFontName( const QString& fontName );
        QString defaultFontName() { return m_fontName; }
        
        float fontScale() { return m_fontScale; }
        void setFontScale( float scale );

        int autoBck() { return m_autoBck; }
        void setAutoBck( int secs ) { m_autoBck = secs; }

        QString getHelp( QString name, bool save=true );

        QString getFilePath( QString file );              // Get file path in SimulIDE folders
        QString getConfigPath( QString file );            // Get file path in config folder
        QString getDataFilePath( QString file );          // Get file path in data folder, first user folder, if not SimulIDE folder
        QString getUserFilePath( QString f );             // Get file path in user folder
        QString userPath() { return m_userDir; }          // User folder path
        void setUserPath( QString p ) { m_userDir = p; }
        void getUserPath();                               // File open Dialog
        
        QTabWidget* m_sidepanel;

    protected:
        void closeEvent( QCloseEvent* event );

    private slots:
        void searchChanged();

    private:
 static MainWindow* m_pSelf;

        Langs m_lang;

        bool m_blocked;

        void createWidgets();
        void createMenus();
        void createToolBars();
        void writeSettings();
        
        QString m_fontName;
        float m_fontScale;
        int m_autoBck;
        
        QDir m_filesDir;
        QDir m_configDir;

        QSettings* m_settings;
        QSettings* m_compSettings;
        
        QString m_version;
        QString m_styleSheet;
        QString m_state;
        QString m_file;
        QString m_userDir;

        QHash<QString, QString> m_help;
        
        CircuitWidget* m_circuit;
        ComponentSelector* m_components;
        QWidget*      m_componentWidget;
        QVBoxLayout*  m_componentWidgetLayout;
        QLineEdit*    m_searchComponent;
        EditorWindow* m_editor;
        
        QSplitter*  m_Centralsplitter;
        FileWidget* m_fileSystemTree;
};

#endif
