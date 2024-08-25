#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <qDebug>
#include <unordered_map>
#include <string>
#include <QDoubleValidator>
#include <QFontDatabase>

HHOOK getKeyHook = nullptr;
HHOOK globalSwitchMonitorHook = nullptr;
static MainWindow *mainWindow = nullptr;

// =============================================================================================== HookCallback
// 全局监控回调函数
LRESULT CALLBACK globalSwitchMonitorCallback(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode >= 0 && wParam == WM_KEYDOWN)
	{
		// 检测按键按下事件
		auto *pKeyBoard = (KBDLLHOOKSTRUCT *) lParam;
		DWORD keyCode = pKeyBoard->vkCode;

		if (mainWindow)
		{
			// 确保在主线程中调用槽函数
			QMetaObject::invokeMethod(mainWindow, "globalSwitchMonitor", Qt::QueuedConnection,
			                          Q_ARG(DWORD, keyCode));
		}
	}
	return CallNextHookEx(getKeyHook, nCode, wParam, lParam);
}

// 热键捕获回调函数
LRESULT CALLBACK hotkeyCaptureCallback(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode >= 0 && wParam == WM_KEYDOWN)
	{
		// 检测按键按下事件
		auto *pKeyBoard = (KBDLLHOOKSTRUCT *) lParam;
		DWORD keyCode = pKeyBoard->vkCode;
		// 卸载钩子
		UnhookWindowsHookEx(getKeyHook);
		getKeyHook = nullptr;

		if (mainWindow)
		{
			// 确保在主线程中调用槽函数
			QMetaObject::invokeMethod(mainWindow, "obtainedKey", Qt::QueuedConnection,
			                          Q_ARG(DWORD, keyCode));
		}
	}
	return CallNextHookEx(getKeyHook, nCode, wParam, lParam);
}

