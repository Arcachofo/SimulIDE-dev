/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QApplication>
#include <QTranslator>
#include <QStandardPaths>
#include <QtGui>

#include "mainwindow.h"
#include "circuitwidget.h"

void myMessageOutput( QtMsgType type, const QMessageLogContext &context, const QString &msg )
{
    const char* file = context.file ? context.file : "";
    const char* func = context.function ? context.function : "";
    const char* mesg = msg.toLocal8Bit().constData();
    switch (type) {
    case QtDebugMsg:
        if( CircuitWidget::self() ) CircuitWidget::self()->simDebugMessage( msg );
        fprintf( stderr, "%s \n", mesg );
        break;
    case QtInfoMsg:
        fprintf(stderr, "Info: %s (%s:%u, %s)\n", mesg, file, context.line, func );
        break;
    case QtWarningMsg:
        fprintf(stderr, "Warning: %s (%s:%u, %s)\n", mesg, file, context.line, func );
        break;
    case QtCriticalMsg:
        fprintf(stderr, "Critical: %s (%s:%u, %s)\n", mesg, file, context.line, func );
        break;
    case QtFatalMsg:
        fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", mesg, file, context.line, func );
        break;
    }
}

QString langFile( QString locale )
{
    QString langF = ":/simulide_"+locale+".qm";

    QFile file( langF );
    if( !file.exists() ) langF = "";

    return langF;
}

int main( int argc, char *argv[] )
{
    qInstallMessageHandler( myMessageOutput );

    QApplication app( argc, argv );

    QSettings settings( QStandardPaths::standardLocations( QStandardPaths::DataLocation).first()+"/simulide.ini",  QSettings::IniFormat, 0l );

    QString locale = QLocale::system().name();
    if( settings.contains( "language" ) ) locale = settings.value( "language" ).toString();

    QString langF = langFile( locale );
    if( langF == "" )
    {
        locale = QLocale::system().name().split("_").first();
        langF = langFile( locale );
    }
    if( langF == "" ) langF = ":/simulide_en.qm";

    QTranslator translator;
    translator.load( langF );
    app.installTranslator( &translator );
    app.setApplicationVersion( APP_VERSION );

    MainWindow window;
    window.setLoc( locale );
    window.show();

    if( argc > 1 )
    {
        QString circ = QString::fromStdString( argv[1] );

        if( circ.endsWith(".sim1") )
        {
            QString file = "file://";
            if( circ.startsWith( file ) ) circ.replace( file, "" ).replace("\r\n", "" ).replace("%20", " ");
        #ifdef _WIN32
            if( circ.startsWith( "/" )) circ.remove( 0, 1 );
        #endif
            QTimer::singleShot( 300, CircuitWidget::self()
                              , [circ]()->void{ CircuitWidget::self()->loadCirc( circ ); } );
        }
    }

    return app.exec();
}

