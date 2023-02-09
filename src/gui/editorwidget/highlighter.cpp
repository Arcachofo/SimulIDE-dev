/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QtGui>

#include "highlighter.h"
#include "mainwindow.h"
#include "utils.h"

Highlighter::Highlighter( QTextDocument* parent )
           : QSyntaxHighlighter( parent )
{ 
    m_multiline = false;
}
Highlighter::~Highlighter(){}

void Highlighter::readSyntaxFile( QString fileName )
{
    QString path = MainWindow::self()->getUserFilePath("codeeditor/syntax/");
    if( path.isEmpty() || !QDir( path ).exists() )
        path = MainWindow::self()->getFilePath("data/codeeditor/syntax/");

    if( !QDir( path ).exists() ) return;

    fileName = path+fileName;

    m_rules.clear();

    QTextCharFormat format;

    QStringList rules;
    QStringList text = fileToStringList( fileName, "Highlighter" );

    while( !text.isEmpty() )                        // Iterate trough lines
    {
        QString line = text.takeFirst();

        if( line.isEmpty() ) continue;
        if( line.startsWith("rules:") )          // Find rule List
        {
            rules = line.split(" ");
            rules.removeFirst(); rules.removeAll(" "); rules.removeAll("");
            continue;
        }
        QStringList allWords = line.split(" ");
        allWords.removeAll(" "); allWords.removeAll("");
        if( allWords.isEmpty() ) continue;

        for( QString rule : rules )
        {
            QStringList words = allWords;
            QString first = words.takeFirst();

            if( !first.startsWith( rule ) ) continue; // Nothing found
            // Found rule
            if( first.endsWith("-style:") )     // Found Style definition
            {
                bool ok = false;

                first = words.takeFirst();          // Foregraund color
                if( first != "default" )
                {
                    uint color = first.remove("#").toUInt( &ok, 16 );
                    if( ok ) format.setForeground( QColor(color) );
                }
                first = words.takeFirst();          // Backgraund color
                if( first != "default" )
                {
                    uint color = first.remove("#").toUInt( &ok, 16 );
                    if( ok ) format.setBackground( QColor(color) );
                }
                first = words.takeFirst().toLower(); // Bold?
                if( first == "true" ) format.setFontWeight( QFont::Bold );

                first = words.takeFirst().toLower(); // Italic?
                if( first == "true" ) format.setFontItalic( true );
            }
            else{                                   // Is RegExp or word List
                if( first.contains( "multiLineComment" ) )
                {
                    m_multiline = true;
                    m_multiFormat = format;
                    if( words.size() > 1 )
                    {
                        QString exp = words.takeFirst();
                        exp = remQuotes( exp );
                        m_multiStart.setPattern( exp.replace("\\\\","\\") );
                        exp = words.takeFirst();
                        exp = remQuotes( exp );
                        m_multiEnd.setPattern( exp.replace("\\\\","\\")  );
                }   }
                else{
                    for( QString exp : words )
                    {
                        if( exp.startsWith("\"")) exp = remQuotes( exp ); // RegExp
                        else                      exp = "\\b"+exp+"\\b";
                        addRule( format, exp );
                }   }
                format.setFontWeight( QFont::Normal );         // Reset to Defaults
                format.setForeground( Qt::black );             // Reset to Defaults
            }
            break;
    }   }
    format.setForeground( QColor(12303291) ); // Show Spaces color
    addRule( format, QString( " " ) );
    addRule( format, QString( "\t" ) );
    
    this->rehighlight();
}

void Highlighter::addRegisters( QStringList patterns )
{
    QTextCharFormat format;
    format.setFontWeight( QFont::Bold );
    format.setForeground( QColor( 55, 65, 20 ) );
    
    for( QString exp : patterns ) addRule( format, "\\b"+exp+"\\b" );
    this->rehighlight();
}

void Highlighter::highlightBlock( const QString &text )
{
    QString lcText = text;
    lcText = lcText.toLower(); // Do case insensitive

    for( const HighlightRule &rule : m_rules ) processRule( rule, lcText );

    if( m_multiline )                              // Multiline comment:
    {
        setCurrentBlockState( 0 );
        int startIndex = 0;
        if( previousBlockState() != -10 )
            startIndex = m_multiStart.indexIn( text );

        while( startIndex >= 0 )
        {
            int endIndex = m_multiEnd.indexIn( text, startIndex );
            int commentLength;
            if( endIndex == -1 )
            {
                setCurrentBlockState( -10 );
                commentLength = text.length()- startIndex;
            }else{
                commentLength = endIndex - startIndex + m_multiEnd.matchedLength();
            }
            setFormat( startIndex, commentLength, m_multiFormat );
            startIndex = m_multiStart.indexIn( text, startIndex + commentLength );
}   }   }

void Highlighter::processRule( HighlightRule rule, QString lcText )
{
    QRegExp expression( rule.pattern );
    int index = expression.indexIn( lcText );
    while( index >= 0 )
    {
        int length = expression.matchedLength();
        setFormat( index, length, rule.format );
        index = expression.indexIn( lcText, index + length );
}   }

void Highlighter::addRule( QTextCharFormat format, QString exp )
{
    HighlightRule rule;

    rule.pattern = QRegExp( exp );
    rule.format = format;
    m_rules.append(rule);
}
