#include "gamewidget.h"
#include "imagebtn.h"
#include <QIcon>
#include <QFont>
#include <QDebug>
#include <QMessageBox>

#include <ctime>

GameWidget::GameWidget(QWidget *parent)
    : QWidget(parent)
{
    QIcon icon(QString(":/images/icon.png"));
    setWindowIcon(icon);
    setFixedSize(800, 540);
    setWindowTitle(tr("Link Up Game"));
    //setWindowFlags(Qt::FramelessWindowHint);

    closeBtn = new QPushButton(tr("close"), this);
    closeBtn->setGeometry(750, 0, 50, 50);
    connect(closeBtn, SIGNAL(clicked(bool)), this, SLOT(close()));

    minBtn = new QPushButton(tr("min"), this);
    minBtn->setGeometry(700, 0, 50, 50);
    connect(minBtn, SIGNAL(clicked(bool)), this, SLOT(showMinimized()));

    iconBtn = new QPushButton(this);
    iconBtn->setGeometry(0, 0, 50, 50);
    iconBtn->setIcon(QIcon(QString(":/images/icon.png")));
    iconBtn->setIconSize(QSize(45, 45));

    startBtn = new QPushButton(tr("start"), this);
    startBtn->setGeometry(50, 0, 50, 50);
    connect(startBtn, SIGNAL(clicked(bool)), this, SLOT(startNewGame()));

    QFont font;
    //font.setBold(true);
    font.setWeight(QFont::Black);
    font.setPixelSize(36);
    titleLabel = new QLabel(tr("连连看"),this);
    titleLabel->setFont(font);
    titleLabel->setGeometry(340, 0, 140, 50);

    mapGroup = new QButtonGroup(this);
    mapGroup->setExclusive(true);
    connect(mapGroup, SIGNAL(buttonClicked(int)), this, SLOT(mapGroupClicked(int)));

    gameInit();

#if 0
    for(int i=0; i<row*column; i++) {
        qDebug()<<"imageBtn "<<i;
        ImageBtn *imageBtn = new ImageBtn(QString(":/images/element_%1.png").arg(3), this);
        imageBtn->move(imageBtn->size().width() * (i % column), imageBtn->size().height() * (i / column) + 55);
        //mapGroup->addButton(imageBtn);
    }
#endif
}

GameWidget::~GameWidget()
{
    delete closeBtn;
    delete minBtn;
    delete iconBtn;
    delete startBtn;
    delete titleLabel;

    for(int i=0; i<row*column; i++) {
        delete mapGroup->button(i);
    }
    delete mapGroup;

    delete[] map;
}

void GameWidget::gameInit()
{
    gameStatus = GAME_PRE;

    this->levelTable[PAR_LEVEL_0][0] = 6;
    this->levelTable[PAR_LEVEL_0][1] = 10;
    this->levelTable[PAR_LEVEL_1][0] = 6;
    this->levelTable[PAR_LEVEL_1][1] = 8;
    this->levelTable[PAR_LEVEL_2][0] = 6;
    this->levelTable[PAR_LEVEL_2][1] = 10;
    this->levelTable[PAR_LEVEL_3][0] = 8;
    this->levelTable[PAR_LEVEL_3][1] = 10;

    this->curLevel = PAR_LEVEL_0;
    this->row = this->levelTable[this->curLevel][0];
    this->column = this->levelTable[this->curLevel][1];

    mapStatus = SEL_NONE;
    point1.setX(0);
    point1.setY(0);
    point2.setX(0);
    point2.setY(0);
}

void GameWidget::startNewGame()
{
    startBtn->setEnabled(false);
    qDebug()<<"startNewGame";

    initMapData();
    mapStatus = SEL_NONE;

    for(int i=0; i<row*column; i++) {
        //qDebug()<<"imageBtn "<<i;
        ImageBtn *imageBtn = new ImageBtn(QString(":/images/element_%1.png").arg(map[i]), this);
        imageBtn->move(imageBtn->size().width() * (i % column), imageBtn->size().height() * (i / column) + 55);
        imageBtn->show();
        mapGroup->addButton(imageBtn);
        mapGroup->setId(imageBtn, i);
        //qDebug() << mapGroup->id(imageBtn);
    }

    //delete[] map;

}

