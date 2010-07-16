/*
    Copyright (c) 2009, Lukas Holecek <hluk@email.cz>

    This file is part of CopyQ.

    CopyQ is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    CopyQ is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with CopyQ.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <qtsingleapplication.h> 
#include "mainwindow.h"
#include <QSettings>
#include <QDebug>

inline bool readCssFile(QIODevice &device, QSettings::SettingsMap &map)
{
    map.insert( "css", device.readAll() );
    return true;
}

inline bool writeCssFile(QIODevice &, const QSettings::SettingsMap &)
{
    return true;
}

QtSingleApplication *newClient() {
    int argc2 = 0;
    QtSingleApplication *client =
            new QtSingleApplication( QString("CopyQclient"), argc2, (char**)NULL );
    return client;
}

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(copyq);
    QCoreApplication::setOrganizationName("copyq");
    QCoreApplication::setApplicationName("copyq");

    // TODO: different session id for other users
    QtSingleApplication app( QString("CopyQ"), argc, argv );

    QString msg;

    msg = argc > 1 ? QString(argv[1]) : QString("toggle");
    // append escaped arguments to message
    for (int i = 2; i < argc; ++i) {
        msg.append( QString('\n') +
                    QString::fromLocal8Bit(argv[i]).replace(
                            QChar('\\'), QString("\\\\")).replace(
                            QChar('\n'), QString(" \\n")));
    }

    if ( app.isRunning() ) {
        QtSingleApplication *client = newClient();

        // if another client running -- wait
        while ( client->isRunning() ) {
            delete client;
#if defined(Q_OS_WIN)
            Sleep(DWORD(1000));
#else
            const struct timespec ts = { 1, 0 };
            nanosleep(&ts, NULL);
#endif
            client = newClient();
        }

        Client obj;
        QObject::connect(
                client, SIGNAL(messageReceived(const QString&)),
                &obj, SLOT(handleMessage(const QString&)) );

        // try to send a message if application already running
        // -1 means wait forever for app to respond (if instance found)
        app.sendMessage(client->id() + QString('\n') + msg, -1);

        return client->exec();
    }

    // style
    QSettings::Format cssFormat = QSettings::registerFormat(
            "css", readCssFile, writeCssFile);
    QSettings cssSettings( cssFormat, QSettings::UserScope,
            QCoreApplication::organizationName(),
            QCoreApplication::applicationName() );
    QString css = cssSettings.value("css", "").toString();

    MainWindow wnd(css);

    QObject::connect(&app, SIGNAL(messageReceived(const QString&)),
            &wnd, SLOT(handleMessage(const QString&)));

    return app.exec();
}
