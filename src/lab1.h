#ifndef LAB1_H
#define LAB1_H

#include <QtWidgets/QMainWindow>
#include "ui_lab1.h"

class lab1 : public QMainWindow {
	Q_OBJECT

public:
	lab1(QWidget *parent = 0);
	~lab1();

private:
	Ui::lab1Window ui;
};

#endif // LAB1_H
