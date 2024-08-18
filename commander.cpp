#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QStandardPaths>
#include <QDebug>
#include <systemd/sd-daemon.h>

static void comment(bool addComment) {
    QFile file("/etc/apt/sources.list");

    // 检查文件是否能被打开
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "无法打开文件" << file.fileName();
        return;
    }

    QStringList lines;
    QTextStream in(&file);

    // 逐行读取文件内容
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.contains("https://community-packages.deepin.com/beige")) {
            if (addComment) {
                // 如果需要添加注释，且行首没有 '#'
                if (!line.startsWith("#")) {
                    line.prepend("#");
                }
            } else {
                // 如果需要移除注释，且行首有 '#'
                if (line.startsWith("#")) {
                    line.remove(0, 1);
                }
            }
        }
        lines.append(line);
    }

    file.close();

    // 重新写入修改后的内容
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        qWarning() << "无法重新打开文件写入内容" << file.fileName();
        return;
    }

    QTextStream out(&file);
    for (const QString &line : lines) {
        out << line << "\n";
    }

    file.close();
}

void writeToFile(const QString &text) {
    QFile file("/etc/apt/sources.list.d/mirror-selector.tmp.list");

    // 打开文件以写入模式（WriteOnly），清空文件内容（Truncate）
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        qWarning() << "无法打开文件" << file.fileName() << "进行写入";
        return;
    }

    QTextStream out(&file);

    // 写入新的字符串内容
    out << text;

    file.close();  // 关闭文件
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QCommandLineOption arg({ "a", "arg" }, "arg", "arg");

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addOptions({ arg });

    parser.process(app);

    if (!parser.isSet(arg)) {
        return -1;
    }

    QString mirror = parser.value(arg);
    if (mirror == "reset") {
        comment(false);
        QFile file("/etc/apt/sources.list.d/mirror-selector.tmp.list");
        file.remove();
    }
    else {
        QFile file(":/mirror.list");

        // 检查文件是否能被打开
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "无法打开文件" << file.fileName();
            return -1;
        }

        QTextStream in(&file);

        int count = 0;
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();  // 去除首尾空白字符
            if (!line.isEmpty()) {
                if (count++ == mirror.toInt()) {
                    writeToFile(QString("deb %1 beige main commercial community").arg(line));
                    comment(true);
                    break;
                }
            }
        }

        file.close();
    }

    sd_notify(0, "READY=1");

    return 0;
}
