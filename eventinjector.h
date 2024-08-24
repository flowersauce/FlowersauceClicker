#ifndef EVENTINJECTOR_H
#define EVENTINJECTOR_H

#include <windows.h>
#include <QTimer>
#include "mainwindow.h"

class EventInjector : public QObject
{
Q_OBJECT

public:
	explicit EventInjector(QObject *parent = nullptr);

	~EventInjector() = default;

private:
	QTimer *eventTimer;
	INPUT inputEvent[2];

public slots:

	void startTimer(int inputKey,
	                 int inputActionMode,
	                 int cursorMoveMode,
	                 DWORD diyKey,
	                 int x,
	                 int y,
	                 double eventCycle
	);
	void killTimer();
};

#endif //EVENTINJECTOR_H
