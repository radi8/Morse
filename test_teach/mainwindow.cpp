#define DEBUGLVL 0
#include "mydebug.h"

#include "mainwindow.h"
#include "teach_morse.h"
#include "morse.h"


MainWindow::MainWindow()
{
	MYTRACE("MainWindow::MainWindow");
	setupUi(this);

	teach = new TeachMorse(this);
	morse = new GenerateMorse(this);

	//morseDisplay->setVisible(false);
	//showMorse->setChecked(false);
	connect(showMorse, SIGNAL(toggled(bool)), morseDisplay, SLOT(setVisible(bool)) );

	connect(groupSpinBox, SIGNAL(valueChanged(int)), teach, SLOT(setGroups(int)) );
	connect(generateButton, SIGNAL(clicked()), teach, SLOT(generateGroups()) );
	connect(checkButton, SIGNAL(clicked()), SLOT(slotCheck()) );
	connect(teach, SIGNAL(newText(const QString &)), morseDisplay, SLOT(setPlainText(const QString &)) );

	groupSpinBox->setValue(2);
	teach->generateGroups();
}


void MainWindow::slotCheck()
{
	QString entry = morseEntry->toPlainText().simplified();
	QString plain;

	for (int i=0; i < entry.count(); i++) {
		QString c = entry.mid(i,1);
		if (c.at(0).isUpper()) {
			c = entry.mid(i,2);
			i++;
		}
#if 0
		if (c == "\n")
			c = ".";
#endif
		if (morse->exists(c))
			plain.append(c);
	}

	qDebug("entered '%s'", qPrintable(plain));

	teach->checkText(plain);
	morseEntry->setFocus();
}
