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

#include <QtGui>

#include "highlighter.h"
#include "utils.h"

Highlighter::Highlighter( QTextDocument* parent )
           : QSyntaxHighlighter( parent )
{ 
    m_multiline = false;
}
Highlighter::~Highlighter(){}

void Highlighter::readSintaxFile( const QString &fileName )
{
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
        if( previousBlockState() != 1 )
            startIndex = m_multiStart.indexIn( text );

        while( startIndex >= 0 )
        {
            int endIndex = m_multiEnd.indexIn( text, startIndex );
            int commentLength;
            if( endIndex == -1 )
            {
                setCurrentBlockState( 1 );
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
