#include "labelededit.h"

LabeledEdit::LabeledEdit(QWidget *parent) : QWidget(parent)
{
    setObjectName("LabeledEdit");
    line_edit = new BottomLineEdit(this);
    up_spacer = new QWidget(this);
    down_spacer = new QWidget(this);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(up_spacer);
    layout->addWidget(line_edit);
    layout->addWidget(down_spacer);
    layout->setSpacing(0);

    up_spacer->setMinimumWidth(1);
    down_spacer->setMinimumWidth(1);

    connect(line_edit, &BottomLineEdit::signalFocusIn, this, [=]{
        connect(startAnimation("FocusProg", getFocusProg(), 100, focus_duration, QEasingCurve::OutQuad), &QPropertyAnimation::finished, this, [=]{
            loses_prog = 0;
        });
        startAnimation("LabelProg", getLabelProg(), 100, label_duration, QEasingCurve::Linear);
    });
    connect(line_edit, &BottomLineEdit::signalFocusOut, this, [=]{
        if (line_edit->hasFocus()) // 比如右键菜单，还是算作聚焦的
            return ;
        connect(startAnimation("LosesProg", getLosesProg(), 100, focus_duration, QEasingCurve::OutQuad), &QPropertyAnimation::finished, this, [=]{
            if (!line_edit->hasFocus())
                focus_prog = 0;
            loses_prog = 0;
        });
        if (line_edit->text().isEmpty())
            startAnimation("LabelProg", getLabelProg(), 0, label_duration, QEasingCurve::Linear);
    });
    connect(line_edit, &BottomLineEdit::textEdited, this, [=]{
        if (correct_prog)
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
    double label_sh = sfm.height();
    double wave_h = nfm.height() * 2 / 3;

    QRect geom = line_edit->geometry();
    double big_margin = (geom.height() - label_nh) / 2;
    double small_margin = big_margin / label_scale;

    up_spacer->setMinimumHeight(static_cast<int>(label_sh * label_scale));
    down_spacer->setMinimumHeight(static_cast<int>(wave_h));
    layout()->setMargin(0);

    // 缓存文字的位置
    label_in_poss.clear();
    label_up_poss.clear();
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

    // 菊花的位置
    loading_inner = label_nh / 4;
    loading_outer = label_nh * 3 / 8;
    loading_rect = QRectF(geom.right() - label_nh, geom.bottom() - label_nh, label_nh, label_nh).toRect();
}

LabeledEdit::LabeledEdit(QString label, QWidget *parent) : LabeledEdit(parent)
{
    setLabelText(label);
}

LabeledEdit::LabeledEdit(QString label, QString def, QWidget *parent) : LabeledEdit(label, parent)
{
    line_edit->setText(def);
}

void LabeledEdit::setLabelText(QString text)
{
    this->label_text = text;
}

void LabeledEdit::setMessageText(QString text)
{
    setMessageText(text, accent_color);
}

void LabeledEdit::setMessageText(QString text, QColor color)
{
    if (!msg_text.isEmpty())
        hideMsg();
    this->msg_text = text;
    this->msg_color = color;
}

void LabeledEdit::setTipText(QString text)
{
    setTipText(text, Qt::gray);
}

void LabeledEdit::setTipText(QString text, QColor color)
{
    this->tip_text = text;
    this->tip_color = color;
}

void LabeledEdit::setAccentColor(QColor color)
{
    this->accent_color = color;
}

void LabeledEdit::showCorrect()
{
    if (show_loading_prog)
        hideLoading();
    // 错误与正确只能选一个
    wrong_prog = 0;
    startAnimation("CorrectProg", getCorrectProg(), 100, correct_duration, QEasingCurve::Linear);
}

void LabeledEdit::hideCorrect()
{
    startAnimation("CorrectProg", getCorrectProg(), 0, correct_duration, QEasingCurve::OutQuad);
}

void LabeledEdit::showWrong()
{
    if (show_loading_prog)
        hideLoading();
    if (tip_prog)
        hideTip();
    // 隐藏现有文字
    line_edit->setViewShowed(false);
    // 开始动画
    correct_prog = 0;
    connect(startAnimation("WrongProg", getWrongProg(), 100, wrong_duration, QEasingCurve::OutQuad), &QPropertyAnimation::finished, this, [=]{
        // 只显示波浪线一次
        wrong_prog = 0;
        line_edit->setViewShowed(true);
        // 恢复隐藏的提示
        if (!msg_text.isEmpty())
            showMsg();
        else if (entering)
            showTip();
    });
    // 随着错误曲线隐藏文字
    if (!msg_text.isEmpty())
    {
        hideMsg();
    }
}

void LabeledEdit::showWrong(QString msg)
{
    setMessageText(msg);
    showWrong();
}

void LabeledEdit::showLoading()
{
    if (correct_prog)
        hideCorrect();
    if (!msg_text.isEmpty())
        hideMsg();

    if (loading_timer == nullptr)
    {
        loading_timer = new QTimer(this);
        loading_timer->setInterval(80);
        connect(loading_timer, &QTimer::timeout, this, [=]{
            loading_index++;
            update();
//          repaint(loading_rect);
        });
    }
    loading_timer->start();
    startAnimation("ShowLoadingProg", getShowLoadingProg(), 100, show_loading_duration, QEasingCurve::QEasingCurve::OutBack);
}

void LabeledEdit::hideLoading()
{
    connect(startAnimation("HideLoadingProg", getHideLoadingProg(), 100, hide_loading_duration, QEasingCurve::OutQuad), &QPropertyAnimation::finished, this, [=]{
        show_loading_prog = 0;
        hide_loading_prog = 0;
        if (loading_timer)
            loading_timer->stop();
    });
}

void LabeledEdit::showTip()
{
    startAnimation("TipProg", getTipProg(), 100, tip_duration, QEasingCurve::InQuad);
}

void LabeledEdit::hideTip()
{
    startAnimation("TipProg", getTipProg(), 0, tip_duration, QEasingCurve::InQuad);
}

void LabeledEdit::showMsg()
{
    startAnimation("MsgShowProg", getMsgShowProg(), 100, msg_show_duration, QEasingCurve::OutQuad);
}

/**
 * 隐藏现有的msg，新的msg可能会是不一样的数值
 * 所以需要备份当前的msg
 */
void LabeledEdit::hideMsg()
{
    msg_hiding = msg_text;
    msg_text = "";
    msg_show_prog = 0;
    connect(startAnimation("MsgHideProg", getMsgHideProg(), 100, msg_hide_duration, QEasingCurve::OutQuad), &QPropertyAnimation::finished, this, [=]{
        msg_hide_prog = 0;
        msg_hiding = "";
    });
}

void LabeledEdit::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    adjustBlank();
}

void LabeledEdit::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
//    painter.drawRect(0,0,width()-1,height()-1); // 测试边距

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
            const int step2 = 75;
            const int step3 = 85;
            const int step4 = 100;

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
                if (correct_prog <= step2)
                {
                    // 画两截的线
                    int move_dis = (short_len + move_left + blank_len) * (correct_prog - step1) / (step2 - step1); // 相对于上一阶段的最左边
                    int blank_left = line_right - (short_len + move_left + blank_len) + move_dis;
                    paint2Line(blank_left, move_left);
                }
                else
                {
                    // 画普通的线
                    paintLine();
                }

                painter.setPen(QPen(accent_color, pen_width));
                painter.setRenderHint(QPainter::Antialiasing, true);
                double radius = short_len / 2.0;
                const int offset = 2; // 线宽的偏移
                double angle_turned = 0;
                if (correct_prog >= step1 && correct_prog <= step2) // 画弧线
                {
                    // 出现的弧线
                    QRect rect(line_right - short_len, line_top-short_len, short_len, short_len);
                    painter.setPen(QPen(accent_color, pen_width));
                    double angle_span = 360/PI;
                    double angle = -90 + (150+angle_span) * (correct_prog - step1) / (step2 - step1);
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

                if (correct_prog >= step2)
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

                if (correct_prog >= step3) // 画勾的左半部分
                {
                    QPointF pos2(line_right - radius*3/2 + offset, line_top - radius + radius/2 * sin(PI/3)); // 右下角
                    QPointF pos1(line_right - short_len + offset, line_top - radius);
                    double cent = (correct_prog - step3) / static_cast<double>(step4 - step3);
                    if (cent > 1)
                        cent = 1;
                    double x = pos2.x() + (pos1.x() - pos2.x()) * cent;
                    double y = pos2.y() + (pos1.y() - pos2.y()) * cent;
                    QPointF pos(x, y);
                    painter.drawLine(pos2, pos);
                }
            }
        }

        // 绘制文字
        if (!label_text.isEmpty())
        {
            QFont nft = line_edit->font();
            painter.setPen(QPen(grayed_color, 1));
//            painter.setRenderHint(QPainter::TextAntialiasing, true);

            if (label_prog <= 0) // 在输入框里面
            {
                painter.setFont(nft);
                for (int i = 0; i < label_text.size(); i++)
                {
                    painter.drawText(label_in_poss.at(i), label_text.at(i));
                }
            }
            else if (label_prog >= 100) // 在输入框上面
            {
                QFont sft = nft;
                sft.setPointSizeF(nft.pointSize() / label_scale);
                painter.setFont(sft);

                for (int i = 0; i < label_text.size(); i++)
                {
                    painter.drawText(label_up_poss.at(i), label_text.at(i));
                }

            }
            else if (focus_prog && !loses_prog)
            {
                // 左边先抬起来，左边进度最大
                QFont aft = line_edit->font();
                const double in_size = nft.pointSizeF();
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
                const double in_size = nft.pointSizeF();
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
    else // 错误曲线
    {
        QFont nft = line_edit->font();
        QFontMetrics nfm(nft);
        double n_offset = nfm.horizontalAdvance(line_edit->displayText()) / 2;
        QFont sft = nft;
        QFontMetrics sfm(sft);
        double s_offset = sfm.horizontalAdvance(label_text) * 2 / 3;

        // 绘制波浪线
        const double ampl = nfm.height()*2/3; // 振幅
        const double ctrlen = line_width / 4; // 控制点对应方向延伸的距离
        const double total_len = line_width * 4 + pen_width*2 + qMax(n_offset, s_offset);
        QPainterPath path;
        painter.save();
        painter.setPen(QPen(accent_color, pen_width));
        painter.setRenderHint(QPainter::Antialiasing, true);
        double paint_left = -wrong_prog * (total_len-line_width-pen_width*2) / 100;
        path.moveTo(paint_left, line_top);
        path.lineTo(paint_left + line_width, line_top);
        path.cubicTo(QPointF(paint_left+line_width+ctrlen, line_top),
                     QPointF(paint_left+line_width*3/2-ctrlen, line_top+ampl),
                     QPointF(paint_left+line_width*3/2, line_top+ampl));
        path.cubicTo(QPointF(paint_left+line_width*3/2+ctrlen, line_top+ampl),
                     QPointF(paint_left+line_width*5/2-ctrlen, line_top-ampl),
                     QPointF(paint_left+line_width*5/2, line_top-ampl));
        path.cubicTo(QPointF(paint_left+line_width*5/2+ctrlen, line_top-ampl),
                     QPointF(paint_left+line_width*3-ctrlen, line_top),
                     QPointF(paint_left+line_width*3, line_top));
        path.lineTo(paint_left + total_len, line_top);

        painter.setClipRect(QRectF(line_left-pen_width/2, line_top - ampl - pen_width, line_width+pen_width, line_top + ampl + pen_width));
        painter.drawPath(path);
        painter.restore();

        // 绘制文字
        painter.setPen(QPen(grayed_color, 1));
        if (line_edit->text().isEmpty() && !line_edit->hasFocus()) // 文字在编辑框中
        {
            painter.setFont(nft);
            for (int i = 0; i < label_text.size(); i++)
            {
                double x = label_in_poss.at(i).x();
                double perc = (x - paint_left) / total_len;
                double y = path.pointAtPercent(perc).y();
                painter.drawText(QPointF(x, y + label_in_poss.at(i).y() - line_top), label_text.at(i));
            }
        }
        else // 文字在编辑框上面
        {
            sft.setPointSizeF(nft.pointSize() / label_scale);
            painter.setFont(sft);

            for (int i = 0; i < label_text.size(); i++)
            {
                double x = label_up_poss.at(i).x();
                double perc = (x - paint_left - s_offset) / total_len;
                if (perc < 0)
                    perc = 0;
                double y = path.pointAtPercent(perc).y();
                painter.drawText(QPointF(x, label_up_poss.at(i).y() + (y-line_top)/label_scale), label_text.at(i));
            }

            // 输入的曲线
            QString display_text = line_edit->displayText();
            if (!display_text.isEmpty())
            {
                painter.setFont(nft);
                painter.setPen(line_edit->palette().color(QPalette::Text));
                QMargins margins = line_edit->textMargins();
                QPointF pos = line_edit->geometry().bottomLeft();
                pos = QPointF(pos.x() + 2 + margins.left(), pos.y() - margins.bottom() + nfm.height() - nfm.lineSpacing());
                for (int i = 0; i < display_text.size(); i++)
                {
                    double x = pos.x() + nfm.horizontalAdvance(display_text.left(i));
                    double perc = (x - n_offset - paint_left) / total_len;
                    if (perc < 0)
                        perc = 0;
                    double y = path.pointAtPercent(perc).y();
                    painter.drawText(QPointF(x, y + pos.y() - line_top), display_text.at(i));
                }
            }

        }
    }

    // 绘制逐渐消失的msg
    if (msg_hide_prog && msg_hide_prog < 100 && !msg_hiding.isEmpty())
    {
        QFont sft = line_edit->font();
        double size = sft.pointSize() / label_scale - pen_width/2;
        sft.setPointSizeF(size);
        QFontMetricsF nfm(sft); // 这是原本的大小
        painter.setPen(accent_color);

        // 方案一：分别绘制每一个文字的大小，从右往左逐个变小（不明显）（时间建议500ms）
        /*const double start_prog = 40.0;
        const double per_prog = start_prog / msg_hiding.size();
        const double total_prog = 100 - start_prog;
        const double ty = line_top + nfm.height() / 2;
        for (int i = 0; i < msg_hiding.size(); i++)
        {
            double my_start_prog = per_prog * i; // 达到这个进度才开始变化
            double prop = (total_prog - (msg_hide_prog - my_start_prog)) / total_prog;
            if (prop < 0)
                prop = 0;
            else if (prop > 1)
                prop = 1;

            size = size * prop;
            if (size >= 0.5)
            {
                sft.setPointSizeF(size);
                painter.setFont(sft);
                double y = ty + QFontMetricsF(sft).height()/2;

                double w = nfm.horizontalAdvance(msg_hiding.left(i));
                QPointF pos(label_up_poss.first().x() + w, y);
                painter.drawText(pos, msg_hiding.at(i));
            }
            else if (size >= 0.1)
            {
                double w = nfm.horizontalAdvance(msg_hiding.left(i));
                QPointF pos(label_up_poss.first().x() + w, ty);
                painter.drawPoint(pos);
            }

        }*/

        // 方案二：全部一起消失（时间建议300ms）
        size = size * (100-msg_hide_prog)/100;
        painter.setPen(accent_color);
        double y = line_top + nfm.height() / 2;
        if (size >= 0.5)
        {
            sft.setPointSizeF(size);
            QFontMetricsF sfm(sft);
            painter.setFont(sft);
            y += QFontMetricsF(sft).height()/2;

            // 从原本的大小变成一个个小点
            for (int i = 0; i < msg_hiding.size(); i++)
            {
                QString le = msg_hiding.left(i);
                double w = nfm.horizontalAdvance(le);
                QString ch = msg_hiding.at(i);
                double x = label_up_poss.first().x() + w
                        + (nfm.horizontalAdvance(ch) - sfm.horizontalAdvance(ch) ) / 2 - 1;
                QPointF pos(x, y);
                painter.drawText(pos, msg_hiding.at(i));
            }
        }
        else if (size >= 0.1)
        {
            // 文字太小了看不见，只能画点……
            for (int i = 0; i < msg_hiding.size(); i++)
            {
                double w = nfm.horizontalAdvance(msg_hiding.left(i));
                double cw = nfm.horizontalAdvance(msg_hiding.at(i));
                double x = label_up_poss.first().x() + w + cw / 2-1;
                QPointF pos(x, y);
                painter.drawPoint(pos);
            }
        }
    }

    // 逐渐显示的msg
    if (msg_show_prog && !msg_text.isEmpty())
    {
        QFont sft = line_edit->font();
        sft.setPointSizeF(sft.pointSize() / label_scale - pen_width/2);
        QFontMetricsF sfm(sft);
        painter.setFont(sft);
        painter.setPen(accent_color);

        if (msg_show_prog != 100) // 绘制从右边过来的文字
        {
            const double tx = label_up_poss.first().x();
            const double ty = line_top + QFontMetricsF(sft).height();

            // double dis = line_width / msg_text.size();
            double start_prog = 60.0;
            double per_prog = 40.0 / msg_text.size();
            for (int i = 0; i < msg_text.size(); i++)
            {
                double my_prog = start_prog + per_prog * i;
                double prop = msg_show_prog / my_prog;
                if (prop > 1)
                    prop = 1;
                // double right = line_right + dis * i; // 非线性延迟出现
                double len = sfm.horizontalAdvance(msg_text.left(i));
                double right = line_right + len;
                double left = tx + len;
                double x = left + (right - left) * (1 - prop);
                painter.drawText(QPointF(x, ty), msg_text.at(i));
            }
        }
        else
        {
            QPointF pos(label_up_poss.first().x(), line_top + QFontMetricsF(sft).height());
            painter.drawText(pos, msg_text);
        }
    }

    // 绘制提示文字
    else if (tip_prog && !tip_text.isEmpty())
    {
        QFont sft = line_edit->font();
        sft.setPointSizeF(sft.pointSize() / label_scale - pen_width/2);
        painter.setFont(sft);

        QColor c = tip_color;
        c.setAlpha(c.alpha() * tip_prog / 100);
        painter.setPen(c);

        QPointF pos(label_up_poss.first().x(), line_top + QFontMetricsF(sft).height());
        painter.drawText(pos, tip_text);
    }

    // 绘制加载中动画
    if (show_loading_prog || hide_loading_prog)
    {
        painter.setRenderHint(QPainter::Antialiasing, true);
        // 半径与 show_loading_prog 有关，线条长度与 hide_loading_prog 有关
        double inner = loading_inner * show_loading_prog / 100;
        double outer = loading_outer * show_loading_prog / 100;
        inner += (outer - inner) * hide_loading_prog / 100;
        QPoint center = loading_rect.center();
        const int per_angle = 360 / 8; // 每一片菊花花瓣之间的夹角
        QColor c = accent_color;

        int current_index = loading_index % loading_petal; // 当前的index（颜色最浓）
        int angle = current_index * per_angle; // 要计算的旋转角
        int alpha = 0xff; // 要绘制的不透明度
        for (int i = 0; i < loading_petal; i++)
        {
            // 绘制每一片菊花
            double radian = angle * PI / 180;
            QPointF inn(center.x() + inner * sin(radian),
                        center.y() + inner * cos(radian));
            QPointF out(center.x() + outer * sin(radian),
                        center.y() + outer * cos(radian));

            alpha -= 0xaa / loading_petal; // 每次减少一点透明度（不能减到0，否则看起来怪怪的）
            c.setAlpha(alpha);
            painter.setPen(QPen(c, pen_width, Qt::SolidLine, Qt::RoundCap));

            painter.drawLine(inn, out);
            angle += per_angle; // 颜色逐渐变淡
        }
    }
}

void LabeledEdit::enterEvent(QEvent *event)
{
    QWidget::enterEvent(event);
    entering = true;
    if (!tip_text.isEmpty())
    {
        showTip();
    }
}

void LabeledEdit::leaveEvent(QEvent *event)
{
    QWidget::leaveEvent(event);
    entering = false;
    if (!tip_text.isEmpty())
    {
        hideTip();
    }
}

QPropertyAnimation* LabeledEdit::startAnimation(QByteArray name, double start, double end, int duration, QEasingCurve curve)
{
    QPropertyAnimation* ani = new QPropertyAnimation(this, name);
    ani->setStartValue(start);
    ani->setEndValue(end);
    ani->setDuration(static_cast<int>(duration * qAbs(start - end) / 100));
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

void LabeledEdit::setShowLoadingProg(int x)
{
    this->show_loading_prog = x;
    update();
}

int LabeledEdit::getShowLoadingProg()
{
    return show_loading_prog;
}

void LabeledEdit::setHideLoadingProg(int x)
{
    this->hide_loading_prog = x;
    update();
}

int LabeledEdit::getHideLoadingProg()
{
    return hide_loading_prog;
}

void LabeledEdit::setTipProg(int x)
{
    this->tip_prog = x;
    update();
}

int LabeledEdit::getTipProg()
{
    return tip_prog;
}

void LabeledEdit::setMsgShowProg(int x)
{
    this->msg_show_prog = x;
    update();
}

int LabeledEdit::getMsgShowProg()
{
    return msg_show_prog;
}

void LabeledEdit::setMsgHideProg(int x)
{
    this->msg_hide_prog = x;
    update();
}

int LabeledEdit::getMsgHideProg()
{
    return msg_hide_prog;
}