// =============================================================================================== Class
MainWindow::MainWindow(QWidget *parent) :
		CustomWidget(parent),
		primaryWidgetColor("#21252b"),
		secondaryWidgetColor("#282c34"),
		ui(new Ui::MainWindow),
		eventInjectorThread(new QThread(this)),
		eventInjector(new EventInjector(nullptr)),
		pageButtonGroup(new QButtonGroup(this)),
		inputKeyButtonGroup(new QButtonGroup(this)),
		inputActionButtonGroup(new QButtonGroup(this)),
		cursorMoveButtonGroup(new QButtonGroup(this)),
		mainWindowPin(false),
		mousePressed(false),
		getGlobalSwitchKeyHook_flag(false),
		getDIYKeyHook_flag(false),
		coordinateCaptureWidget_flag(false),
		pageNum(IOCONFIGPAGE),
		inputKey(MOUSELEFTKEY),
		inputActionMode(CLICKS),
		cursorMoveMode(FREE),
		globalSwitchKey(VK_F8),
		diyKey(0x00),
		coordinateXY({0, 0}),
		eventCycle(1.000)
{
	ui->setupUi(this);
	// 事件注入器线程初始化
	eventInjectorThread->start();
	eventInjector->moveToThread(eventInjectorThread);

	mainWindow = this;
	// 设置全局开关捕获钩子
	globalSwitchMonitorHook = SetWindowsHookEx(WH_KEYBOARD_LL, globalSwitchMonitorCallback, nullptr, 0);

	// 设置按钮组
	pageButtonGroup->addButton(ui->IOConfig_widget_button);
	pageButtonGroup->addButton(ui->about_widget_button);

	inputKeyButtonGroup->addButton(ui->mouseLeftButton);
	inputKeyButtonGroup->addButton(ui->mouseMiddleButton);
	inputKeyButtonGroup->addButton(ui->mouseRightButton);
	inputKeyButtonGroup->addButton(ui->DIYKeyButton);

	inputActionButtonGroup->addButton(ui->clicksButton);
	inputActionButtonGroup->addButton(ui->pressButton);

	cursorMoveButtonGroup->addButton(ui->cursorFreeButton);
	cursorMoveButtonGroup->addButton(ui->cursorLockButton);

	// 设置周期值输入框的输入字符类型和数值范围
	auto *doubleValidator = new QDoubleValidator(0.000, 99999.999, 3, ui->PeriodValueInputLineEdit);
	doubleValidator->setNotation(QDoubleValidator::StandardNotation); // 设置QDoubleValidator为标准计数法 ( 默认为科学计数法 )
	ui->PeriodValueInputLineEdit->setValidator(doubleValidator);

	// 设置外观
	this->setWidgetCornerRadius(28);
	this->setWidgetColor(QColor(primaryWidgetColor));
	this->setWidgetBorderLightness(200);

	ui->titleBar->setWidgetCornerRadius(16);
	ui->titleBar->setWidgetColor(QColor(secondaryWidgetColor));
	ui->titleBar->setWidgetBorderLightness(100);

	ui->function_widget->setWidgetCornerRadius(16);
	ui->function_widget->setWidgetColor(QColor(secondaryWidgetColor));
	ui->function_widget->setWidgetBorderLightness(100);

	ui->IOConfig_bar_1->setWidgetCornerRadius(8);
	ui->IOConfig_bar_1->setWidgetColor(QColor(secondaryWidgetColor).lighter(180));
	ui->IOConfig_bar_1->setWidgetBorderLightness(100);

	ui->IOConfig_bar_2->setWidgetCornerRadius(8);
	ui->IOConfig_bar_2->setWidgetColor(QColor(secondaryWidgetColor).lighter(180));
	ui->IOConfig_bar_2->setWidgetBorderLightness(100);

	ui->IOConfig_bar_3->setWidgetCornerRadius(8);
	ui->IOConfig_bar_3->setWidgetColor(QColor(secondaryWidgetColor).lighter(180));
	ui->IOConfig_bar_3->setWidgetBorderLightness(100);

	ui->IOConfig_bar_4->setWidgetCornerRadius(8);
	ui->IOConfig_bar_4->setWidgetColor(QColor(secondaryWidgetColor).lighter(180));
	ui->IOConfig_bar_4->setWidgetBorderLightness(100);

	ui->IOConfig_bar_5->setWidgetCornerRadius(8);
	ui->IOConfig_bar_5->setWidgetColor(QColor(secondaryWidgetColor).lighter(180));
	ui->IOConfig_bar_5->setWidgetBorderLightness(100);

	ui->IOConfig_bar_6->setWidgetCornerRadius(8);
	ui->IOConfig_bar_6->setWidgetColor(QColor(secondaryWidgetColor).lighter(180));
	ui->IOConfig_bar_6->setWidgetBorderLightness(100);

	// 连接信号槽
	connect(ui->exitButton, &QPushButton::clicked, this, &MainWindow::applicationExit);
	connect(ui->minimizeButton, &QPushButton::clicked, this, &MainWindow::applicationMinimize);
	connect(ui->pinButton, &QPushButton::clicked, this, &MainWindow::applicationPin);
	connect(ui->globalSwitchCaptureButton, &QPushButton::toggled, this, &MainWindow::getGlobalSwitchKey);
	connect(ui->cursorCoordinateCaptureButton, &QPushButton::toggled, this, &MainWindow::startCoordinateCapture);
	connect(ui->IOConfig_widget_button, &QPushButton::clicked, this, [=]()
	{
		pageNum = IOCONFIGPAGE;
		ui->IOConfig_widget->setVisible(true);
		ui->about_widget->setVisible(false);
	});
	connect(ui->about_widget_button, &QPushButton::clicked, this, [=]()
	{
		pageNum = ABOUTPAGE;
		ui->about_widget->setVisible(true);
		ui->IOConfig_widget->setVisible(false);
	});
	connect(ui->mouseLeftButton, &QPushButton::clicked, this, [=]()
	{
		inputKey = MOUSELEFTKEY;
	});
	connect(ui->mouseMiddleButton, &QPushButton::clicked, this, [=]()
	{
		inputKey = MOUSEMIDDLEKEY;
	});
	connect(ui->mouseRightButton, &QPushButton::clicked, this, [=]()
	{
		inputKey = MOUSERIGHTKEY;
	});
	connect(ui->DIYKeyButton, &QPushButton::clicked, this, &MainWindow::getDIYKey);
	connect(ui->DIYKeyButton, &QPushButton::toggled, this, [=](bool status)
	{
		if (!status)
		{
				ui->IOConfig_bar_2->setEnabled(true);
				ui->IOConfig_bar_5->setEnabled(true);
		}
	});
	connect(ui->clicksButton, &QPushButton::clicked, this, [=]()
	{
		inputActionMode = CLICKS;
	});
	connect(ui->pressButton, &QPushButton::toggled, this, [=](bool status)
	{
		if (status)
		{
			inputActionMode = PRESS;
			ui->IOConfig_bar_6->setEnabled(false);
		}
		else
		{
			ui->IOConfig_bar_6->setEnabled(true);
		}
	});
	connect(ui->cursorFreeButton, &QPushButton::clicked, this, [=]()
	{
		cursorMoveMode = FREE;
	});
	connect(ui->cursorLockButton, &QPushButton::clicked, this, [=]()
	{
		cursorMoveMode = LOCK;
	});
	connect(ui->PeriodValueInputLineEdit, &QLineEdit::textChanged, this, [=](const QString &text)
	{
		if (text.isEmpty() || text.toDouble() == 0)
		{
			ui->startButton->setEnabled(false);
		}
		else
		{
			ui->startButton->setEnabled(true);
		}
	});
	connect(ui->startButton, &QPushButton::toggled, this, [=](bool status)
	{
		if (status)
		{
			ui->IOConfig_bar_1->setEnabled(false);
			ui->IOConfig_bar_2->setEnabled(false);
			ui->IOConfig_bar_3->setEnabled(false);
			ui->IOConfig_bar_4->setEnabled(false);
			ui->IOConfig_bar_5->setEnabled(false);
			ui->IOConfig_bar_6->setEnabled(false);
			ui->startButton->setText("-STOP-");

			emit startEventInjector(inputKey, inputActionMode, cursorMoveMode, diyKey, coordinateXY.at(0),
			                        coordinateXY.at(1),
			                        ui->PeriodValueInputLineEdit->text().toDouble());
		}
		else
		{
			ui->IOConfig_bar_1->setEnabled(true);
			ui->IOConfig_bar_2->setEnabled(true);
			ui->IOConfig_bar_3->setEnabled(true);
			ui->IOConfig_bar_4->setEnabled(true);
			ui->IOConfig_bar_5->setEnabled(true);
			ui->IOConfig_bar_6->setEnabled(true);
			ui->startButton->setText("-START-");

			eventInjector->eventInjector_flag = false;
		}
	});
	connect(this, &MainWindow::startEventInjector, eventInjector, &EventInjector::startTimer);

	// 页面初始化
	ui->about_widget->setVisible(false);
	// 设置字体
	int fontId = QFontDatabase::addApplicationFont(":/resources/CascadiaMono.ttf");
	QString fontFamily = QFontDatabase::applicationFontFamilies(fontId).at(0);
	QFont font(fontFamily);
	font.setPointSize(10);
	ui->titleLabel->setFont(font);
	ui->appName_label_A->setFont(font);
	ui->appName_label_B->setFont(font);
}

