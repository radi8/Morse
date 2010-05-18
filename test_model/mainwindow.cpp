#define DEBUGLVL 3
#include "mydebug.h"

#include "mainwindow.h"
#include "characters.h"

static void store(const QString &sign, const QString &code)
{
	MorseCharacter c;
	foreach(c, chars) {
		if (c.sign == sign) {
			qWarning("'%s' already defined", qPrintable(sign));
			return;
		}
	}
	c.number = chars.count();
	c.sign = sign;
	c.code = code;
	c.right = 0;
	c.wrong = 0;
	c.enabled = false;
	c.lastWrong = false;
	chars.append(c);
}


MainWindow::MainWindow()
{
	MYTRACE("MainWindow::MainWindow");
	setupUi(this);

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
	//store("*oA", ".--.-");
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
	// http://en.wikipedia.org/wiki/Prosigns_for_Morse_code
	store("AR",  ".-.-.");    // end of message
	store("AS",  ".-...");    // wait (followed by seconds)
	store("BK",  "-···-·-");  // Break
	store("BT",  "-..-.");    // separator within message, written as "="
	store("CL",  "-·-··-··"); // going "off-the-air"
	store("CT",  "-·-·-");    // beginning of message, same as KA
	store("DO",  "-··---");   // shift to japanese wabun code
	store("KN",  "-·--·");    // invite names station to send
	store("SK",  "...-.-");   // end of contract
	store("SN",  "...-.");    // understood
	store("SO",  "...---..."); // SOS
	store("VA",  "...-.-");   // same as SK
	store("VE",  "...-.");    // same as SN
	store("KA",  "-.-.-");    // same as CT
	store("TV",  "-..-.");    // same as BT
	store("HH",  "........"); // Error
	store(" ",   " ");

	chars[10].enabled = true; //TODO

	model = new CharacterModel(this);
	table->setModel(model);
}
