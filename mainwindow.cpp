#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QDoubleValidator>
#include <QFontDatabase>

HHOOK globalKeyboardCaptureHook = nullptr;
static MainWindow *mainWindow = nullptr;

// =============================================================================================== HookCallback
// 全局键盘捕获回调函数
LRESULT CALLBACK globalKeyboardCaptureCallback(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode >= 0 && wParam == WM_KEYDOWN)
	{
		// 检测按键按下事件
		auto *pKeyBoard = (KBDLLHOOKSTRUCT *) lParam;
		DWORD keyCode = pKeyBoard->vkCode;

		if (mainWindow)
		{
			// 确保在mainWindow中调用槽函数
			QMetaObject::invokeMethod(mainWindow, "obtainedKey", Qt::QueuedConnection,
			                          Q_ARG(DWORD, keyCode));
		}
	}
	return CallNextHookEx(globalKeyboardCaptureHook, nCode, wParam, lParam);
}

// =============================================================================================== Class
MainWindow::MainWindow(QWidget *parent) :
		CustomWidget(parent),
		primaryWidgetColor("#21252b"),
		secondaryWidgetColor("#282c34"),
		ui(new Ui::MainWindow),
		soundEffectPlayer(new QMediaPlayer(this)),
		soundEffectAudioOutput(new QAudioOutput(this)),
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
		startEventInjectorAllowed_flag(true),
		pageNum(IOCONFIGPAGE),
		inputKey(MOUSELEFTKEY),
		inputActionMode(CLICKS),
		cursorMoveMode(FREE),
		globalSwitchKey(VK_F8),
		diyKey(0x00),
		coordinateXY({0, 0})
{
	ui->setupUi(this);

	// 事件注入器线程初始化
	eventInjectorThread->start();
	eventInjector->moveToThread(eventInjectorThread);

	// 播放器初始化
	soundEffectPlayer->setAudioOutput(soundEffectAudioOutput);
	soundEffectAudioOutput->setVolume(100);

	// 设置全局按键捕获钩子
	mainWindow = this;
	globalKeyboardCaptureHook = SetWindowsHookEx(WH_KEYBOARD_LL, globalKeyboardCaptureCallback, nullptr, 0);

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

	connect(ui->globalSwitchCaptureButton, &QPushButton::toggled, this, &MainWindow::getGlobalSwitchKey);
	connect(ui->cursorCoordinateCaptureButton, &QPushButton::toggled, this, &MainWindow::startCoordinateCapture);

	connect(ui->mouseLeftButton, &QPushButton::toggled, this, [=](bool status)
	{
		if (status)
		{
			inputKey = MOUSELEFTKEY;
		}
	});
	connect(ui->mouseMiddleButton, &QPushButton::toggled, this, [=](bool status)
	{
		if (status)
		{
			inputKey = MOUSEMIDDLEKEY;
		}
	});
	connect(ui->mouseRightButton, &QPushButton::toggled, this, [=](bool status)
	{
		if (status)
		{
			inputKey = MOUSERIGHTKEY;
		}
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

	connect(ui->clicksButton, &QPushButton::toggled, this, [=](bool status)
	{
		if (status)
		{
			inputActionMode = CLICKS;
		}
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

	connect(ui->cursorFreeButton, &QPushButton::toggled, this, [=](bool status)
	{

		if (status)
		{
			cursorMoveMode = FREE;
			ui->IOConfig_bar_2->setEnabled(false);
		}
	});
	connect(ui->cursorLockButton, &QPushButton::toggled, this, [=](bool status)
	{
		if (status)
		{
			cursorMoveMode = LOCK;
			ui->IOConfig_bar_2->setEnabled(true);
		}
	});

	connect(ui->PeriodValueInputLineEdit, &QLineEdit::textChanged, this, [=](const QString &text)
	{
		if (text.isEmpty() || text.toDouble() == 0)
		{
			ui->startLabel->setStyleSheet("border: none; border-radius: 8px; background-color: #1b397e; color: #7e7e7e;");  // 禁用
			startEventInjectorAllowed_flag = false;
		}
		else
		{
			ui->startLabel->setStyleSheet("border: none; border-radius: 8px; background-color: #3686f2; color: white;");  // 空闲
			startEventInjectorAllowed_flag = true;
		}
	});

	connect(this, &MainWindow::startEventInjector, eventInjector, &EventInjector::startTimer);

	// 设置周期值输入框的输入字符类型和数值范围
	auto *doubleValidator = new QDoubleValidator(0.000, 99999.999, 3, ui->PeriodValueInputLineEdit);
	doubleValidator->setNotation(QDoubleValidator::StandardNotation); // 设置QDoubleValidator为标准计数法 ( 默认为科学计数法 )
	ui->PeriodValueInputLineEdit->setValidator(doubleValidator);

	// 设置字体
	int fontId = QFontDatabase::addApplicationFont(":/resources/CascadiaMono.ttf");
	QString fontFamily = QFontDatabase::applicationFontFamilies(fontId).at(0);
	QFont font(fontFamily);
	font.setPointSize(10);
	ui->titleLabel->setFont(font);
	ui->appName_label_A->setFont(font);
	ui->appName_label_B->setFont(font);

	// 设置介绍页超链接
	ui->about_label_4->setText(
			"<a style='color: #ffada9; text-decoration: none;' href=\"https://github.com/flowersauce/FlowersauceClicker\">点击跳转到仓库</a>");
	ui->about_label_4->setOpenExternalLinks(true);  // 允许外部链接打开

	// 页面控件初始化
	ui->about_widget->setVisible(false);
	ui->IOConfig_bar_2->setEnabled(false);
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
		ui->startLabel->setStyleSheet("border: none; border-radius: 8px; background-color: #1b397e; color: #7e7e7e;");  // 禁用

		ui->IOConfig_bar_1->setEnabled(false);
		ui->IOConfig_bar_3->setEnabled(false);
		ui->globalSwitchCaptureButton->clearFocus();
		ui->globalSwitchCaptureButton->setText("捕获中");

		getGlobalSwitchKeyHook_flag = true;
	}
	else
	{
		ui->startLabel->setStyleSheet("border: none; border-radius: 8px; background-color: #3686f2; color: white;");  // 空闲

		ui->IOConfig_bar_1->setEnabled(true);
		ui->IOConfig_bar_3->setEnabled(true);
	}
}

void MainWindow::getDIYKey()
{
	ui->DIYKeyButton->clearFocus();
	ui->IOConfig_bar_1->setEnabled(false);
	ui->IOConfig_bar_2->setEnabled(false);
	ui->IOConfig_bar_3->setEnabled(false);
	ui->IOConfig_bar_5->setEnabled(false);
	ui->startLabel->setStyleSheet("border: none; border-radius: 8px; background-color: #1b397e; color: #7e7e7e;");  // 禁用

	inputKey = DIYKEY;
	ui->DIYKeyButton->setText("捕获中");

	getDIYKeyHook_flag = true;
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
		}
	}
	else if (getDIYKeyHook_flag)
	{
		if (theKeyValue != keyMap.end() && keyCode != globalSwitchKey)
		{
			ui->DIYKeyButton->setText(QString::fromStdString(theKeyValue->second));
			diyKey = keyCode;
			getDIYKeyHook_flag = false;
			ui->startLabel->setStyleSheet("border: none; border-radius: 8px; background-color: #3686f2; color: white;");  // 空闲

			ui->IOConfig_bar_1->setEnabled(true);
			ui->IOConfig_bar_3->setEnabled(true);
		}
		else
		{
			ui->DIYKeyButton->setText("请重试");
		}
	}
	else if (startEventInjectorAllowed_flag)
	{
		if (keyCode == globalSwitchKey)
		{
			if (!eventInjector->eventInjector_flag)
			{
				// 播放音频
				soundEffectPlayer->setSource(QUrl("qrc:/resources/open.wav"));
				soundEffectPlayer->play();
				// 修改控件状态
				ui->IOConfigBars_widget->setEnabled(false);
				ui->startLabel->setStyleSheet("border: none; border-radius: 8px; background-color: #ff5f56; color: white;");  // 工作中
				ui->startLabel->setText("按下 -全局开关- 终止");
				// 启动事件注入
				emit startEventInjector(inputKey, inputActionMode, cursorMoveMode, diyKey, coordinateXY.at(0),
										coordinateXY.at(1),
										ui->PeriodValueInputLineEdit->text().toDouble());
				while(!eventInjector->eventInjector_flag)
				{
					QThread::usleep(1000);
				}
			}
			else
			{
				// 播放音频
				soundEffectPlayer->setSource(QUrl("qrc:/resources/close.wav"));
				soundEffectPlayer->play();
				// 修改控件状态
				ui->IOConfigBars_widget->setEnabled(true);
				ui->startLabel->setStyleSheet("border: none; border-radius: 8px; background-color: #3686f2; color: white;");  // 空闲
				ui->startLabel->setText("按下 -全局开关- 启动");
				// 修改循环标志停止事件注入
				eventInjector->eventInjector_flag = false;

				while(eventInjector->eventInjector_flag)
				{
					QThread::usleep(1000);
				}
			}
		}
	}
}

void MainWindow::startCoordinateCapture(bool status)
{
	if (status)
	{
		// 禁止启动事件注入器
		startEventInjectorAllowed_flag = false;
		ui->startLabel->setStyleSheet("border: none; border-radius: 8px; background-color: #1b397e; color: #7e7e7e;");  // 禁用
		// 控件设置
		ui->cursorCoordinateCaptureButton->clearFocus();
		ui->cursorCoordinateCaptureButton->setText("捕获中");
		// 动态创建窗口
		auto *captureWindow = new CoordinateCaptureWindow(this);
		// 自动删除窗口对象
		captureWindow->setAttribute(Qt::WA_DeleteOnClose);
		connect(captureWindow, &CoordinateCaptureWindow::coordinatesCaptured, this, &MainWindow::getCursorCoordinate);
		captureWindow->show();
	}
}

void MainWindow::getCursorCoordinate(int x, int y)
{
	coordinateXY.at(0) = x;
	coordinateXY.at(1) = y;
	auto coordinateXYStr = QString::number(x) + "," + QString::number(y);
	ui->cursorCoordinateCaptureButton->setText(coordinateXYStr);
	ui->cursorCoordinateCaptureButton->setChecked(false);
	// 允许启动事件注入器
	startEventInjectorAllowed_flag = true;
	ui->startLabel->setStyleSheet("border: none; border-radius: 8px; background-color: #3686f2; color: white;");  // 空闲
}
