#include <QApplication>

// #define TEST_GEN_MORSE

#ifdef TEST_GEN_MORSE
#include "gen_morse.h"
#else
#include "mainwindow.h"
#endif

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
#ifdef TEST_GEN_MORSE
	GenerateMorse *gen_morse = new GenerateMorse();

#if 0
	gen_morse->append("p");
	qDebug("1 -1 3 -1 3 -1 1 (-3) = 14");
	gen_morse->totalElements();

	gen_morse->append("a", false);
	qDebug("1 -1 3 (-3) = 8");
	gen_morse->totalElements(8);

	gen_morse->append("r", false);
	qDebug("1 -1 3 -1 1 (-3) = 10");
	gen_morse->totalElements(12);

	gen_morse->append("i", false);
	qDebug("1 -1 1 (-3) = 6");
	gen_morse->totalElements(18);

	gen_morse->append("s ");
	qDebug("1 -1 1 -1 1 [-7] = 12");
	gen_morse->totalElements(22);
	//gen_morse->totalElements();
#endif

#if 0
	gen_morse->append("paris");
	int elements = gen_morse->totalElements();
	qDebug("%d elements", elements);
#endif

#if 0
	gen_morse->append("paris");
	gen_morse->play();
#endif

#if 0
	gen_morse->setWpm(5);
	gen_morse->setDitFactor(1.0);
	float f = gen_morse->getWpm();
	qDebug("actual wpm: %f", f);
#endif

#else
	MainWindow *main = new MainWindow();
	main->show();
#endif

	int res = app.exec();

#ifdef TEST_GEN_MORSE
	delete gen_morse;
#else
	delete main;
#endif

	return res;
}
