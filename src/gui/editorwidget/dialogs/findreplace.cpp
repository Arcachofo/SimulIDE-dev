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

#include "findreplace.h"
#include "codeeditor.h"
#include "basedebugger.h"

FindReplace::FindReplace( QWidget* parent )
           : QDialog( parent )
{
    setupUi(this);
    m_editor = NULL;
}

/*void FindReplace::on_findEdit_textEdited( QString text )
{

}

void FindReplace::on_replaceEdit_textEdited( QString text )
{

}*/

void FindReplace::on_prevButton_clicked() { find( false ); }
void FindReplace::on_nextButton_clicked() { find( true ); }

void FindReplace::on_allButton_clicked()
{
    //int pos = m_editor->textCursor().position();
    m_textCursor.setPosition( 0 );
    m_editor->setTextCursor( m_textCursor );
    QList<QTextEdit::ExtraSelection> extraSelections;

    int i=0;
    while( find( true ) )
    {
        QTextEdit::ExtraSelection extra;
        extra.cursor = m_editor->textCursor();
        extra.format.setBackground( QBrush(QColor(240,240,0)) );
        extraSelections.append( extra );
        i++;
    }
    m_editor->setExtraSelections( extraSelections );
    showMsg( tr("Found %1 occurrence(s)").arg(i) );
}

void FindReplace::on_replaceButton_clicked()
{
    if( m_editor->textCursor().hasSelection() )
        m_editor->textCursor().insertText( replaceEdit->text() );
}

void FindReplace::on_replFindButton_clicked()
{
    if( m_editor->textCursor().hasSelection() )
        m_editor->textCursor().insertText( replaceEdit->text() );
    find( true );
}

void FindReplace::on_replAllButton_clicked()
{
    m_textCursor.setPosition( 0 );
    m_editor->setTextCursor( m_textCursor );

    int i=0;
    while( find( true ) )
    {
        m_editor->textCursor().insertText( replaceEdit->text() );
        i++;
    }
    showMsg( tr("Replaced %1 occurrence(s)").arg(i) );
}

void FindReplace::on_closeButton_clicked()
{
    hide();
}

void FindReplace::on_caseS_toggled( bool c )
{

}

void FindReplace::on_whole_toggled( bool w )
{

}

void FindReplace::on_regexp_toggled( bool w )
{

}

bool FindReplace::find( bool next )
{
    if( !m_editor ) return false;

    bool result = false;
    QString toSearch = findEdit->text();

    QTextDocument::FindFlags flags;

    if( !next )               flags |= QTextDocument::FindBackward;
    if( caseS->isChecked() )  flags |= QTextDocument::FindCaseSensitively;
    if( whole->isChecked() )  flags |= QTextDocument::FindWholeWords;
    if( regexp->isChecked() )
    {
        QRegExp reg( toSearch, (caseS->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive) );

        m_textCursor = m_editor->document()->find( reg, m_textCursor, flags );
        m_editor->setTextCursor( m_textCursor );
        result = !m_textCursor.isNull();
    }
    else result = m_editor->find( toSearch, flags );

    return result;
}

void FindReplace::showMsg( QString msg )
{

}
