#define DEBUGLVL 0
#include "mydebug.h"

#include "teach_morse.h"
#include "diff_match_patch.h"


// http://www.dj4uf.de/morsen/morsen.html


TeachMorse::TeachMorse(QObject *parent)
	: QObject(parent)
	, factor(0)
{
	MYTRACE("TeachMorse::TeachMorse");

	for (int i=0; i<256; i++) {
		enabled[i] = false;
		right[i] = 0;
		wrong[i] = 0;
		lastWrong[i] = false;
	}

	setCharacters("esno");
}


void TeachMorse::setCharacters(const QString &chars)
{
	MYVERBOSE("TeachMorse::setCharacters(%s)", qPrintable(chars));

	foreach(QChar c, chars)
		enableCharacter(c.toLatin1());
}


void TeachMorse::enableCharacter(char c)
{
	MYTRACE("TeachMorse::enableCharacter(%c)", c);

	enabled[(unsigned char)c] = true;
}


void TeachMorse::disableCharacter(char c)
{
	MYTRACE("TeachMorse::disableCharacter(%c)", c);

	enabled[(unsigned char)c] = false;
}


void TeachMorse::disableAllCharacters()
{
	MYTRACE("TeachMorse::disableAllCharacters");

	for (int i=0; i<256; i++)
		enabled[i] = false;
}


QString TeachMorse::getCharacters() const
{
	QString s;

	for (int c=0; c < 256; c++)
		if (enabled[c])
		    s.append((char)c);
	return s;
}


void TeachMorse::setGroups(int n)
{
	MYVERBOSE("TeachMorse::setGroups(%d)", n);

	groups = n;
}


void TeachMorse::setFactor(double f)
{
	MYVERBOSE("TeachMorse::setFactor(%f)", f);

	factor = f;
}

void TeachMorse::generateGroups()
{
	MYVERBOSE("TeachMorse::generateGroups");

	clearText.clear();

	QString src = getCharacters();
	MYVERBOSE("  %d enabled entries", src.count());

	// Add characters that were decoded wrongly in the last run a second
	// time:
	for (int i=0; i<256; i++)
		if (enabled[i] && lastWrong[i])
			src.append((char)i);
	MYVERBOSE("  after lastWrong: %d", src.count());

	// characters which have been misdetected more often than others
	// should be asked more often:
	if (factor != 0) {
		for (int i=0; i<256; i++) {
			if (!wrong[i] || !enabled[i])
				continue;
			if (100.0 / (right[i]+wrong[i]) * wrong[i] > factor)
				src.append((char)i);
		}
	}
	MYVERBOSE("  after factor consideration: %d", src.count());

	int n = src.count();
	for (int i=0; i<groups; i++) {
		if (i)
			clearText.append(" ");
		for (int j=0; j<5; j++) {
			clearText.append( src.at(qrand() % n) );
		}
	}
	MYVERBOSE("  text now '%s'", qPrintable(clearText));

	clearText.prepend("KA ");
	clearText.append(" AR");
	emit newText(clearText);
}


void TeachMorse::checkText(const QString &text)
{
	MYTRACE("TeachMorse::checkText(%s)", qPrintable(text));

	// Reset lastWrong status
	for (int i=0; i<256; i++)
		if (enabled[i])
		    lastWrong[i] = false;

	int srcPos = 0;
	int n;
	unsigned char c;

	diff_match_patch dmp;
	QList<Diff> dl = dmp.diff_main(clearText, text);
	int totalRight = 0;
	int totalWrong = 0;
	foreach(Diff d, dl) {
		switch (d.operation) {
		case INSERT: {
			// this is what the user types, mark bad?
			//MYVERBOSE("insert %d '%s'", d.text.count(), qPrintable(d.text));
			break;
		}
		case DELETE:
			n = d.text.count();
			MYVERBOSE("delete %d '%s'", n, qPrintable(d.text));
			// mark bad
			while (n--) {
				c = clearText.at(srcPos++).toLatin1();
				if (enabled[c]) {
					MYDEBUG("mark %c as bad", c);
					wrong[c]++;
					totalWrong++;
					lastWrong[c] = true;
				}
			}
			break;
		case EQUAL:
			n = d.text.count();
			MYVERBOSE("equal %d", n);
			// mark good
			while (n--) {
				c = clearText.at(srcPos++).toLatin1();
				if (enabled[c]) {
					MYDEBUG("mark %c as good", c);
					right[c]++;
					totalRight++;
				}
			}
			break;
		}
	}
	emit checkResults(totalRight, totalWrong);

	//qDebug("delta: '%s'", qPrintable( dmp.diff_toDelta(dl)));
}
