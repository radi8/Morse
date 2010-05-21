#define DEBUGLVL 0
#include "mydebug.h"

/**
 * @file
 * @author Holger Schurig
 *
 * @section DESCRIPTION
 *
 * Generic class that
 * -# can translate cleartext into morse
 * -# can play translated morse
 *
 * Oh, err, it doesn't actually play, but instead generate appropriate
 * signals which other classes can then use to play, display or to control a
 * rig.
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 */


#include "morse.h"
#include "characters.h"

#include <QTimer>


bool Morse::contains(const QString &clearText) const
{
	MYTRACE("Morse::contains(%s)", qPrintable(clearText));
	foreach(MorseCharacter m, chars) {
		MYVERBOSE("  at %s", qPrintable(m.sign));
		if (m.sign == clearText)
			return true;
	}
	return false;
}

const QString Morse::operator[] (const QString &clearText) const
{
	MYTRACE("Morse::operator[](%s)", qPrintable(clearText));
	foreach(MorseCharacter m, chars) {
		if (m.sign == clearText)
			return m.code;
	}
	return QString::null;
}


/*!
 * \brief Class to generate morse code
 *
 * Simple usage:
 * \code
 *   GenerateMorse *morse = new GenerateMorse(this);
 *   connect(morse, SIGNAL(playSound(int)), audio, SLOT(playSound(int)) );
 *   morse->append("73 de");
 *   morse->append("dh3hs");
 *   morse->play();
 * \endcode
 */
GenerateMorse::GenerateMorse(QObject *parent)
	: QObject(parent)
	, playElement(0)
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

	playTimer = new QTimer(this);
	playTimer->setSingleShot(true);
	connect(playTimer, SIGNAL(timeout()), this, SLOT(slotPlayNext()) );
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

/*!
 * \brief Add morse code (in string representation) to morse storage
 *
 * The string representation will be converted into lengths of dits and
 * dahs, but also into spacings (intra-character spacing, between character
 * spacing, word spacing) and then added to \ref morse. The cleartext is also
 * added to \ref clearText.
 *
 * @param dahdits  string representation of morse, e.g. "-."
 * @param clear    clear-text of the same
 *
 * \sa append
 */
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


/*!
 * \brief Add clear-text to morse storage
 *
 * @param str       clear text, e.g. "73 de dh3hs". \a str can be a single
 *                  character, or it can be a character sequence. In all
 *                  cases should it be lower-case! \n
 *                  Use upper-case only to add morse pro-signs. Currently
 *                  the program understands
 *                  - AR: Stop (end of message)
 *                  - AS: Wait (for 10 seconds)
 *                  - BK: Break
 *                  - BT: Separator within message
 *                  - CL: Going off the air
 *                  - CT: Start (beginning of message)
 *                  - DO: Shift to japanese wabun code
 *                  - HH: Error
 *                  - KA: same as CT
 *                  - KN: Invitation to a specific named station to transmit
 *                  - SK: End (end of contact)
 *                  - SN: Understood
 *                  - SO: SOS without word spacings
 *                  - TV: same as BT
 *                  - VA: sama as SK
 *                  - VE: same as SN
 * @param addSpace  optional flag that tells us if a space (as a means to
 *                  delimit words) should be added. This flag is by default
 *                  on, you'd want to set it to falls if you characters
 *                  one-by-one, e.g. when directly feeding typed characters
 *                  into the class.
 *
 * \sa appendMorse
 */
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


/*!
 * \brief Clear the morse storage
 *
 * Clears \ref morse, \ref clearText and resets the replay indexes \ref playIdx
 * and \ref clearIdx.
 */
