#ifndef IMAGEBTN_H
#define IMAGEBTN_H

//#include <QObject>
#include <QToolButton>

class ImageBtn : public QToolButton
{
public:
    ImageBtn(QWidget *parent = 0);
    ImageBtn(const QString &imagePath, QWidget *parent = 0);

    void setFlag();

private:
    //uint size;
};

#endif // IMAGEBTN_H
