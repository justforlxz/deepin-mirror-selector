#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFile>
#include <QStandardItemModel>
#include <QDBusInterface>
#include <QDebug>
#include <QUrl>
#include <QLocale>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle(QLocale::system().language() == QLocale::Chinese ? "镜像源选择器" : "Mirror Selector");
    ui->label->setText(QLocale::system().language() == QLocale::Chinese ? "选择镜像源" : "Select");
    ui->pushButton->setText(QLocale::system().language() == QLocale::Chinese ? "应用" : "Apply");
    ui->pushButton_2->setText(QLocale::system().language() == QLocale::Chinese ? "重设" : "Reset");

    QFile file(":/mirror.list");

    // 检查文件是否能被打开
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "无法打开文件" << file.fileName();
        return;
    }

    QStandardItemModel *model = new QStandardItemModel(ui->comboBox);
    QTextStream in(&file);

    // 逐行读取文件内容并添加到 model 中
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();  // 去除首尾空白字符
        if (!line.isEmpty()) {
            QUrl url(line);
            QStandardItem *item = new QStandardItem(url.host());
            model->appendRow(item);
        }
    }

    file.close();

    ui->comboBox->setModel(model);

    connect(ui->pushButton, &QPushButton::clicked, this, [=] {
        QDBusInterface systemd("org.freedesktop.systemd1",
                               "/org/freedesktop/systemd1",
                               "org.freedesktop.systemd1.Manager",
                               QDBusConnection::systemBus());
        systemd.call("StartUnit",
              QString("mirror-selector@%1.service").arg(ui->comboBox->currentIndex()), "replace");
    });
    connect(ui->pushButton_2, &QPushButton::clicked, this, [=] {
        QDBusInterface systemd("org.freedesktop.systemd1",
                               "/org/freedesktop/systemd1",
                               "org.freedesktop.systemd1.Manager",
                               QDBusConnection::systemBus());
        systemd.call("StartUnit", QString("mirror-selector@reset.service"), "replace");
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}
