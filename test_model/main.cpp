#include <QApplication>

#include "mainwindow.h"

//bool saveChars(const QString &fname);
bool loadChars(const QString &fname);

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	loadChars("characters.txt");

	MainWindow *main = new MainWindow();
	main->show();

	int res = app.exec();
	saveChars("characters.txt");

	delete main;
	return res;
}
