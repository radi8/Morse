#include <QApplication>

#include "mainwindow.h"

//bool saveChars(const QString &fname);
bool loadChars(const QString &fname);

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	loadChars("../characters.csv");

	MainWindow *main = new MainWindow();
	main->show();

	int res = app.exec();
	saveChars("../characters.csv");

	delete main;
	return res;
}