MainWindow::~MainWindow()
{
	eventInjector->eventInjector_flag = false;
	delete eventInjector;
	eventInjector = nullptr;

	eventInjectorThread->quit();
	eventInjectorThread->wait();
	eventInjectorThread->deleteLater();
	eventInjectorThread = nullptr;

	delete ui;
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
	{
		// 检查鼠标下的控件是否是MainWindow本身
		if (ui->titleBar->rect().contains(event->pos()))
		{
			mousePressed = true;
			mouseStartPoint = event->globalPosition().toPoint();
			windowStartPoint = this->frameGeometry().topLeft();
		}
	}
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
	if (mousePressed)
	{
		QPoint movePoint = event->globalPosition().toPoint() - mouseStartPoint;
		this->move(windowStartPoint + movePoint);
	}
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
	{
		mousePressed = false;
	}
}

void MainWindow::applicationExit()
{
	// 关闭程序
	QApplication::quit();
}

void MainWindow::applicationMinimize()
{
	// 最小化窗口
	this->showMinimized();
}

void MainWindow::applicationPin()
{
	if (!mainWindowPin)
	{
		this->setWidgetBorderLightness(500);
		setWindowFlag(Qt::WindowStaysOnTopHint, true);
		show();
		mainWindowPin = true;
	}
	else
	{
		this->setWidgetBorderLightness(200);
		setWindowFlag(Qt::WindowStaysOnTopHint, false);
		show();
		mainWindowPin = false;
	}
}

