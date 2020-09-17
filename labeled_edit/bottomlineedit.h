#ifndef BOTTOMLINEEDIT_H
#define BOTTOMLINEEDIT_H

#include <QObject>
#include <QLineEdit>

class BottomLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    BottomLineEdit(QWidget* parent = nullptr);

protected:
    void focusInEvent(QFocusEvent *e) override;
    void focusOutEvent(QFocusEvent *e) override;

signals:
    void signalFocusIn();
    void signalFocusOut();
};

#endif // BOTTOMLINEEDIT_H
