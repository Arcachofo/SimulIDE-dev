/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "about.h"

AboutDialog::AboutDialog( QWidget* parent )
           : QDialog( parent )
{
    setupUi(this);

    version->setText( QString( APP_VERSION )+" at Rev "+QString( REVNO ) );
    compiled->setText( QString( BUILDDATE )+" (dd-MM-yy)" );
    icon->setPixmap( QPixmap(":/simulide.png") );
}
