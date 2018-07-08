#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QToolButton>
#include <QButtonGroup>
#include <QPoint>
#include <QLabel>
//#include "imagebtn.h"
#include "linkup_common.h"

class GameWidget : public QWidget
{
    Q_OBJECT

public:
    GameWidget(QWidget *parent = 0);
    ~GameWidget();

private slots:
    void startNewGame();
    void mapGroupClicked(int id);

private:
    GAME_STATUS gameStatus;	// 游戏状态
    PAR_LEVEL_T curLevel;	// 游戏等级
    SEL_STATUS  mapStatus;  // 用于确定选中两个图片元素的动作是否完成
    int levelTable[PAR_LEVEL_END][2];
    int *map;	// 游戏区域
    int row;	// 记录当前游戏行数
    int column;	// 记录当前游戏列数
    QPoint point1;	// 记录用户输入的第一个点
    QPoint point2;	// 记录用户输入的第二个点

    QPushButton *closeBtn;
    QPushButton *minBtn;
    QPushButton *iconBtn;
    QPushButton *startBtn;
    QPushButton *helpBtn;
    QLabel *titleLabel;


    QButtonGroup *mapGroup;

    void gameInit();
    void initMapData();

    bool canClear(QPoint& p1, QPoint& p2);				// 判断用户输入的两个点是否可以消除
    bool horizontalScan(QPoint p1, QPoint p2);			// 水平扫描
    bool verticalScan(QPoint p1, QPoint p2);			// 垂直扫描
    bool isOver();										// 判断是否全部消完

};

#endif // GAMEWIDGET_H
