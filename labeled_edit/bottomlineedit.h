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
    // 错误波浪线自绘文字，需要隐藏edit内容
    // 方案1：重写paintEvent
    // 优点：可以实时编辑内容
    // 缺点：全部隐藏了，若设置QSS样式，无法显示
    bool show_view = true;

    // 方案2：设置NoEcho
    // 优缺点与上面相反
    QLineEdit::EchoMode real_echo = EchoMode::Normal;
};

#endif // BOTTOMLINEEDIT_H
