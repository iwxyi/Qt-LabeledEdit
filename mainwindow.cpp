#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->widget->setLabelText("usernamenick");
    ui->widget_2->setLabelText("password");
}

MainWindow::~MainWindow()
{
    delete ui;
}

