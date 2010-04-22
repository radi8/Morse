#ifndef MORSE_SCROLLER_H
#define MORSE_SCROLLER_H


#include <QLabel>
#include <QList>

class QTimer;


class MorseScroller : public QLabel {
	Q_OBJECT
public:
	MorseScroller(QWidget *parent=0, Qt::WindowFlags f=0);
	virtual void paintEvent(QPaintEvent *event);
public slots:
	void setSound(bool);
private slots:
	void slotScroll();
private:
	QTimer *timer;
	QList<bool> data;
	bool on;
};

#endif