void MainWindow::getGlobalSwitchKey(bool status)
{
	if (status)
	{
		// 卸载钩子
		UnhookWindowsHookEx(globalSwitchMonitorHook);
		globalSwitchMonitorHook = nullptr;

		ui->startButton->setEnabled(false);
		ui->IOConfig_bar_1->setEnabled(false);
		ui->IOConfig_bar_3->setEnabled(false);
		ui->globalSwitchCaptureButton->clearFocus();

		if (getKeyHook == nullptr)
		{
			ui->globalSwitchCaptureButton->setText("捕获中");
			getKeyHook = SetWindowsHookEx(WH_KEYBOARD_LL, hotkeyCaptureCallback, nullptr, 0);
			getGlobalSwitchKeyHook_flag = true;
		}
	}
	else
	{
		// 设置全局开关捕获钩子
		globalSwitchMonitorHook = SetWindowsHookEx(WH_KEYBOARD_LL, globalSwitchMonitorCallback, nullptr, 0);

		ui->startButton->setEnabled(true);
		ui->IOConfig_bar_1->setEnabled(true);
		ui->IOConfig_bar_3->setEnabled(true);
	}
}

void MainWindow::getDIYKey()
{
	ui->DIYKeyButton->clearFocus();
	if (getKeyHook == nullptr)
	{
		// 卸载钩子
		UnhookWindowsHookEx(globalSwitchMonitorHook);
		globalSwitchMonitorHook = nullptr;

		ui->IOConfig_bar_1->setEnabled(false);
		ui->IOConfig_bar_2->setEnabled(false);
		ui->IOConfig_bar_3->setEnabled(false);
		ui->IOConfig_bar_5->setEnabled(false);
		ui->startButton->setEnabled(false);

		inputKey = DIYKEY;
		ui->DIYKeyButton->setText("捕获中");

		getKeyHook = SetWindowsHookEx(WH_KEYBOARD_LL, hotkeyCaptureCallback, nullptr, 0);
		getDIYKeyHook_flag = true;
	}
}

void MainWindow::obtainedKey(DWORD keyCode)
{
	auto theKeyValue = keyMap.find(keyCode);
	if (getGlobalSwitchKeyHook_flag)
	{
		if (theKeyValue != keyMap.end() && keyCode != diyKey)
		{
			ui->globalSwitchCaptureButton->setText(QString::fromStdString(theKeyValue->second));
			ui->globalSwitchCaptureButton->setChecked(false);
			globalSwitchKey = keyCode;
			getGlobalSwitchKeyHook_flag = false;
		}
		else
		{
			ui->globalSwitchCaptureButton->setText("请重试");
			getKeyHook = SetWindowsHookEx(WH_KEYBOARD_LL, hotkeyCaptureCallback, nullptr, 0);
		}
	}
	else if (getDIYKeyHook_flag)
	{
		if (theKeyValue != keyMap.end() && keyCode != globalSwitchKey)
		{
			ui->DIYKeyButton->setText(QString::fromStdString(theKeyValue->second));
			diyKey = keyCode;
			getDIYKeyHook_flag = false;

			// 设置全局开关捕获钩子
			globalSwitchMonitorHook = SetWindowsHookEx(WH_KEYBOARD_LL, globalSwitchMonitorCallback, nullptr, 0);

			ui->IOConfig_bar_1->setEnabled(true);
			ui->IOConfig_bar_3->setEnabled(true);
			ui->startButton->setEnabled(true);
		}
		else
		{
			ui->DIYKeyButton->setText("请重试");
			getKeyHook = SetWindowsHookEx(WH_KEYBOARD_LL, hotkeyCaptureCallback, nullptr, 0);
		}
	}
}

void MainWindow::startCoordinateCapture(bool status)
{
	if (status)
	{
		ui->cursorCoordinateCaptureButton->clearFocus();
		ui->cursorCoordinateCaptureButton->setText("捕获中");
		// 动态创建窗口
		auto *captureWindow = new CoordinateCaptureWindow(this);
		// 自动删除窗口对象
		captureWindow->setAttribute(Qt::WA_DeleteOnClose);
		connect(captureWindow, &CoordinateCaptureWindow::coordinatesCaptured, this, &MainWindow::getCursorCoordinate);
		captureWindow->show();
		coordinateCaptureWidget_flag = true;
	}
}

void MainWindow::getCursorCoordinate(int x, int y)
{
	coordinateXY.at(0) = x;
	coordinateXY.at(1) = y;
	auto coordinateXYStr = QString::number(x) + "," + QString::number(y);
	ui->cursorCoordinateCaptureButton->setText(coordinateXYStr);
	ui->cursorCoordinateCaptureButton->setChecked(false);
	coordinateCaptureWidget_flag = false;
}

