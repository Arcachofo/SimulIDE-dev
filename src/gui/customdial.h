#ifndef CUSTOMDIAL_H
#define CUSTOMDIAL_H

#include <QDial>

class CustomDial : public QDial
{
    Q_OBJECT
    public:
        CustomDial( QWidget* parent=NULL );

    private:
        virtual void paintEvent( QPaintEvent* e ) override;
};
#endif
