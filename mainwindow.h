#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <windows.h>
#include <array>
#include <QButtonGroup>
#include <QThread>
#include <QMediaPlayer>
#include <QAudioOutput>
#include "customwidget.h"
#include "coordinatecapturewindow.h"
#include "eventinjector.h"

QT_BEGIN_NAMESPACE

namespace Ui
{
	class MainWindow;
}

QT_END_NAMESPACE

class EventInjector;

enum pages
{
	IOCONFIGPAGE,
	ABOUTPAGE,
};
enum inputKeyType
{
	MOUSELEFTKEY,
	MOUSEMIDDLEKEY,
	MOUSERIGHTKEY,
	DIYKEY,
};
enum inputActionType
{
	CLICKS,
	PRESS,
};
enum cursorMoveType
{
	FREE,
	LOCK,
};

class MainWindow : public CustomWidget
{
Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = nullptr);
	~MainWindow() override;

	QString primaryWidgetColor;
	QString secondaryWidgetColor;

protected:
	void mousePressEvent(QMouseEvent *event) override;
	void mouseMoveEvent(QMouseEvent *event) override;
	void mouseReleaseEvent(QMouseEvent *event) override;

private:
	Ui::MainWindow *ui;
	QMediaPlayer *soundEffectPlayer;
	QAudioOutput *soundEffectAudioOutput;
	QThread *eventInjectorThread;
	EventInjector *eventInjector;
	static const std::unordered_map<DWORD, std::string> keyMap;     // 虚拟键值映射表
	QButtonGroup *pageButtonGroup;
	QButtonGroup *inputKeyButtonGroup;
	QButtonGroup *inputActionButtonGroup;
	QButtonGroup *cursorMoveButtonGroup;
	bool mainWindowPin;
	bool mousePressed;
	bool getGlobalSwitchKeyHook_flag;                               // 获取全局开关键钩子工作状态
	bool getDIYKeyHook_flag;                                        // 获取自定义按键钩子工作状态
	bool coordinateCaptureWidget_flag;                              // 坐标捕获窗口工作状态
	QPoint mouseStartPoint;                                         // 记录鼠标按下时的全局位置
	QPoint windowStartPoint;                                        // 记录窗口初始位置
	int pageNum;                                                    // 页码
	int inputKey;                                                   // 输入按键
	int inputActionMode;                                            // 输入行为模式
	int cursorMoveMode;                                             // 光标位置模式
	DWORD globalSwitchKey;                                          // 全局开关键
	DWORD diyKey;                                                   // 自定义按键
	std::array<int, 2> coordinateXY;                                // 坐标

private slots:

	static void applicationExit();                          // 程序退出
	void applicationMinimize();                             // 程序最小化
	void applicationPin();                                  // 程序置顶
	void getGlobalSwitchKey(bool status);                   // 获取全局开关键
	void getDIYKey();                                       // 获取自定义按键
	void obtainedKey(DWORD keyCode);                        // 得到按键
	void startCoordinateCapture(bool status);               // 开始坐标捕获
	void getCursorCoordinate(int x, int y);                 // 获取光标坐标
	void globalSwitchMonitor(DWORD keyCode);                // 全局开关监控器

signals:

	void startEventInjector(int inputKey,
	                        int inputActionMode,
	                        int cursorMoveMode,
	                        DWORD diyKey,
	                        int x,
	                        int y,
	                        double eventCycle);             // 启动事件注入器
};

#endif // MAINWINDOW_H
