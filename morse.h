#ifndef MORSE_H
#define MORSE_H

/**
 * @file
 * @author Holger Schurig
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


#include <QObject>
#include <QString>
#include <QHash>


class QTimer;


/*!
 * \brief Class to generate and play morse code
 */
class GenerateMorse : public QObject {
	Q_OBJECT

public:
	GenerateMorse(QObject *parent=0);
	/*! Checks if the morse code for \c clearText exists */
	bool exists(QString clearText) { return codes.contains(clearText); };
private:
	void store(const QString &sign, const QString &code);
	/*! \brief Translation from characters to morse-code */
	QHash<QString, QString> codes;
public:
	void append(const QString &s, bool addSpace=true);
	void appendMorse(const QString &dahdits, const QString &clear);
	int  totalElements(int from=0) const; //!< Total elements in \ref morse.
public slots:
	void clear();
	/*! \brief Set new text */
	void setText(const QString &s) { clear(); append(s); };
private:
	/*!
	 * \brief Morse storage
	 *
	 * Here we store elements of sound and silence. To make things
	 * simple, we use positive values for sound and negative values for silence:
	 * - 1  one element sound, representing a dit
	 * - 3  three element sound, representing a dah
	 * - -1 one element silence, represinting the silence inside between
	 *      dits and dahs inside a character
	 * - -3 three elements silence, representing the silence between two
	 *      morse characters
	 * - -7 seven elements silence, representing the silence between two
	 *      word
	 * - 0  next \ref clearText entry
	 *
	 * \sa playIdx
	 */
	QList<int> morse;
	/*!
	 * \brief Clear text storage
	 *
	 * Stores the clear text for each morse characters from \ref morse.
	 * Whenever a \c 0 is found in \ref morse, the next entry from \ref
	 * clearText will be be used in \ref charChanged().
	 *
	 * \sa clearIdx
	 * \sa charChanged
	 */
	QList<QString> clearText;

public slots:
	void play();
	void stop();
	void setLoop(bool b);
	void setWpm(float wpm);
	void setDitFactor(float factor);
	void setDahFactor(float factor);
	void setIntraFactor(float factor);
	void setCharFactor(float factor);
	void setWordFactor(float factor);
public:
	float getWpm() const;
signals:
	/*! \brief Emitted whenever the sound should be turned on or off */
	void playSound(bool onoff);
	/*! \brief Emitted whenever the sound should be turned on for for the specified
	milliseconds */
	void playSound(unsigned int ms);
	/*! \brief Emitted whenever \ref play() finished. \sa play() */
	void hasStopped();
	/*! \brief Emitted whenever a new cleartext characters get's morsed */
	void charChanged(const QString &);
	/*! \brief Emitted whenever a new dit or dah get's morsed */
	void symbolChanged(const QString &);
	/*! \brief Emits how many duration elements are stored \ref morse.
	 * Usage:
	 * \code
	 *    connect(morse, SIGNAL(maxElements(int), progressBar, SLOT(setMaximum(int)) );
	 * \endcode
	 */
	void maxElements(int);
	/*! \brief Emits the current position inside \ref morse
	 * Usage:
	 * \code
	 *    connect(morse, SIGNAL(currElement(int), progressBar, SLOT(setValue(int)) );
	 * \endcode
	 */
	void currElement(int);
private:
	/*! \brief Element index from \ref morse. \sa maxElements(), currElement() */
	int playElement;
	/*! \brief Index into \ref morse */
	int playIdx;
	/*! \brief Index into \ref clearText */
	int clearIdx;
	/*! \brief Timer for \ref play(), used to call \ref slotPlayNext() */
	QTimer *playTimer;
	/*! \brief Should \ref play() loop?  \sa setLoop() */
	float playLoop;
	/*! \brief Current replay speed \sa setWpm() */
	float playWpm;
	/*! \brief Current dit facto, normally 1.0. \sa setDitFactor() */
	float ditFactor;
	/*! \brief Current dah factor, normally 1.0. \sa setDahFactor() */
	float dahFactor;
	/*! \brief Current intra-character spacing, normally 1.0. \sa setIntraFactor() */
	float intraFactor;
	/*! \brief Current character spacing, normally 1.0. \sa setCharFactor() */
	float charFactor;
	/*! \brief Current word spacing, normally 1.0. \sa setWordFactor() */
	float wordFactor;
private slots:
	void slotPlayNext();
};

#endif