void GameWidget::initMapData()
{
    /* 保证 map 中的元素成对【方案一】
     * 先保证 tmpMap 中的元素成对，然后从 tmpMap 中随机抽取元素到 map 中
     *
     */
    int i;
    int tmpMap[row * column];   // 中间地图，用于控制图片元素成对，以及难易程度
    map = new int[row * column]; // 与 mapGroup 相对应的地图
    srand(time(NULL));

    for(i=0; i<row * column; i=i+2) {
        // 随机生成成对的图片元素标号，存放于tmpMap中
        tmpMap[i] = tmpMap[i+1] = rand()%ELEMENT_SIZE + 1; // 随机数范围：1 ~ ELEMENT_SIZE
    }

    for(i=0; i<row * column; i++) {
        // 将 tmpMap 中的图片元素随机排列在 map 中
        int tmp = rand()%(row * column - i); // 随机数范围（逐渐缩减）：0 ~ (row * column - i)
        map[i] = tmpMap[tmp];

        if(tmp != row * column - i - 1) {
            tmpMap[tmp] = tmpMap[row * column - i - 1];
            tmpMap[row * column - i - 1] = 0;
        }
    }
}

void GameWidget::mapGroupClicked(int id)
{
    //qDebug() << "Button " << id;

    if(mapStatus == SEL_NONE) {
        point1.setX(id % column);
        point1.setY(id / column);
        mapStatus = SEL_ONE;
    }
    else if(mapStatus == SEL_ONE) {
        point2.setX(id % column);
        point2.setY(id / column);
        mapStatus = SEL_TWO;
    }
    else if(mapStatus == SEL_TWO) {
        point1.setX(id % column);
        point1.setY(id / column);
        mapStatus = SEL_ONE;
    }

    // 判断能否两个图片元素能否相连
    if(mapStatus == SEL_TWO) {
        qDebug() << "p1: x = " << point1.x() << ", y = " << point1.y();
        qDebug() << "p2: x = " << point2.x() << ", y = " << point2.y();
        if(canClear(point1, point2)) {
            map[point1.y()*column + point1.x()] = 0;
            map[point2.y()*column + point2.x()] = 0;
            //mapGroup->button(point1.y()*column + point1.x())->setIcon(QIcon(QString(":/images/element_0.png")));
            //mapGroup->button(point2.y()*column + point2.x())->setIcon(QIcon(QString(":/images/element_0.png")));
            mapGroup->button(point1.y()*column + point1.x())->hide();
            mapGroup->button(point2.y()*column + point2.x())->hide();

            if(isOver()) {
                QMessageBox msgBox;
                msgBox.setText("Congratulation! You're winner.");
                msgBox.exec();
            }
        }
    }
}

// 判断用户输入的两个点是否可以消除
bool GameWidget::canClear(QPoint& p1, QPoint& p2)
{
    if(map[p1.y()*column + p1.x()] != map[p2.y()*column + p2.x()]) {
        return false;
    }
    if(map[p1.y()*column + p1.x()] == 0) {
        return false;
    }
    // horizontal scan & vertical scan
    // 因为 row < column，所以我想先垂直扫描再水平扫描
    if(verticalScan(p1, p2) || horizontalScan(p1, p2)) {
        return true;
    }
    return false;
}

// 水平扫描
bool GameWidget::horizontalScan(QPoint p1, QPoint p2)
{
    /* 水平标定
     *
     * 以 p1 和 p2 所在的两个行作为基准线，然后用垂直线在有效范围内扫描，
     * 一旦发现可行路径，直接返回。
     *
     */
    int hLine1, hLine2;			// 上边线和下边线
    int leftLimit, rightLimit;	// 左边线和右边线
    int i, j;

    // 如果 p1 和 p2 在同一行，则不符合要求
    if(p1.y() == p2.y())
        return false;

    qDebug() << "Horizontal Scanning ...";

    // 确保 p1 在 p2 的左边，或在同一条垂直线上（非必要条件）
#if 0
    if(p1.x > p2.x) {
        point pt = p1;
        p1 = p2;
        p2 = pt;
    }
#endif

    // 记录两条水平基准线
    hLine1 = p1.y() < p2.y() ? p1.y() : p2.y(); // 上水平线
    hLine2 = p1.y() > p2.y() ? p1.y() : p2.y(); // 下水平线

    // 初始化左、右边界线为 0
    leftLimit = 0;
    rightLimit = column-1;

    // 寻找左边界线
    i = p1.x();
    // 第一次扫描
    while(i) {	// 当 i 大于 0 时，才进入循环
        if(map[p1.y() * column + i - 1] != 0) break; // 判断左边点是否为空
        i--; // 当左边点为空时会继续扫面下一个左边点
    }
    leftLimit = i;

    // 第二次扫描
    i = p2.x();
    while(i) {
        if(map[p2.y() * column + i - 1] != 0) break;
        i--;
    }

    // leftLimit 记录左边界线，该界线所在的点为空或p1、p2本身
    if(i > leftLimit) {
        leftLimit = i;
    }

    // 如果 leftLimit 为 0，说明p1、p2已经在外界接通了，直接返回
    if(leftLimit == 0) {
        return true;
    }

    // 寻找右边界线
    i = p1.x();
    while(i < column-1) {
        if(map[p1.y() * column + i + 1] != 0) break;
        i++;
    }
    rightLimit = i;

    i = p2.x();
    while(i < column-1) {
        if(map[p2.y() * column + i + 1] != 0) break;
        i++;
    }

    if(i < rightLimit) {
        rightLimit = i;
    }

    if(rightLimit == column-1) {
        return true;    // Bug
    }

    // 判断 leftLimit 和 rightLimit
    if(leftLimit > rightLimit) {
        return false;	// 如果左边界线超出右边界线，则无法连接
    }
    else { // 从左往右扫描
        for(i=leftLimit; i<=rightLimit; i++) {
            for(j=hLine1+1; j<hLine2; j++) {
                if(map[j * column + i] != 0) break; // 只要当前列有阻碍，马上跳出
            }
            if(j == hLine2) {
                return true;
            }
        }
        return false;
    }
}

