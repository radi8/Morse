#include <QApplication>

#include "mainwindow.h"
#include "characters.h"


int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	loadChars("../characters.csv");

	MainWindow *main = new MainWindow();
	main->show();
	int res = app.exec();

	delete main;

	return res;
}
