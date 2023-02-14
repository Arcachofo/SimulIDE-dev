#ifndef CUSTOMBUTTON_H
#define CUSTOMBUTTON_H

#include <QToolButton>

class CustomButton : public QToolButton
{
    Q_OBJECT
    public:
        CustomButton( QWidget* parent=NULL );

    private:
        virtual void paintEvent( QPaintEvent* ) override;
};
#endif
