#include "widget.h"
#include <QImageReader>
#include <QDir>
#include <QKeyEvent>

#define DIR_PATH_PREFIX "/opt/images/"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    /* init image resources */
    QString dirPath(DIR_PATH_PREFIX);
    QDir dir(dirPath);

    QStringList nameFilters;
    nameFilters << "*.png";

    imageList = dir.entryList(nameFilters, QDir::Files|QDir::Readable, QDir::Name);

    qDebug() << "size: " << imageList.size();
    qDebug() << imageList;

    /* init show area */
    imageLabel = new QLabel(this);

    ind = 0;
    QImageReader reader(DIR_PATH_PREFIX + imageList.at(ind++));
    reader.setAutoTransform(true);
    const QImage newImage = reader.read();

    setImage(newImage);

    this->showFullScreen();

    /* default timer */
    timer = new QTimer(this);
    //connect(timer, &QTimer::timeout, this, SLOT(updateImage()));
    connect(timer, SIGNAL(timeout()), this, SLOT(updateImage()));
    //connect(timer, &QTimer::timeout, this, QOverload<>::of(updateImage()));
    timer->start(3700);
}

Widget::~Widget()
{

}

void Widget::setImage(const QImage &newImage)
{
    image = newImage;
    imageLabel->setPixmap(QPixmap::fromImage(image));
    imageLabel->adjustSize();
}

void Widget::updateImage(void)
{
    ind++;

    if (ind >= imageList.size())
        ind = 0;

    //qDebug() << "timeout " << ind;

    QImageReader reader(DIR_PATH_PREFIX + imageList.at(ind));
    reader.setAutoTransform(true);
    const QImage newImage = reader.read();

    setImage(newImage);
}

void Widget::keyPressEvent(QKeyEvent *event)
{
    timer->stop();

    switch(event->key()){
        case Qt::Key_Escape:
            qDebug() << "Key_Escape Press";
            break;
        case Qt::Key_Tab:
            qDebug() << "Key_Tab Press";
            break;
        case Qt::Key_Enter:
            qDebug() << "Key_Enter Press";
            timer->start();
            break;
        case Qt::Key_Delete:
            qDebug() << "Key_Delete Press";
            break;
        case Qt::Key_Space:
            static int pauseFlag = 0;
            if (pauseFlag == 0)
            {
                timer->stop();
                pauseFlag = 1;
                qDebug() << "Key_Space Press, Pause";
            }
            else if (pauseFlag == 1)
            {
                updateImage();
                timer->start();
                pauseFlag = 0;
                qDebug() << "Key_Space Press, Resume";
            }
            break;
        case Qt::Key_Left:
            prevPage();
            qDebug() << "Key_Left Press";
            break;
        case Qt::Key_Up:
            prevPage();
            qDebug() << "Key_Up Press";
            break;
        case Qt::Key_Right:
            nextPage();
            qDebug() << "Key_Right Press";
            break;
        case Qt::Key_Down:
            nextPage();
            qDebug() << "Key_Down Press";
            break;
        default:
            break;
    }
}

void Widget::keyReleaseEvent(QKeyEvent *event)
{

}

void Widget::pauseUpdate(void)
{

}

void Widget::prevPage(void)
{
    ind--;

    if (ind < 0)
        ind = imageList.size() - 1;

    //qDebug() << "timeout " << ind;

    QImageReader reader(DIR_PATH_PREFIX + imageList.at(ind));
    reader.setAutoTransform(true);
    const QImage newImage = reader.read();

    setImage(newImage);
}

void Widget::nextPage(void)
{
    updateImage();
}
