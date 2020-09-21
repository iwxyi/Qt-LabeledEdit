#ifndef BOTTOMLINEEDIT_H
#define BOTTOMLINEEDIT_H

#include <QObject>
#include <QLineEdit>

class BottomLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    BottomLineEdit(QWidget* parent = nullptr);

    void setViewShowed(bool show);

protected:
    void focusInEvent(QFocusEvent *e) override;
    void focusOutEvent(QFocusEvent *e) override;
    void paintEvent(QPaintEvent *e) override;

signals:
    void signalFocusIn();
    void signalFocusOut();

private:
    bool show_view = true;
};

#endif // BOTTOMLINEEDIT_H
