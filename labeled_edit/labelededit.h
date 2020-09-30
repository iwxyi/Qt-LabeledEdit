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
    Q_PROPERTY(double LabelProg READ getFocusProg WRITE setLabelProg)
    Q_PROPERTY(int FocusProg READ getFocusProg WRITE setFocusProg)
    Q_PROPERTY(int LosesProg READ getLosesProg WRITE setLosesProg)
    Q_PROPERTY(int WrongProg READ getWrongProg WRITE setWrongProg)
    Q_PROPERTY(int CorrectProg READ getCorrectProg WRITE setCorrectProg)
    Q_PROPERTY(int ShowLoadingProg READ getShowLoadingProg WRITE setShowLoadingProg)
    Q_PROPERTY(int HideLoadingProg READ getHideLoadingProg WRITE setHideLoadingProg)
    Q_PROPERTY(int TipProg READ getTipProg WRITE setTipProg)
    Q_PROPERTY(int MsgShowProg READ getMsgShowProg WRITE setMsgShowProg)
    Q_PROPERTY(int MsgHideProg READ getMsgHideProg WRITE setMsgHideProg)
public:
    LabeledEdit(QWidget *parent = nullptr);
    LabeledEdit(QString label, QWidget* parent = nullptr);
    LabeledEdit(QString label, QString def, QWidget* parent = nullptr);

    BottomLineEdit* editor();
    void adjustBlank();

    void setLabelText(QString text);
    void setMessageText(QString text);
    void setMessageText(QString text, QColor color);
    void setTipText(QString text);
    void setTipText(QString text, QColor color);
    void setAccentColor(QColor color);

    void showCorrect();
    void hideCorrect();
    void showWrong();
    void showWrong(QString msg);
    void showLoading();
    void hideLoading();

private:
    void showTip();
    void hideTip();
    void showMsg();
    void hideMsg();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;

signals:

public slots:

private:
    QPropertyAnimation *startAnimation(QByteArray name, double start, double end, int duration, QEasingCurve curve = QEasingCurve::Linear);
    void setLabelProg(double x);
    double getLabelProg();
    void setFocusProg(int x);
    int getFocusProg();
    void setLosesProg(int x);
    int getLosesProg();
    void setWrongProg(int x);
    int getWrongProg();
    void setCorrectProg(int x);
    int getCorrectProg();
    void setShowLoadingProg(int x);
    int getShowLoadingProg();
    void setHideLoadingProg(int x);
    int getHideLoadingProg();
    void setTipProg(int x);
    int getTipProg();
    void setMsgShowProg(int x);
    int getMsgShowProg();
    void setMsgHideProg(int x);
    int getMsgHideProg();

private:
    BottomLineEdit* line_edit;
    QWidget* up_spacer;
    QWidget* down_spacer;

    QColor grayed_color;   // 没有聚焦的颜色：下划线+文字
    QColor accent_color;   // 终点颜色

    QString label_text;    // 标签
    QList<QPointF> label_in_poss; // 标签在输入框里面的左下角位置
    QList<QPointF> label_up_poss; // 标签在输入框上方的左下角位置
    QRect loading_rect;    // 加载菊花的位置

    QString tip_text;
    QColor tip_color;
    bool entering = false; // showWrong隐藏tip，用来做flag

    QString msg_text; // 警告信息
    QColor msg_color; // 警告颜色
    QString msg_hiding; // 隐藏中的msg，用于两次msg的切换

    QTimer* loading_timer = nullptr;
    int loading_petal = 8;    // 菊花花瓣数量
    double loading_inner = 0; // 菊花内环半径
    double loading_outer = 0; // 菊花外环半径
    int loading_index = 0; // 加载到了哪个花瓣（最右边为0）

    double label_prog = 0; // 标签上下移动
    int focus_prog = 0;    // 下划线从左往右
    int loses_prog = 0;    // 下划线从右边消失
    int wrong_prog = 0;    // 底部下划线浪动
    int correct_prog = 0;  // 右边的勾
    int show_loading_prog = 0;  // 显示加载
    int hide_loading_prog = 0;  // 隐藏加载
    int tip_prog = 0;
    int msg_show_prog = 0;
    int msg_hide_prog = 0;

    const int pen_width = 2;
    const double label_scale = 1.5;
    const int label_duration = 500;
    const int focus_duration = 500;
    const int wrong_duration = 900;
    const int correct_duration = 600;
    const int show_loading_duration = 600;
    const int hide_loading_duration = 200;
    const int tip_duration = 400;
    const int msg_show_duration = 600;
    const int msg_hide_duration = 300;
};

#endif // LABELEDEDIT_H
