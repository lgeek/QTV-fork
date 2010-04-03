#ifndef MSGBOX_H
#define MSGBOX_H

#include <QtGui/QDialog>

class QAbstractButton;
class QCloseEvent;

namespace Ui {
    class MsgBox;
}

class MsgBox : public  QDialog {
    Q_OBJECT
public:
    MsgBox(QWidget *parent = 0);
    ~MsgBox();
    void setInformativeText(const QString text);
    void setText(const QString text);
    void showMessage(const QString title, const QString text);

signals:
    void messageClicked(void);

protected:
    void changeEvent(QEvent *e);
    void closeEvent(QCloseEvent *event);

private:
    Ui::MsgBox *m_ui;
};

#endif // MSGBOX_H