void GenerateMorse::clear()
{
	MYTRACE("GenerateMorse::clear");

	morse.clear();
	clearText.clear();
	playElement = 0;
	playIdx = 0;
	clearIdx = 0;
	emit currElement(playElement);
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

/*!
 * \brief Play morse
 *
 * Used to actually play morse. As this class doesn't play anything at all,
 * it just emits signals, so other class(es) can visualize / audiolize
 * things.
 *
 * Internally \ref playTimer get's started and whenever something has to be
 * done, one of the signals are emitted. This happens in \ref slotPlayNext().
 *
 * \sa stop(), slotPlayNext(), setLoop()
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

	emit maxElements(totalElements());

	// Nothing left?  Bail out!
	if (morse.isEmpty()) {
		emit hasStopped();
		return;
	}

	// One one small silence back, to stop the sound
	morse.append(intraSpacing);

	playElement = 0;
	playIdx = 0;
	clearIdx = 0;
	playTimer->start(0);
}


/*!
 * \brief Stop playing morse
 *
 * \sa start()
 */
void GenerateMorse::stop()
{
	MYTRACE("GenerateMorse::stop");

	playTimer->stop();
}


/*!
 * \brief Handle next morse event
 *
 * Called from \ref playTimer. \ref playIdx is used to step throught \ref morse
 * and \ref clearIdx is used to step throught \ref clearText. The contents of
 * both lists are then used to emit various signals.
 *
 * Any class (or classes) receiving those signals can then generate sound
 * or controll the PTT of your rig and similar things.
 *
 * As \ref morse contains times in "elements" units, the actual timing
 * is controlled by \ref setWpm() and the other \c setXFactor() functions.
 *
 * \sa play(), playSound(bool), playSound(unsigned int), hasStopped(),
 * charChanged() symbolChanged()
 */
void GenerateMorse::slotPlayNext()
{
	MYTRACE("GenerateMorse::slotPlayNext");

	MYVERBOSE("playIdx %d, count %d", playIdx, morse.count());
	if (playIdx >= morse.count() ) {
		if (playLoop) {
			playElement = 0;
			playIdx = 0;
			clearIdx = 0;
			emit currElement(0);
		} else {
			emit hasStopped();
			return;
		}
	}

	int t = morse[playIdx];
	MYVERBOSE("playIdx %d, play %d", playIdx, t);
	playElement += qAbs(t);
	emit currElement(playElement);

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


/*!
 * \brief return current WPM (words per minute) speed
 *
 * The returned number is based on the word "paris " and the current
 * settings of \ref playWpm, \ref ditFactor, \ref dahFactor, \ref intraFactor,
 * \ref charFactor and \ref wordFactor.
 *
 * \sa setWpm
 */
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


/*!
 * \brief Makes the play() function loop endlessly
 *
 * @param loop   boolean if play() should loop endlessly
 * \sa play()
 */
void GenerateMorse::setLoop(bool loop)
{
	// First make sure that we have a pause at the end if we're in loop
	// mode, but no pause if not.
	while (!morse.isEmpty() && morse.last() < 0)
		morse.removeLast();
	while (!clearText.isEmpty() && clearText.last().isEmpty())
		clearText.removeLast();
	if (loop)
		append(" ");

	playLoop = loop;
}



/*!
 * \brief Set replay speed in words per minute
 *
 * Setting 5 wpm means that playing "paris " 5 times will take exactly
 * one minute.
 *
 * \note The current implementation won't play the last " " word spacing,
 * so if you time this function, you'll measure slightly less than one
 * minute. However, if you \ref setLoop() on, you'll get exactly one minute.
 *
 * \sa getWpm()
 */
void GenerateMorse::setWpm(float wpm)
{
	playWpm = wpm;
};


/*!
 * \brief Set factor to make dits longer or shorter
 *
 * Normally set to 1.0. If you call \ref setWpm() with high speeds, then
 * dits might no longer become audioble. So you can artificially make
 * them longer. \ref setDitFactor(2.0) makes them twice as long as normally.
 *
 * You can direcly wire this slot to the \c valueChanged() signal of a
 * \c QDoubleSpinBox.
 */
void GenerateMorse::setDitFactor(float factor)
{
	ditFactor = factor;
};


/*!
 * \brief Set factor to make dahs longer or shorter
 */
void GenerateMorse::setDahFactor(float factor)
{
	dahFactor = factor;
};


/*!
 * \brief Set factor to make the silence between dits and dahs longer or
 * shorter
 *
 * Here the dits and dahs inside a morse sign are meant. E.g. for the morse
 * code of the character "n" the duration between the "-" and "." can be
 * made shorter when calling \ref setIntraFactor(0.8).
 */
void GenerateMorse::setIntraFactor(float factor)
{
	intraFactor = factor;
};


/*!
 * \brief Set factor to make the silence between characters longer or
 * shorter
 *
 * This can be used for the Koch/Farnsworth learning method, e.g. you can
 * have a high \ref setWpm(), but a low \ref setCharFactor().
 */
void GenerateMorse::setCharFactor(float factor)
{
	charFactor = factor;
};


/*!
 * \brief Set factor to make the silence between words longer or
 * shorter
 *
 * This can be used for the Koch/Farnsworth learning method, e.g. you can
 * have a high \ref setWpm(), but a low \ref setCharFactor().
 */
void GenerateMorse::setWordFactor(float factor)
{
	wordFactor = factor;
};
