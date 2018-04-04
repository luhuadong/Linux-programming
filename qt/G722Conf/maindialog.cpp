#include "maindialog.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSettings>
#include <QFont>
#include <QIcon>

const char *ini_path =  "/usr/seat/ata0a/Ini/MediaInfo.ini";

MainDialog::MainDialog(QWidget *parent)
    : QDialog(parent)
{
    this->setWindowTitle(tr("G722"));
    this->setWindowIcon(QIcon(":/pictures/G722_icon.png"));

    G722Label = new QLabel(tr("G722En :    "), this);
    QFont font;
    font.setPixelSize(24);
    font.setBold(true);
    G722Label->setFont(font);

    G722EnBox = new QCheckBox(this);
    G722EnBox->setCheckable(true);
    //G722EnBox->setFixedSize(50, 50);

    applyBtn = new QPushButton(tr("Apply"), this);
    applyBtn->setFixedSize(200, 60);

    QHBoxLayout *boxLayout = new QHBoxLayout;
    boxLayout->setSpacing(5);
    boxLayout->setMargin(10);
    boxLayout->addWidget(G722Label);
    boxLayout->addWidget(G722EnBox);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setSpacing(20);
    mainLayout->addLayout(boxLayout);
    mainLayout->addWidget(applyBtn);

    this->setLayout(mainLayout);

    connect(applyBtn, SIGNAL(clicked()), this, SLOT(applyBtnClicked()));

    configIniObj = new QSettings(QString(ini_path), QSettings::IniFormat);
    G722EnValue = configIniObj->value("/System/G722En", 0).toInt();

    if(G722EnValue == 0) {
        G722EnBox->setChecked(false);
    }
    else if(G722EnValue == 1) {
        G722EnBox->setChecked(true);
    }
    else {
        this->close();
    }

}

MainDialog::~MainDialog()
{
    delete configIniObj;
    delete G722Label;
    delete G722EnBox;
    delete applyBtn;

}


/*
 * root@imx6qsabresd:~# cat /usr/seat/ata0a/Ini/MediaInfo.ini
 * [System]
 *   MediaNum=1
 *   VideoMediaNum=2
 *   G722En=0
 * [Media_1]
 *   MediaType=1
 *   Payload=8
 * [VideoMedia_1]
 *   MediaType=1
 *   Payload=8
 * [VideoMedia_2]
 *   MediaType=129
 *
 * */
void MainDialog::applyBtnClicked()
{
    if(G722EnBox->isChecked()) {
        G722EnValue = 1;
    }
    else {
        G722EnValue = 0;
    }

    configIniObj->setValue("/System/G722En", G722EnValue);
	configIniObj->sync();

    /*
     * /opt/sdsp/lnxconf
     *
     * */
    system("killall lnxconf");
    system("sleep 2");
    system("/opt/sdsp/lnxconf &");
    this->close();
}
