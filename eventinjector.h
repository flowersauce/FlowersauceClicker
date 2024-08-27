#ifndef EVENTINJECTOR_H
#define EVENTINJECTOR_H

#include <QObject>
#include <QThread>
#include <windows.h>
#include "globaldefinitions.h"

class EventInjector : public QObject
{
Q_OBJECT

public:
	explicit EventInjector(QObject *parent = nullptr);
	~EventInjector() override = default;
	std::atomic<bool> eventInjector_flag;

private:
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
};

#endif //EVENTINJECTOR_H
