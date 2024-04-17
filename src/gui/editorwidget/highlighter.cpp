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

QStringList Highlighter::readSyntaxFile( QString fileName )
{
    QStringList keyWords;
    QString path = MainWindow::self()->getDataFilePath("codeeditor/syntax/");

    if( !QDir( path ).exists() ) return keyWords;

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
                    for( QString exp : words )  // Keywords
                    {

                        if( exp.startsWith("\"")) exp = remQuotes( exp ); // RegExp
                        else{
                            if( exp.length() > 2 ) keyWords.append( exp );
                            exp = "\\b"+exp+"\\b";
                        }
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

    return keyWords;
}

void Highlighter::addObjects( QStringList patterns )
{
    m_objectRules.clear();
    QTextCharFormat f;
    f.setFontWeight( QFont::Bold );
    f.setForeground( QColor( 0, 120, 70 ) );
    
    for( QString exp : patterns ) m_objectRules.append( HighlightRule{ QRegExp( "\\b"+exp+"\\b"), f } );

    this->rehighlight();
}

void Highlighter::addMembers( QStringList patterns )
{
    m_memberRules.clear();
    QTextCharFormat f;
    f.setFontWeight( QFont::Bold );
    f.setForeground( QColor( 0, 95, 160 ) );

    for( QString exp : patterns )
    {
        m_memberRules.append( HighlightRule{ QRegExp( "\\b"+exp+"\\b"), f } );
    }

    this->rehighlight();
}

void Highlighter::highlightBlock( const QString &text )
{
    QString lcText = text;
    lcText = lcText.toLower(); // Do case insensitive

    for( const HighlightRule &rule : m_rules ) processRule( rule, lcText );
    for( const HighlightRule &rule : m_objectRules ) processRule( rule, text );
    for( const HighlightRule &rule : m_memberRules ) processRule( rule, text );

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
    m_rules.append( rule );
}
