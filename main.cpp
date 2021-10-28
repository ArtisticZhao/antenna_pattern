#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
	// 解决Windows high DPI 缩放带来的字体显示不全的问题
	QApplication::setAttribute(Qt::AA_DisableHighDpiScaling);
	if (qgetenv("QT_FONT_DPI").isEmpty()) {
		qputenv("QT_FONT_DPI", "90");
	}

    QApplication a(argc, argv);

    MainWindow w;
    w.show();
    return a.exec();
}