// 垂直扫描
bool GameWidget::verticalScan(QPoint p1, QPoint p2)
{
    /* 垂直标定
     *
     * 以 p1 和 p2 所在的两个列作为基准线，然后用水平线在有效范围内扫描，
     * 一旦发现可行路径，直接返回。
     *
     */
    int vLine1, vLine2;
    int topLimit, bottomLimit;
    int i, j;

    // 如果 p1 和 p2 在同一列，则不符合要求
    if(p1.x() == p2.x())
        return false;

    qDebug() << "Vertical Scanning ...";

    // 确保 p1 在 p2 的上边，或在同一条水平线上（非必要条件）
#if 0
    if(p1.y > p2.y) {
        point pt = p1;
        p1 = p2;
        p2 = pt;
    }
#endif

    // 记录两条垂直基准线
    vLine1 = p1.x() < p2.x() ? p1.x() : p2.x(); // 左垂直线
    vLine2 = p1.x() > p2.x() ? p1.x() : p2.x(); // 右垂直线

    // 初始化上、下边界线
    topLimit = 0;
    bottomLimit = row-1;

    // 寻找上边界线
    i = p1.y();
    // 第一次扫描
    while(i) {	// 当 i 大于 0 时，才进入循环
        if(map[p1.x() + (i-1) * column] != 0) break; // 判断上边点是否为空
        i--; // 当上边点为空时会继续扫面下一个上边点
    }
    topLimit = i;

    // 第二次扫描
    i = p2.y();
    while(i) {
        if(map[p2.x() + (i-1) * column] != 0) break;
        i--;
    }

    // topLimit 记录上边界线，该界线所在的点为空或p1、p2本身
    if(i > topLimit) {
        topLimit = i;
    }

    // 如果 topLimit 为 0，说明p1、p2已经在外界接通了，直接返回
    if(topLimit == 0) {
        return true;
    }

    // 寻找下边界线
    i = p1.y();
    while(i < row-1) {
        if(map[p1.x() + (i+1) * column] != 0) break;
        i++;
    }
    bottomLimit = i;

    i = p2.y();
    while(i < row-1) {
        if(map[p2.x() + (i+1) * column] != 0) break;
        i++;
    }

    if(i < bottomLimit) {
        bottomLimit = i;
    }

    if(bottomLimit == row-1) {
        return true;
    }

    // 判断 topLimit 和 bottomLimit
    if(topLimit > bottomLimit) {
        return false;	// 如果上边界线超出下边界线，则无法连接
    }
    else { // 从上往下扫描
        for(i=topLimit; i<=bottomLimit; i++) {
            for(j=vLine1+1; j<vLine2; j++) {
                if(map[i * column + j] != 0) break; // 只要当前列有阻碍，马上跳出
            }
            if(j == vLine2) {
                return true;
            }
        }
        return false;
    }
}

// 判断是否全部消完
bool GameWidget::isOver()
{
    for(int i=0; i<row*column; i++) {
        if(map[i] != 0)
            return false;
    }
    this->gameStatus = GAME_OVER;
    return true;
}
