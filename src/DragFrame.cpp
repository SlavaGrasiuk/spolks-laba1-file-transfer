#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include "DragFrame.hpp"



/*
==================
DragFrame::DragFrame
==================
*/
DragFrame::DragFrame(QWidget * parent) : QFrame(parent) {
	setFrameShape(QFrame::Box);
	setFrameShadow(QFrame::Raised);
}

/*
==================
DragFrame::dragEnterEvent
==================
*/
void DragFrame::dragEnterEvent(QDragEnterEvent * event) {
	event->acceptProposedAction();
}

/*
==================
DragFrame::dropEvent
==================
*/
void DragFrame::dropEvent(QDropEvent * event) {
	if (event->mimeData()->hasUrls()) {
		auto name = event->mimeData()->urls().at(0).toLocalFile();
		event->acceptProposedAction();
		emit GetFile(name);
	}
}
