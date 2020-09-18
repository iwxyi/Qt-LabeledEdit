#ifndef LABELEDEDIT_H
#define LABELEDEDIT_H

#include <QObject>
#include <QWidget>
#include <QVBoxLayout>
#include <QPropertyAnimation>
#include <QPainter>
#include <QPainterPath>
#include <QTimer>
#include <cmath>
#include <QDebug>
#include "bottomlineedit.h"

class LabeledEdit : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int LabelProg READ getFocusProg WRITE setLabelProg)
    Q_PROPERTY(int FocusProg READ getFocusProg WRITE setFocusProg)
    Q_PROPERTY(int LosesProg READ getLosesProg WRITE setLosesProg)
    Q_PROPERTY(int WrongProg READ getWrongProg WRITE setWrongProg)
    Q_PROPERTY(int CorrectProg READ getCorrectProg WRITE setCorrectProg)
public:
    LabeledEdit(QWidget *parent = nullptr);

    void setLabelText(QString text);
    void setAccentColor(QColor color);

    BottomLineEdit* getEdit();
    void adjustBlank();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *) override;

signals:

public slots:

private:
    QPropertyAnimation *startAnimation(QByteArray name, QVariant start, QVariant end, int duration, QEasingCurve curve = QEasingCurve::Linear);
    void setLabelProg(int x);
    int getLabelProg();
    void setFocusProg(int x);
    int getFocusProg();
    void setLosesProg(int x);
    int getLosesProg();
    void setWrongProg(int x);
    int getWrongProg();
    void setCorrectProg(int x);
    int getCorrectProg();

private:
    BottomLineEdit* line_edit;
    QSpacerItem* label_spacer;

    QColor grayed_color; // 没有聚焦的颜色：下划线+文字
    QColor accent_color; // 终点颜色

    QString label_text;      // 标签
    QList<QPointF> label_in_poss; // 标签在输入框里面的左下角位置
    QList<QPointF> label_up_poss; // 标签在输入框上方的左下角位置

    int label_prog = 0;   // 标签上下移动
    int focus_prog = 0;   // 下划线从左往右
    int loses_prog = 0;   // 下划线从右边消失
    int wrong_prog = 0;   // 底部下划线浪动
    int correct_prog = 0; // 右边的勾

    const int pen_width = 2;
    const double label_scale = 1.5;
    const int label_duration = 500;
    const int focus_duration = 350;
    const int wrong_duration = 400;
    const int correct_duration = 300;

};

#endif // LABELEDEDIT_H
