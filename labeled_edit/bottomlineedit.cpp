#include "bottomlineedit.h"

BottomLineEdit::BottomLineEdit(QWidget *parent) : QLineEdit(parent)
{
    setObjectName("BottomLineEdit");

    setStyleSheet("#BottomLineEdit { background: transparent; border: none;}");
}

void BottomLineEdit::setViewShowed(bool show)
{
    /*if (!show)
    {
        this->real_echo = this->echoMode();
        this->setEchoMode(QLineEdit::EchoMode::NoEcho);
    }
    else
    {
        this->setEchoMode(this->real_echo);
    }*/
    this->show_view = show;
    update();
}

void BottomLineEdit::focusInEvent(QFocusEvent *e)
{
    QLineEdit::focusInEvent(e);
    emit signalFocusIn();
}

void BottomLineEdit::focusOutEvent(QFocusEvent *e)
{
    QLineEdit::focusOutEvent(e);
    emit signalFocusOut();
}

void BottomLineEdit::paintEvent(QPaintEvent *e)
{
    if (!show_view)
        return ;
    QLineEdit::paintEvent(e);
}
