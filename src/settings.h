#ifndef SETTINGS_H
#define SETTINGS_H

#include <QtGui/QDialog>
#include <QTime>
#include <QCloseEvent>

#include "defines.h"

class QCheckBox;

namespace Ui {
    class Settings;
}

class Settings : public QDialog {
    Q_OBJECT
    Q_DISABLE_COPY(Settings)
public:
    explicit Settings(QWidget *parent = 0);
    virtual ~Settings();

public slots:
    void loadSettings(SettingsData &settings);

protected:
    virtual void changeEvent(QEvent *e);

private:
    Ui::Settings *m_ui;
    void closeEvent(QCloseEvent *event);

    void setCheckBoxInt(qint16 value, QCheckBox *box);
    qint16 getCheckBoxInt(QCheckBox *box);

private slots:
    void saveSettings(void);
    void enableAutostart(bool);

signals:
    void settingsChanged(SettingsData settings);
};

#endif // SETTINGS_H
