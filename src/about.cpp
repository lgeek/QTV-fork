#include "about.h"
#include "ui_about.h"

#include <QDesktopServices>
#include <QUrl>

About::About(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::About)
{
    QPixmap icon(":/images/icon");
    m_ui->setupUi(this);

    icon = icon.scaledToHeight(icon.height()/3);
    m_ui->labelIcon->setPixmap(icon);
    m_ui->labelApplication->setText("<h1>"+qApp->applicationName()+" "+qApp->applicationVersion()+"</h1>");
    m_ui->labelOrganisation->setText("<h3><a href=\""+qApp->organizationDomain()+"\">"+qApp->organizationDomain()+"</a></h3>");
    connect(m_ui->labelOrganisation, SIGNAL(linkActivated(QString)), this, SLOT(openUrl(QString)));
    m_ui->label->setText("\n"+tr("Created by")+":\n\nHerndl Martin <martin.herndl@gmail.com>\nHorn Matthias <matthias.g.horn@gmail.com>");
    m_ui->labelFetch->setText(tr("Data is fetched from: ")+"<a href=\"http://next-episode.net/\">http://next-episode.net/</a>");
    connect(m_ui->labelFetch, SIGNAL(linkActivated(QString)), this, SLOT(openUrl(QString)));

    connect(m_ui->buttonBox, SIGNAL(accepted()), this, SLOT(hide()));
}

About::~About()
{
    delete m_ui;
}

void About::openUrl(QString url)
{
        QDesktopServices::openUrl(QUrl(url));
}

void About::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
