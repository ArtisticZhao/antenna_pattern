#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
	// ���Windows high DPI ���Ŵ�����������ʾ��ȫ������
	QApplication::setAttribute(Qt::AA_DisableHighDpiScaling);
	if (qgetenv("QT_FONT_DPI").isEmpty()) {
		qputenv("QT_FONT_DPI", "90");
	}

    QApplication a(argc, argv);

    MainWindow w;
    w.show();
    return a.exec();
}
