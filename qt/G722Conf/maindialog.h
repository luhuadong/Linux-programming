#ifndef MAINDIALOG_H
#define MAINDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QCheckBox>
#include <QPushButton>
#include <QSettings>
#include <QString>



class MainDialog : public QDialog
{
    Q_OBJECT

public:
    MainDialog(QWidget *parent = 0);
    ~MainDialog();

private slots:
    void applyBtnClicked(void);

private:
    QLabel *G722Label;
    QCheckBox *G722EnBox;
    QPushButton *applyBtn;

    QSettings *configIniObj;
    int G722EnValue;
};

#endif // MAINDIALOG_H
