#include "aboutbox.h"

/**
 * Constructor. Receives a pointer to the QStyle of the parent form.
 */
AboutBox::AboutBox(QStyle *parentStyle, Comm14CUXVersion cuxVersion) :
    urlString(QString("http://code.google.com/p/rovergauge/")),
    aboutString(QString("A graphical interface to the 14CUX engine management system.\n\nUsing comm14cux "))
{
    ver.major = cuxVersion.major;
    ver.minor = cuxVersion.minor;
    ver.patch = cuxVersion.patch;
    style = parentStyle;
    setupWidgets();
}

/**
 * Builds a dot-separated string with the version number.
 * @maj Major version number
 * @min Minor version number
 * @patch Patch version number
 * @return Dot-separated version string
 */
QString AboutBox::makeVersionString(int maj, int min, int patch)
{
    return (QString::number(maj) + "." + QString::number(min) + "." + QString::number(patch));
}

/**
 * Creates widgets and places them on the form.
 */
void AboutBox::setupWidgets()
{
    grid = new QGridLayout();
    this->setLayout(grid);

    iconLabel = new QLabel(this);
    iconLabel->setPixmap(style->standardIcon(QStyle::SP_MessageBoxInformation).pixmap(32, 32));

    name = new QLabel("RoverGauge " +
                      makeVersionString(ROVERGAUGE_VER_MAJOR, ROVERGAUGE_VER_MINOR, ROVERGAUGE_VER_PATCH),
                      this);
    QFont defaultFont = name->font();
    defaultFont.setPointSize(14);
    name->setFont(defaultFont);

    info = new QLabel(aboutString + makeVersionString(ver.major, ver.minor, ver.patch) + ".", this);

    url = new QLabel("<a href=\"" + urlString + "\">" + urlString + "</a>", this);
    url->setOpenExternalLinks(true);

    ok = new QPushButton("Close", this);
    connect(ok, SIGNAL(clicked()), SLOT(accept()));

    grid->addWidget(iconLabel, 0, 0, 1, 1, Qt::AlignCenter);
    grid->addWidget(name, 0, 1, 1, 1, Qt::AlignLeft);
    grid->addWidget(info, 1, 1, 1, 1, Qt::AlignLeft);
    grid->addWidget(url, 2, 1, 1, 1, Qt::AlignLeft);
    grid->addWidget(ok, 3, 1, 1, 1, Qt::AlignRight);
}

