/***************************************************************************
 *   Copyright (C) 2021 by santiago González                               *
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

#ifndef COMPILER_H
#define COMPILER_H

#include <QString>
#include <QProcess>

class OutPanelText;

class Compiler : public QObject
{
    Q_OBJECT

    public:
        Compiler( QObject* parent, OutPanelText* outPane, QString filePath);
        ~Compiler();

        void clearCompiler();
        void loadCompiler( QString file );
        virtual int compile( bool debug );

        QString compName() { return m_compName; }

        QString compilerPath() { return m_toolPath; }
        void setCompilerPath( QString path );
        void getCompilerPath();

        QString includePath() { return m_inclPath; }
        void setIncludePath( QString path );
        void getIncludePath();

        QString device() { return m_device; }
        void setDevice( QString d ) { m_device = d; }
        bool useDevice() { return m_useDevice; }

        void readSettings();

    protected:
        int runStep( QString fullCommand );
        QString replaceData( QString str );
        QString getPath( QString msg );
        void toolChainNotFound();

        bool m_useDevice;

        QString m_compName;
        QString m_toolPath;
        QString m_inclPath;
        QStringList m_command;
        QStringList m_arguments;
        QStringList m_argsDebug;
        QStringList m_fileList;

        QString m_debugMode;
        QString m_device;
        QString m_firmware;
        QString m_file;
        QString m_fileDir;
        QString m_fileName;
        QString m_fileExt;

        QProcess m_compProcess;

        OutPanelText* m_outPane;
};

#endif
