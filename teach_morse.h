#ifndef TEACH_MORSE_H
#define TEACH_MORSE_H


#include <QObject>
#include <QList>


class TeachMorse : public QObject {
	Q_OBJECT
public:
	TeachMorse(QObject *parent=0);
public slots:
	void setCharacters(const QString &chars);
	void enableCharacter(char c);
	void disableCharacter(char c);
	void disableAllCharacters();
	QString getCharacters() const;

	void setGroups(int n);
	void setFactor(double f);
	void generateGroups();

	QString getText() const { return clearText; }
	void checkText(const QString &text);
signals:
	void newText(const QString &);
private:
	bool enabled[256];
	bool lastWrong[256];
	quint32 right[256];
	quint32 wrong[256];

	int groups;
	QString clearText;
	double factor;
};


#endif
