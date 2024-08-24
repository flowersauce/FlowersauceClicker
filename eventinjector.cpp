#include "eventinjector.h"

EventInjector::EventInjector(QObject* parent) :
    QObject(parent),
    eventTimer(nullptr),
    inputEvent{{0}} {}

void EventInjector::startTimer(int inputKey,
                               int inputActionMode,
                               int cursorMoveMode,
                               DWORD diyKey,
                               int x,
                               int y,
                               double eventCycle)
{
    // 设置定时器周期
    eventTimer = new QTimer(this);
    if (inputActionMode == CLICKS)
    {
        eventTimer->setInterval(static_cast<int>(eventCycle * 1000.0));
    }
    else
    {
        eventTimer->setInterval(10); // ms
    }

    // 根据参数配置INPUT结构体
    int eventNum = 2;
    if (inputActionMode == PRESS)
    {
        eventNum = 1;
    }

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    x = (x * 65535) / screenWidth;
    y = (y * 65535) / screenHeight;

    inputEvent[0] = {0};
    inputEvent[1] = {0};
    switch (inputKey)
    {
    case MOUSELEFTKEY:
        {
            inputEvent[0].type = INPUT_MOUSE;
            inputEvent[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
            inputEvent[1].type = INPUT_MOUSE;
            inputEvent[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;
            if (cursorMoveMode == LOCK)
            {
                inputEvent[0].mi.dwFlags |= MOUSEEVENTF_MOVE;
                inputEvent[0].mi.dwFlags |= MOUSEEVENTF_ABSOLUTE;
                inputEvent[0].mi.dx = x;
                inputEvent[0].mi.dy = y;
            }
            break;
        }
    case MOUSEMIDDLEKEY:
        {
            inputEvent[0].type = INPUT_MOUSE;
            inputEvent[0].mi.dwFlags = MOUSEEVENTF_MIDDLEDOWN;
            inputEvent[1].type = INPUT_MOUSE;
            inputEvent[1].mi.dwFlags = MOUSEEVENTF_MIDDLEUP;
            if (cursorMoveMode == LOCK)
            {
                inputEvent[0].mi.dwFlags |= MOUSEEVENTF_MOVE;
                inputEvent[0].mi.dwFlags |= MOUSEEVENTF_ABSOLUTE;
                inputEvent[0].mi.dx = x;
                inputEvent[0].mi.dy = y;
            }
            break;
        }
    case MOUSERIGHTKEY:
        {
            inputEvent[0].type = INPUT_MOUSE;
            inputEvent[0].mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
            inputEvent[1].type = INPUT_MOUSE;
            inputEvent[1].mi.dwFlags = MOUSEEVENTF_RIGHTUP;
            if (cursorMoveMode == LOCK)
            {
                inputEvent[0].mi.dwFlags |= MOUSEEVENTF_MOVE;
                inputEvent[0].mi.dwFlags |= MOUSEEVENTF_ABSOLUTE;
                inputEvent[0].mi.dx = x;
                inputEvent[0].mi.dy = y;
            }
            break;
        }
    case DIYKEY:
        {
            inputEvent[0].type = INPUT_KEYBOARD;
            inputEvent[0].ki.wVk = diyKey;
            inputEvent[0].ki.dwFlags = 0;

            inputEvent[1].type = INPUT_KEYBOARD;
            inputEvent[1].ki.wVk = diyKey;
            inputEvent[1].ki.dwFlags = KEYEVENTF_KEYUP;
            break;
        }
    default:
        break;
    }

    connect(eventTimer, &QTimer::timeout, this, [=]()
    {
        SendInput(eventNum, inputEvent, sizeof(INPUT));
    });

    eventTimer->start();
    qDebug() << "DEBUG: Timer start.";
}

void EventInjector::killTimer()
{
    eventTimer->stop();
    eventTimer->deleteLater();
    eventTimer = nullptr;
    qDebug() << "DEBUG: Timer kill.";
}
