/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QtGui>

#include "highlighter.h"
#include "mainwindow.h"
#include "thememanager.h"
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
    bool dark = ThemeManager::self()->isDark();
    if( dark )                                  // Search for file_dark.syntax
    {
        QString darkFile = fileName;
        darkFile.replace(".syntax","_dark.syntax");
        if( QFile::exists( darkFile ) ){
            fileName = darkFile;
            dark = false;
        }
    }
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
                    uint colVal = first.remove("#").toUInt( &ok, 16 );
                    if( ok ){
                        QColor color( colVal );
                        if( dark ){
                            int h,s,l;
                            color.getHsl( &h, &s, &l);
                            color.setHsl( h, s, 255-l );
                        }
                        format.setForeground( color );
                    }
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
    format.setForeground( QColor(0xBBBBBB) ); // Show Spaces color
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
    
    for( QString exp : patterns ) m_objectRules.append( HighlightRule{ QRegularExpression( "\\b"+exp+"\\b"), f } );

    this->rehighlight();
}

void Highlighter::setMembers( QStringList patterns )
{
    m_memberRules.clear();
    addMembers( patterns );
}

void Highlighter::addMembers( QStringList patterns )
{
    QTextCharFormat f;
    f.setFontWeight( QFont::Bold );
    f.setForeground( QColor( 0, 95, 160 ) );

    for( QString exp : patterns ) m_memberRules.append( HighlightRule{ QRegularExpression( "\\b"+exp+"\\b"), f } );

    this->rehighlight();
}

void Highlighter::setExtraTypes( QStringList patterns )
{
    m_extraRules.clear();
    QTextCharFormat f;
    f.setFontWeight( QFont::Bold );
    f.setForeground( QColor( 0x904020 ) );

    for( QString exp : patterns ) m_extraRules.append( HighlightRule{ QRegularExpression( "\\b"+exp+"\\b"), f } );

    this->rehighlight();
}

void Highlighter::highlightBlock( const QString &text )
{
    QString lcText = text;
    lcText = lcText.toLower(); // Do case insensitive

    for( const HighlightRule &rule : m_objectRules ) processRule( rule, text );
    for( const HighlightRule &rule : m_memberRules ) processRule( rule, text );
    for( const HighlightRule &rule : m_extraRules  ) processRule( rule, text );
    for( const HighlightRule &rule : m_rules       ) processRule( rule, lcText );

    if( m_multiline )                              // Multiline comment:
    {
        setCurrentBlockState( 0 );
        int startIndex = 0;

        if( previousBlockState() != -10 ) {
            QRegularExpressionMatch match = m_multiStart.match( text );
            startIndex = match.hasMatch() ? match.capturedStart() : -1;
        }

        while( startIndex >= 0 )
        {
            QRegularExpressionMatch endMatch = m_multiEnd.match(text, startIndex);
            int endIndex = endMatch.hasMatch() ? endMatch.capturedStart() : -1;
            int commentLength;
            if( endIndex == -1 )
            {
                setCurrentBlockState( -10 );
                commentLength = text.length()- startIndex;
            }else{
                commentLength = endIndex - startIndex + endMatch.capturedLength();
            }
            setFormat( startIndex, commentLength, m_multiFormat );
            //startIndex = m_multiStart.indexIn( text, startIndex + commentLength );
            QRegularExpressionMatch nextMatch = m_multiStart.match(text, startIndex + commentLength);
            startIndex = nextMatch.hasMatch() ? nextMatch.capturedStart() : -1;
}   }   }

void Highlighter::processRule( HighlightRule rule, QString lcText )
{
    //QRegularExpression expression( rule.pattern );
    //int index = expression.indexIn( lcText );
    //while( index >= 0 )
    //{
    //    int length = expression.matchedLength();
    //    setFormat( index, length, rule.format );
    //    index = expression.indexIn( lcText, index + length );
    //}

    QRegularExpressionMatchIterator it = rule.pattern.globalMatch( lcText );

    while( it.hasNext() )
    {
        QRegularExpressionMatch match = it.next();
        setFormat( match.capturedStart(), match.capturedLength(), rule.format );
    }
}

void Highlighter::addRule( QTextCharFormat format, QString exp )
{
    HighlightRule rule;

    rule.pattern = QRegularExpression( exp );
    rule.format = format;
    m_rules.append( rule );
}
