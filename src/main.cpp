#include "lab1.h"
#include <QtWidgets/QApplication>


int main(int argc, char *argv[]) {
	QApplication a(argc, argv);
	lab1 w;
	w.setFixedSize(w.size());
	w.show();
	return a.exec();
}
