#define DEBUGLVL 0
#include "mydebug.h"

#include "morse.h"

#include <QTimer>



GenerateMorse::GenerateMorse(QObject *parent)
	: QObject(parent)
	, playIdx(0)
	, clearIdx(0)
	, playLoop(false)
	, playWpm(5)
	, ditFactor(1)
	, dahFactor(1)
	, intraFactor(1)
	, charFactor(1)
	, wordFactor(1)
{
	MYTRACE("GenerateMorse::GenerateMorse");
	// http://www.qsl.net/dk5ke/wort.html
	// http://de.wikipedia.org/wiki/Morsecode
	store("a",   ".-");
	store("b",   "-...");
	store("c",   "-.-.");
	store("d",   "-..");
	store("e",   ".");
	store("f",   "..-.");
	store("g",   "--.");
	store("h",   "....");
	store("i",   "..");
	store("j",   ".---");
	store("k",   "-.-");
	store("l",   ".-..");
	store("m",   "--");
	store("n",   "-.");
	store("o",   "---");
	store("p",   ".--.");
	store("q",   "--.-");
	store("r",   ".-.");
	store("s",   "...");
	store("t",   "-");
	store("u",   "..-");
	store("v",   "...-");
	store("w",   ".--");
	store("x",   "-..-");
	store("y",   "-.--");
	store("z",   "--..");
	store("1",   ".----");
	store("2",   "..---");
	store("3",   "...--");
	store("4",   "....-");
	store("5",   ".....");
	store("6",   "-....");
	store("7",   "--...");
	store("8",   "---..");
	store("9",   "----.");
	store("0",   "-----");
	store("*oA", ".--.-");
	store("ä",   ".-.-");
	store("ö",   "---.");
	store("ü",   "..--");
	store("*",   "--..--");
	store(":",   "---...");
	store(";",   "-.-.-.");
	store("?",   "..--..");
	store("-",   "-....-");
	store("_",   "..--.-");
	store("(",   "-.--.");
	store(")",   "-.--.-");
	store("'",   ".----.");
	store("=",   "-...-");
	store("+",   ".-.-.");
	store("/",   "-..-.");
	store("@",   ".--.-.");
	store("CH",  "----");
	store("KA",  "-.-.-"); // Spruchanfang
	store("BT",  "-..-."); // Pause
	store("AR",  ".-.-."); // Spruchende
	store("VE",  "...-."); // Verstanden
	store("SK",  "...-.-"); // Verkehrsende
	store("HH",  "........"); // Irrung
	store(" ",   " ");

	playTimer = new QTimer(this);
	playTimer->setSingleShot(true);
	connect(playTimer, SIGNAL(timeout()), this, SLOT(slotPlayNext()) );
}


void GenerateMorse::store(const QString &sign, const QString &code)
{
	//MYTRACE("GenerateMorse::store('%s', '%s', '%s')",
	//        qPrintable(sign), qPrintable(code), qPrintable(help));

	if (codes.contains(sign))
		qFatal("'%s' already defined as '%s'",
		       qPrintable(sign), qPrintable(codes[sign]) );
	codes[sign] = code;
}


/*
 * Section: adding morse characters and cleartext
 */

/* NOTE: all values must be different ! */
const int ditLength = 1;
const int dahLength = 3;
const int intraSpacing = -1;
const int charSpacing = -3;
const int wordSpacing = -7;

void GenerateMorse::appendMorse(const QString &dahdits, const QString &clear)
{
	MYTRACE("GenerateMorse::appendMorse('%s', '%s')",
	        qPrintable(dahdits), qPrintable(clear) );

	int last = 0;

	morse.append(0);
	clearText.append(clear);

	for (int i = 0; i < dahdits.size(); ++i) {
		char c = dahdits.at(i).toAscii();

		if (!morse.isEmpty()) {
			last = morse.last();
			//MYVERBOSE("last %d", last);
		}

		MYVERBOSE("  append '%c'", c);
		switch (c) {
		case '.':
			if (last > 0) {
				MYVERBOSE("  DIT %d", intraSpacing);
				morse.append(intraSpacing);
			}
			MYVERBOSE("  dit %d, last %d", ditLength, last);
			morse.append(ditLength);
			break;
		case '-':
			if (last > 0) {
				MYVERBOSE("  DAH %d", intraSpacing);
				morse.append(intraSpacing);
			}
			MYVERBOSE("  dah %d, last %d", dahLength, last);
			morse.append(dahLength);
			break;
		case ' ':
			if (last == charSpacing) {
				MYVERBOSE("  spc %d, last %d (promoted from charSpacing)", wordSpacing, last);
				morse.last() = wordSpacing;
			} else {
				MYVERBOSE("  spc %d, last %d", wordSpacing, last);
				morse.append(wordSpacing);
			}
			break;
		}
	}
	if (!morse.isEmpty())
		last = morse.last();
	if (last != wordSpacing) {
		MYVERBOSE("  chr %d, last %d", charSpacing, last);
		morse.append(charSpacing);
	}
}


