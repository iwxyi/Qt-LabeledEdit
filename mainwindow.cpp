#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->widget->setText("aaaaaa");
    ui->widget->setLabelText("用户名");
    ui->widget->setTipText("允许大小写字母、数字、下划线");
    ui->widget_2->setLabelText("密码");
    ui->widget_2->setTipText("任意文字");
    ui->widget_2->editor()->setEchoMode(QLineEdit::EchoMode::Password);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_2_clicked()
{
    if (ui->widget->editor()->hasFocus())
    {
        ui->widget->showCorrect();
        ui->widget->setMsgText("");
    }
    else if (ui->widget_2->editor()->hasFocus())
    {
        ui->widget_2->showCorrect();
        ui->widget_2->setMsgText("");
    }
}

void MainWindow::on_pushButton_clicked()
{
    if (ui->widget->editor()->hasFocus())
    {
        ui->widget->showWrong("用户名已存在");
    }
    else if (ui->widget_2->editor()->hasFocus())
    {
        ui->widget_2->showWrong("密码错误，您还剩余2次机会");
    }
}

void MainWindow::on_pushButton_3_clicked()
{
    if (ui->widget->editor()->hasFocus())
    {
        ui->widget->showLoading();
    }
    else if (ui->widget_2->editor()->hasFocus())
    {
        ui->widget_2->showLoading();
    }
}

void MainWindow::on_pushButton_4_clicked()
{
    if (ui->widget->editor()->hasFocus())
    {
        ui->widget->hideLoading();
    }
    else if (ui->widget_2->editor()->hasFocus())
    {
        ui->widget_2->hideLoading();
    }
}
