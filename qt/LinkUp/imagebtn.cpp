#include "imagebtn.h"
#include <QPixmap>
#include <QSize>

ImageBtn::ImageBtn(QWidget *parent)
    : QToolButton(parent)
{
    //setFixedSize(80, 80);
    //setText(tr("test"));

    //size = 60;


}

ImageBtn::ImageBtn(const QString &imagePath, QWidget *parent)
    : QToolButton(parent)
{
    if(!imagePath.isNull())
    {
        this->setIcon(QPixmap(imagePath));
        QSize size(78, 78);
        this->setIconSize(size);   // 设置图片大小
        this->setFixedSize(size.width()+2, size.height()+2); // 设置按钮大小
        this->setCheckable(true);
    }
}

