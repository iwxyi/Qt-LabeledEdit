#include "labelededit.h"

LabeledEdit::LabeledEdit(QWidget *parent) : QWidget(parent)
{
    line_edit = new BottomLineEdit(this);
    label_spacer = new QSpacerItem(0, 0);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addSpacerItem(label_spacer);
    layout->addWidget(line_edit);

    connect(line_edit, &BottomLineEdit::signalFocusIn, this, [=]{
        connect(startAnimation("FocusProg", getFocusProg(), 100, focus_duration, QEasingCurve::OutQuad), &QPropertyAnimation::finished, this, [=]{
            loses_prog = 0;
        });
        startAnimation("LabelProg", getLabelProg(), 100, label_duration, QEasingCurve::Linear);
    });
    connect(line_edit, &BottomLineEdit::signalFocusOut, this, [=]{
        connect(startAnimation("LosesProg", getLosesProg(), 100, focus_duration, QEasingCurve::OutQuad), &QPropertyAnimation::finished, this, [=]{
            focus_prog = 0;
            loses_prog = 0;
        });
        if (line_edit->text().isEmpty())
            startAnimation("LabelProg", getLabelProg(), 0, label_duration, QEasingCurve::Linear);
    });
    connect(line_edit, &BottomLineEdit::textEdited, this, [=]{
        if (correct_prog == 100)
        {
            startAnimation("CorrectProg", getCorrectProg(), 0, correct_duration, QEasingCurve::Linear);
        }
    });

    grayed_color = Qt::gray;
    accent_color = QColor(198, 47, 47);

    QFont ft = line_edit->font();
    ft.setPointSizeF(ft.pointSize() * 1.5);
    line_edit->setFont(ft);
    QTimer::singleShot(0, [=]{
        adjustBlank();
    });

    this->setFocusProxy(line_edit);
}

void LabeledEdit::setLabelText(QString text)
{
    this->label_text = text;
}

void LabeledEdit::setAccentColor(QColor color)
{
    this->accent_color = color;
}

void LabeledEdit::showCorrect()
{
    wrong_prog = 0;
    startAnimation("CorrectProg", getCorrectProg(), 100, correct_duration, QEasingCurve::Linear);
}

void LabeledEdit::showWrong()
{
    correct_prog = 0;
    connect(startAnimation("WrongProg", getWrongProg(), 100, wrong_duration, QEasingCurve::Linear), &QPropertyAnimation::finished, this, [=]{
        // 只显示波浪线一次
        wrong_prog = 0;
    });
}

BottomLineEdit *LabeledEdit::editor()
{
    return line_edit;
}

/**
 * 修改控件大小或者字体大小后，调整各种间距与位置
 */
void LabeledEdit::adjustBlank()
{
    // 计算四周的空白
    QFont nft = line_edit->font();
    QFontMetricsF nfm(nft);
    double label_nh = nfm.height();
    QFont sft = nft;
    sft.setPointSizeF(sft.pointSize() / label_scale);
    QFontMetricsF sfm(sft);
    double label_sh = sfm.lineSpacing();

    label_spacer->changeSize(0, static_cast<int>(label_sh));

    // 缓存文字的位置
    label_in_poss.clear();
    label_up_poss.clear();
    QRect geom = line_edit->geometry();
    double big_margin = (geom.height() - label_nh) / 2;
    double small_margin = big_margin / label_scale;
    QPointF in_pos(geom.left() + big_margin, geom.bottom() - big_margin);
    QPointF up_pos(geom.left() + small_margin, geom.top() - small_margin);
    label_in_poss.append(in_pos);
    label_up_poss.append(up_pos);
    for (int i = 1; i < label_text.size(); i++)
    {
        QString t = label_text.left(i);
        double in_w = nfm.horizontalAdvance(t);
        double up_w = sfm.horizontalAdvance(t);
        label_in_poss.append(QPointF(in_pos + QPointF(in_w, 0)));
        label_up_poss.append(QPointF(up_pos + QPointF(up_w, 0)));
    }
}

void LabeledEdit::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    adjustBlank();
}

