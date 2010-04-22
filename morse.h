#ifndef MORSE_H
#define MORSE_H


#include <QObject>
#include <QString>
#include <QHash>


class QTimer;


class GenerateMorse : public QObject {
	Q_OBJECT

/*
 * This section contains the code that stores the morse code, e.g. a
 * translation from "n" to "-." (dahdit). That's stored into "codes".
 *
 * The has "pronunciation" contains words that roughtly have the same
 * rhythm as the dahdits.
 */
public:
	GenerateMorse(QObject *parent=0);
private:
	void store(const QString &sign, const QString &code);
	QHash<QString, QString> codes;            // e.g. "-.."

/*
 * Here we append letters (or special morse-sign like AR / SK etc) into a
 * list of dah-dits ("morse") and a list of cleartext ("clearText"). This
 * can later be used to generate tones.
 */
public:
	void append(const QString &s, bool addSpace=true);
	void appendMorse(const QString &dahdits, const QString &clear);
	int  totalElements(int from=0) const;
public slots:
	void clear();
private:
	// Here we store elements of sound and silence:
	// 1  one element sound
	// -1 one element silence
	// -7 seven elements silence
	// 0  next clearText
	QList<int> morse;
	QList<QString> clearText;


/*
 * This is finally used to actually play morse. As this class doesn't
 * play anything at all, it just emits signals, so other class(es) can
 * visualize / audiolize things.
 */
public slots:
	void play();
	void stop();
	void setLoop(bool b);
	void setWpm(float wpm) { playWpm = wpm; };
	void setDitFactor(float factor) { ditFactor = factor; };
	void setDahFactor(float factor) { dahFactor = factor; };
	void setIntraFactor(float factor) { intraFactor = factor; };
	void setCharFactor(float factor) { charFactor = factor; };
	void setWordFactor(float factor) { wordFactor = factor; };
public:
	float getWpm() const;
signals:
	void playSound(bool);
	void playSound(unsigned int);
	void hasStopped();
	void charChanged(const QString &);
	void symbolChanged(const QString &);
private:
	int playIdx;
	int clearIdx;
	QTimer *playTimer;
	float playLoop;
	float playWpm;
	float ditFactor;
	float dahFactor;
	float intraFactor;
	float charFactor;
	float wordFactor;
private slots:
	void slotPlayNext();
};

#endif
