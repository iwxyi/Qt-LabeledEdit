#include "bottomlineedit.h"

BottomLineEdit::BottomLineEdit(QWidget *parent) : QLineEdit(parent)
{
    setObjectName("BottomLineEdit");

    setStyleSheet("#BottomLineEdit { background: transparent; border: none;}");
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