void LabeledEdit::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.drawRect(0,0,width()-1,height()-1);

    // 绘制标签
    const double PI = 3.141592;
    QRect geom = line_edit->geometry();


    const int line_left = geom.left(), line_right = geom.right();
    const int line_top = geom.bottom(), line_width = geom.width();
    if (!wrong_prog)
    {
        auto paintLine = [&]{
            // 绘制普通下划线
            painter.setPen(QPen(grayed_color, 1, Qt::SolidLine, Qt::RoundCap));
            painter.drawLine(line_left, line_top, line_right, line_top);

            // 绘制高亮下划线
            if (focus_prog || loses_prog)
            {
                int w = line_width * focus_prog / 100;
                int l = line_left + line_width * loses_prog / 100;
                painter.setPen(QPen(accent_color, pen_width));
                painter.drawLine(l, line_top, line_left + w, line_top);
            }
        };

        // 绘制勾
        if (!correct_prog) // 普通下划线
        {
            paintLine();
        }
        else // correct_prog // 显示箭头
        {
            QFont ft = line_edit->font();
            QFontMetrics fm(ft);
            const int short_len = fm.height(); // 线的长度
            const int blank_len = fm.horizontalAdvance(" ") / 2; // 空白点的宽度
            const int move_left = short_len / 2;
            const int step1 = 40;
            // const int step2 = 60;
            const int step3 = 75;
            const int step4 = 85;
            const int step5 = 100;

            // 绘制两截的线
            auto paint2Line = [&](int blank_left, int right_margin) {
                // 绘制普通下划线
                painter.setPen(QPen(grayed_color, 1, Qt::SolidLine, Qt::RoundCap));
                painter.drawLine(line_left, line_top, blank_left, line_top);
                if (blank_left+blank_len < line_right)
                    painter.drawLine(blank_left + blank_len, line_top, line_right - right_margin, line_top);

                // 绘制高亮下划线
                if (focus_prog || loses_prog)
                {
                    painter.setPen(QPen(accent_color, pen_width));
                    int w = line_width * focus_prog / 100;
                    int l = line_left + line_width * loses_prog / 100;
                    int r = line_left + w;
                    if (r <= blank_left)
                    {
                        painter.drawLine(l, line_top, r, line_top);
                    }
                    else // 分为两截画
                    {
                        painter.drawLine(l, line_top, blank_left, line_top);
                        if (blank_left + blank_len < line_right)
                            painter.drawLine(blank_left + blank_len, line_top, qMin(r, line_right - right_margin), line_top);
                    }
                }
            };

            if (correct_prog <= step1) // 分割
            {
                int move_dis = move_left * correct_prog / step1; // 左移的位置
                int blank_left = line_right - blank_len - short_len - move_dis; // 隔断点的左边
                paint2Line(blank_left, move_dis);
            }
            else // 延伸、旋转
            {
                if (correct_prog <= step3)
                {
                    // 画两截的线
                    int move_dis = (short_len + move_left + blank_len) * (correct_prog - step1) / (step3 - step1); // 相对于上一阶段的最左边
                    int blank_left = line_right - (short_len + move_left + blank_len) + move_dis;
                    paint2Line(blank_left, move_left);
                }
                else
                {
                    // 画普通的线
                    paintLine();
                }

                painter.setRenderHint(QPainter::Antialiasing, true);
                double radius = short_len / 2.0;
                const int offset = 2; // 线宽的偏移
                double angle_turned = 0;
                if (correct_prog >= step1 && correct_prog <= step3) // 画弧线
                {
                    // 出现的弧线
                    QRect rect(line_right - short_len, line_top-short_len, short_len, short_len);
                    painter.setPen(QPen(accent_color, pen_width));
                    double angle_span = 360/PI;
                    double angle = -90 + (150+angle_span) * (correct_prog - step1) / (step3 - step1);
                    if (angle - angle_span < -90) // 一开始的
                        angle_span = angle+90;
                    if (angle > 60) // 准备转弯
                    {
                        angle_turned = angle - 60;
                        angle_span -= (angle - 60);
                        angle = 60;
                    }
                    painter.drawArc(rect, static_cast<int>(angle * 16), static_cast<int>(-angle_span * 16));
                }

                if (correct_prog >= step3)
                    angle_turned = 360/PI;
                if (angle_turned > 0)
                {
                    QPointF pos2(line_right - radius/2, line_top - radius - radius/2 * sin(PI/3)-3);   // 右上角
                    QPointF pos1(line_right - radius*3/2 + offset, line_top - radius + radius/2 * sin(PI/3)); // 左下角
                    double turned_len = PI * short_len * angle_turned / 360;
                    double cent = turned_len / short_len;
                    if (cent > 1)
                        cent = 1;
                    double x = pos2.x() + (pos1.x() - pos2.x()) * cent;
                    double y = pos2.y() + (pos1.y() - pos2.y()) * cent;
                    QPointF pos(x, y);
                    painter.drawLine(pos2, pos);
                }

                // 为了保持线条长度一致，已经放入了step3中
                /*if (correct_prog >= step2) // 画勾的右半部分
                {
                    QPointF pos2(line_right - radius/2, line_top - radius - radius/2 * sin(PI/3)-3);   // 右上角
                    QPointF pos1(line_right - radius*3/2 + offset, line_top - radius + radius/2 * sin(PI/3)); // 左下角
                    double cent = (correct_prog - step2) / static_cast<double>(step4 - step2);
                    if (cent > 1)
                        cent = 1;
                    double x = pos2.x() + (pos1.x() - pos2.x()) * cent;
                    double y = pos2.y() + (pos1.y() - pos2.y()) * cent;
                    QPointF pos(x, y);
                    painter.drawLine(pos2, pos);
                }*/

                if (correct_prog >= step4) // 画勾的左半部分
                {
                    QPointF pos2(line_right - radius*3/2 + offset, line_top - radius + radius/2 * sin(PI/3)); // 右下角
                    QPointF pos1(line_right - short_len + offset, line_top - radius);
                    double cent = (correct_prog - step4) / static_cast<double>(step5 - step4);
                    if (cent > 1)
                        cent = 1;
                    double x = pos2.x() + (pos1.x() - pos2.x()) * cent;
                    double y = pos2.y() + (pos1.y() - pos2.y()) * cent;
                    QPointF pos(x, y);
                    painter.drawLine(pos2, pos);
                }
//                painter.drawRect(line_right - short_len, line_top - short_len, short_len, short_len);
            }
        }

        // 绘制文字
        if (!label_text.isEmpty())
        {
            QFont nfm = line_edit->font();
            painter.setPen(QPen(grayed_color, 1));
//            painter.setRenderHint(QPainter::TextAntialiasing, true);

            if (label_prog <= 0) // 在输入框里面
            {
                painter.setFont(nfm);
                for (int i = 0; i < label_text.size(); i++)
                {
                    painter.drawText(label_in_poss.at(i), label_text.at(i));
                }
            }
            else if (label_prog >= 100) // 在输入框上面
            {
                QFont sfm = nfm;
                sfm.setPointSizeF(nfm.pointSize() / label_scale);
                painter.setFont(sfm);

                for (int i = 0; i < label_text.size(); i++)
                {
                    painter.drawText(label_up_poss.at(i), label_text.at(i));
                }

            }
            else if (focus_prog && !loses_prog)
            {
                // 左边先抬起来，左边进度最大
                QFont aft = line_edit->font();
                const double in_size = nfm.pointSizeF();
                const double up_size = in_size / label_scale;
                const int count = label_text.size();
                const double step = 100.0 / count / 2.5; // 每个文字动画比前面文字慢一点，有种曲线感
                const double persist_prog = 100 - step * (count-1); // 每个字符动画的真正时长
                for (int i = 0; i < count; i++)
                {
                    double char_min_prog = step * i;
                    double prog = label_prog - char_min_prog; // 相对于这个字符串的本身周期的prog
                    if (prog < 0)
                        prog = 0;
                    else if (prog > persist_prog)
                        prog = persist_prog;
                    const double max_angle = PI * (0.5 + 1.0/6 * (count-i/2) / count); // 2/3π~4/3π角度为超过上限
                    const double out_prob = 1.0 / sin(max_angle) - 1;
                    const double angle = max_angle * prog / persist_prog;
                    const double cent = sin(angle) * (1 + out_prob); // sin(a)是100的百分比，这里超出20%左右
                    const double size = in_size - (in_size - up_size) * cent;
                    aft.setPointSizeF(size);
                    QPointF in_pos(label_in_poss.at(i)), up_pos(label_up_poss.at(i));
                    const double x = in_pos.x() - (in_pos.x() - up_pos.x()) * cent;
                    const double y = in_pos.y() - (in_pos.y() - up_pos.y()) * cent;
                    QPointF pos(x, y);
                    painter.setFont(aft);
                    painter.drawText(pos, label_text.at(i));
                    prog -= step;
                }
            }
            else // loses_prog
            {
                // 左边先下来
                QFont aft = line_edit->font();
                const double in_size = nfm.pointSizeF();
                const double up_size = in_size / label_scale;
                const int count = label_text.size();
                const double step = 100.0 / count / 4; // 每个文字动画比前面文字慢一点，有种曲线感
                const double max_angle = PI / 2; // 2/3π~4/3π角度为超过上限
                const double persist_prog = 100 - step * (count-1); // 每个字符动画的真正时长
                for (int i = 0; i < count; i++)
                {
                    double char_min_prog = step * (count - i - 1);
                    double prog = label_prog - char_min_prog; // 相对于这个字符串的本身周期的prog
                    if (prog < 0)
                        prog = 0;
                    else if (prog > persist_prog)
                        prog = persist_prog;
                    double angle = max_angle * prog / persist_prog;
                    double cent = sin(angle); // sin(a)是100的百分比
                    double size = in_size - (in_size - up_size) * cent;
                    aft.setPointSizeF(size);
                    QPointF in_pos(label_in_poss.at(i)), up_pos(label_up_poss.at(i));
                    double x = in_pos.x() - (in_pos.x() - up_pos.x()) * cent;
                    double y = in_pos.y() - (in_pos.y() - up_pos.y()) * cent;
                    QPointF pos(x, y);
                    painter.setFont(aft);
                    painter.drawText(pos, label_text.at(i));
                    prog -= step;
                }
            }
        }
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

void LabeledEdit::setLabelProg(double x)
{
    this->label_prog = x;
    update();
}

double LabeledEdit::getLabelProg()
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
