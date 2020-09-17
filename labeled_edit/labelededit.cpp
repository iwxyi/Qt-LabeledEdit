#include "labelededit.h"

LabeledEdit::LabeledEdit(QWidget *parent) : QWidget(parent)
{
    line_edit = new BottomLineEdit(this);
    label_spacer = new QSpacerItem(0, 0);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addSpacerItem(label_spacer);
    layout->addWidget(line_edit);

    adjustBlank();

    connect(line_edit, &BottomLineEdit::signalFocusIn, this, [=]{
        connect(startAnimation("FocusProg", getFocusProg(), 100, focus_duration, QEasingCurve::OutQuad), &QPropertyAnimation::finished, this, [=]{
            loses_prog = 0;
        });
    });
    connect(line_edit, &BottomLineEdit::signalFocusOut, this, [=]{
        connect(startAnimation("LosesProg", getLosesProg(), 100, focus_duration, QEasingCurve::OutQuad), &QPropertyAnimation::finished, this, [=]{
            focus_prog = 0;
            loses_prog = 0;
        });
    });

    grayed_color = Qt::gray;
    accent_color = QColor(198, 47, 47);
}

BottomLineEdit *LabeledEdit::getEdit()
{
    return line_edit;
}

void LabeledEdit::adjustBlank()
{
    // 计算四周的空白
    QFont ft = line_edit->font();
    ft.setPointSize(ft.pointSize() / 2);
    QFontMetrics fm(ft);
    int label_height = fm.lineSpacing();

    auto layout = static_cast<QVBoxLayout*>(this->layout());
    delete layout->takeAt(0);
    layout->insertItem(0, label_spacer = new QSpacerItem(0, label_height));
}

void LabeledEdit::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
}

void LabeledEdit::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    // 绘制标签
    QRect geom = line_edit->geometry();


    int line_left = geom.left(), line_right = geom.right();
    int line_top = geom.bottom(), line_width = geom.width();
    if (!wrong_prog)
    {
        // 绘制普通下划线
        painter.setPen(QPen(grayed_color, pen_width));
        painter.drawLine(line_left, line_top, line_right, line_top);

        // 绘制高亮下划线
        if (focus_prog || loses_prog)
        {
            int w = line_width * focus_prog / 100;
            int l = line_left + line_width * loses_prog / 100;
            painter.setPen(QPen(accent_color, pen_width));
            painter.drawLine(l, line_top, line_left + w, line_top);
        }

        // 绘制勾


    }
    else // 绘制波浪线
    {

    }
}

QPropertyAnimation* LabeledEdit::startAnimation(QByteArray name, QVariant start, QVariant end, int duration, QEasingCurve curve)
{
    QPropertyAnimation* ani = new QPropertyAnimation(this, name);
    ani->setStartValue(start);
    ani->setEndValue(end);
    ani->setDuration(duration);
    ani->setEasingCurve(curve);
    ani->start();
    connect(ani, SIGNAL(finished()), ani, SLOT(deleteLater()));
    return ani;
}

void LabeledEdit::setLabelProg(int x)
{
    this->label_prog = x;
    update();
}

int LabeledEdit::getLabelProg()
{
    return label_prog;
}

void LabeledEdit::setFocusProg(int x)
{
    this->focus_prog = x;
    update();
}

int LabeledEdit::getFocusProg()
{
    return focus_prog;
}

void LabeledEdit::setLosesProg(int x)
{
    this->loses_prog = x;
    update();
}

int LabeledEdit::getLosesProg()
{
    return loses_prog;
}

void LabeledEdit::setWrongProg(int x)
{
    this->wrong_prog = x;
    update();
}

int LabeledEdit::getWrongProg()
{
    return wrong_prog;
}

void LabeledEdit::setCorrectProg(int x)
{
    this->correct_prog = x;
    update();
}

int LabeledEdit::getCorrectProg()
{
    return correct_prog;
}