void GenerateMorse::append(const QString &str, bool addSpace)
{
	MYTRACE("GenerateMorse::append('%s')", qPrintable(str) );

	if (codes.contains(str)) {
		appendMorse(codes[str], str);
		return;
	}

	for (int i=0; i<str.count(); i++) {
		QString c = str.mid(i,1);
		MYVERBOSE("processing '%s'", qPrintable(c));
		if (c.at(0).isUpper()) { // && i < str.count()-1) {
			c = str.mid(i,2);
			MYVERBOSE("  special '%s'", qPrintable(c));
			i++;
		}
		if (codes.contains(c)) {
			appendMorse(codes[c], c);
		} else {
			qFatal("no morse code for '%s' known", qPrintable(c));
		}
	}
	if (addSpace)
		appendMorse(" ", " ");
}


void GenerateMorse::clear()
{
	MYTRACE("GenerateMorse::clear");

	morse.clear();
	clearText.clear();
	playIdx = 0;
	clearIdx = 0;
}


int GenerateMorse::totalElements(int from) const
{
	MYTRACE("GenerateMorse::totalElements");

	int dits = 0;
	int dahs = 0;
	int intras = 0;
	int chars = 0;
	int words = 0;

	int sum = 0;
	for (int i=from; i < morse.count(); i++) {
		int elem = morse.at(i);
		switch (elem) {
		case ditLength: dits++; break;
		case dahLength: dahs++; break;
		case intraSpacing: intras++; break;
		case charSpacing: chars++; break;
		case wordSpacing: words++; break;
		}
		//MYVERBOSE("%d: %d", i, elem );
		sum += qAbs(elem);
	}
	MYVERBOSE("  elements %d, dits %d, dahs %d, intras %d, chars %d, words %d",
	          sum, dits, dahs, intras, chars, words);
	return sum;
}



/*
 * Section: playing
 */

void GenerateMorse::play()
{
	MYTRACE("GenerateMorse::play");

	if (!playLoop) {
		// Remove all trailing silence. Note that we don't do this
		// in loop mode, otherwise we'd jam the end of the text to
		// the start of the text with no pause at all.
		while (!morse.isEmpty() && morse.last() < 0)
			morse.removeLast();
		while (!clearText.isEmpty() && clearText.last().isEmpty())
			clearText.removeLast();
	}

	// Nothing left?  Bail out!
	if (morse.isEmpty()) {
		emit hasStopped();
		return;
	}

	// One one small silence back, to stop the sound
	morse.append(intraSpacing);

	playIdx = 0;
	clearIdx = 0;
	playTimer->start(0);
}


void GenerateMorse::stop()
{
	MYTRACE("GenerateMorse::stop");

	playTimer->stop();
}


void GenerateMorse::slotPlayNext()
{
	MYTRACE("GenerateMorse::slotPlayNext");

	MYVERBOSE("playIdx %d, count %d", playIdx, morse.count());
	if (playIdx >= morse.count() ) {
		if (playLoop) {
			playIdx = 0;
			clearIdx = 0;
		} else {
			emit hasStopped();
			return;
		}
	}

	int t = morse[playIdx];
	MYVERBOSE("playIdx %d, play %d", playIdx, t);

	float factor = 1;
	switch (t) {
	case 0:
		{
			QString clear = clearText[clearIdx++];
			MYVERBOSE("  cleartext '%s'", qPrintable(clear));
			emit charChanged(clear);
		}
		break;
	case ditLength:
		emit symbolChanged(".");
		emit playSound(true);
		factor = ditFactor;
		break;
	case dahLength:
		emit symbolChanged("-");
		emit playSound(true);
		factor = dahFactor;
		break;
	case intraSpacing:
		emit symbolChanged(" ");
		emit playSound(false);
		factor = intraFactor;
		break;
	case charSpacing:
		emit charChanged(" ");
		emit symbolChanged(" ");
		emit playSound(false);
		factor = charFactor;
		break;
	case wordSpacing:
		emit charChanged(" ");
		emit symbolChanged(" ");
		emit playSound(false);
		factor = wordFactor;
		break;
	}

	playIdx++;


	// http://forums.qrz.com/showthread.php?t=178795

	// paris = 50 elements -> 1 word per minuts = 50 elements per minute
	//
	//        60s / wpm * 50 elements = x  s/element
	// 1000 * 60s / wpm * 50 elements = x ms/element
	//     60000s / wpm * 50 elements = x ms/element
	//      1200  / wpm               = x ms/element

	float length = 1200 / playWpm;
	//MYVERBOSE("elem length: %f ms", length);
	length *= qAbs(t) * factor;

	if (t > 0)
		emit playSound((unsigned int)length);

	//MYVERBOSE("delay: %f ms", length);
	playTimer->start(length);
}


float GenerateMorse::getWpm() const
{
	MYTRACE("GenerateMorse::getWpm");

	// "paris " has exactly
	//    10 dits
	//     4 dahs
	//     9 intra-character spaces
	//     4 character spaces
	//     1 word space

	float length = 0;
	float ms = 1200 / playWpm;
	length += ms * 10 * ditLength * ditFactor;
	length += ms *  4 * dahLength * dahFactor;
	length += ms * -9 * intraSpacing * intraFactor;
	length += ms * -4 * charSpacing * charFactor;
	length += ms * -1 * wordSpacing * wordFactor;
	length /= 1000; // convert von ms to s
	return 60 / length;
}


void GenerateMorse::setLoop(bool b)
{
	// First make sure that we have a pause at the end if we're in loop
	// mode, but no pause if not.
	while (!morse.isEmpty() && morse.last() < 0)
		morse.removeLast();
	while (!clearText.isEmpty() && clearText.last().isEmpty())
		clearText.removeLast();
	if (b)
		append(" ");

	playLoop = b;
}
