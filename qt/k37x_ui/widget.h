#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QImage>
#include <QLabel>
#include <QString>
#include <QStringList>
#include <QTimer>

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    void setImage(const QString url);
    void setImage(const QImage &newImage);

private slots:
    void updateImage(void);
    void pauseUpdate(void);
    void prevPage(void);
    void nextPage(void);

    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

private:
    //QString *dirPath;
    QStringList imageList;
    int ind;

    QImage image;
    QLabel *imageLabel;

    QTimer *timer;

};
#endif // WIDGET_H