void MainWindow::globalSwitchMonitor(DWORD keyCode)
{
	if (keyCode == globalSwitchKey)
	{
		ui->startButton->click();
	}
}

std::unordered_map<DWORD, std::string> MainWindow::keyMap = {
		{VK_LBUTTON,    "鼠标左键"},
		{VK_RBUTTON,    "鼠标右键"},
		{VK_MBUTTON,    "鼠标中键"},
		{VK_BACK,       "退格"},
		{VK_TAB,        "TAB"},
		{VK_RETURN,     "回车"},
		{VK_SHIFT,      "SHIFT"},
		{VK_CONTROL,    "CTRL"},
		{VK_CAPITAL,    "CL"},
		{VK_ESCAPE,     "ESC"},
		{VK_SPACE,      "空格"},
		{VK_PRIOR,      "PGUP"},
		{VK_NEXT,       "PGDN"},
		{VK_END,        "END"},
		{VK_HOME,       "HOME"},
		{VK_LEFT,       "LEFT"},
		{VK_UP,         "UP"},
		{VK_RIGHT,      "RIGHT"},
		{VK_DOWN,       "DOWN"},
		{VK_SELECT,     "SEL"},
		{VK_SNAPSHOT,   "截屏"},
		{VK_DELETE,     "DEL"},
		{0x30,          "0"},
		{0x31,          "1"},
		{0x32,          "2"},
		{0x33,          "3"},
		{0x34,          "4"},
		{0x35,          "5"},
		{0x36,          "6"},
		{0x37,          "7"},
		{0x38,          "8"},
		{0x39,          "9"},
		{0x41,          "A"},
		{0x42,          "B"},
		{0x43,          "C"},
		{0x44,          "D"},
		{0x45,          "E"},
		{0x46,          "F"},
		{0x47,          "G"},
		{0x48,          "H"},
		{0x49,          "I"},
		{0x4A,          "J"},
		{0x4B,          "K"},
		{0x4C,          "L"},
		{0x4D,          "M"},
		{0x4E,          "N"},
		{0x4F,          "O"},
		{0x50,          "P"},
		{0x51,          "Q"},
		{0x52,          "R"},
		{0x53,          "S"},
		{0x54,          "T"},
		{0x55,          "U"},
		{0x56,          "V"},
		{0x57,          "W"},
		{0x58,          "X"},
		{0x59,          "Y"},
		{0x5A,          "Z"},
		{VK_LWIN,       "LWIN"},
		{VK_RWIN,       "RWIN"},
		{VK_NUMPAD0,    "NUM 0"},
		{VK_NUMPAD1,    "NUM 1"},
		{VK_NUMPAD2,    "NUM 2"},
		{VK_NUMPAD3,    "NUM 3"},
		{VK_NUMPAD4,    "NUM 4"},
		{VK_NUMPAD5,    "NUM 5"},
		{VK_NUMPAD6,    "NUM 6"},
		{VK_NUMPAD7,    "NUM 7"},
		{VK_NUMPAD8,    "NUM 8"},
		{VK_NUMPAD9,    "NUM 9"},
		{VK_F1,         "F1"},
		{VK_F2,         "F2"},
		{VK_F3,         "F3"},
		{VK_F4,         "F4"},
		{VK_F5,         "F5"},
		{VK_F6,         "F6"},
		{VK_F7,         "F7"},
		{VK_F8,         "F8"},
		{VK_F9,         "F9"},
		{VK_F10,        "F10"},
		{VK_F11,        "F11"},
		{VK_F12,        "F12"},
		{VK_NUMLOCK,    "NL"},
		{VK_LSHIFT,     "LSHFT"},
		{VK_RSHIFT,     "RSHFT"},
		{VK_LCONTROL,   "LCTRL"},
		{VK_RCONTROL,   "RCTRL"},
		{VK_OEM_1,      ";"},
		{VK_OEM_PLUS,   "+"},
		{VK_OEM_COMMA,  "<"},
		{VK_OEM_MINUS,  "-"},
		{VK_OEM_PERIOD, ">"},
		{VK_OEM_2,      "?"},
		{VK_OEM_3,      "~"},
		{VK_OEM_4,      "["},
		{VK_OEM_5,      "|"},
		{VK_OEM_6,      "]"},
		{VK_OEM_7,      "\""},
};
