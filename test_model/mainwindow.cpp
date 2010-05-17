#define DEBUGLVL 3
#include "mydebug.h"

#include "mainwindow.h"


MainWindow::MainWindow()
{
	MYTRACE("MainWindow::MainWindow");
	setupUi(this);

	model = new CharacterModel(this);
	table->setModel(model);
}
