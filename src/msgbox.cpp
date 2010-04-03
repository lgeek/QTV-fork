#include "msgbox.h"
#include "ui_msgbox.h"

#include <QAbstractButton>
#include <QCloseEvent>
#include <QDesktopWidget>

MsgBox::MsgBox(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::MsgBox)
{
    m_ui->setupUi(this);

    this->setWindowFlags(Qt::WindowStaysOnTopHint);
    connect(m_ui->buttonOk, SIGNAL(clicked()), this, SIGNAL(messageClicked()));
}

MsgBox::~MsgBox()
{
    delete m_ui;
}

void MsgBox::setInformativeText(const QString text)
{
    m_ui->label->setText(text);
    this->resize(this->minimumSizeHint());
}

void MsgBox::setText(const QString text)
{
    this->setWindowTitle(text);
}

void MsgBox::showMessage(const QString title, const QString text)
{
    // get the current screen size (screen 0)
    qint16 width = QApplication::desktop()->screenGeometry(0).width();
    qint16 height = QApplication::desktop()->screenGeometry(0).height();
    // set the window according to it (right in the middle of the screen :)
    this->move((width/2) - (this->width()/2), (height/2) - (this->height()/2));

    this->setWindowTitle(title);
    m_ui->label->setText(text);
    this->show();
}

void MsgBox::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MsgBox::closeEvent(QCloseEvent *event)
{
    event->ignore();
    this->hide();
}
