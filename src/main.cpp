#include <QApplication>
#include <QMessageBox>
#include <QDir>
#include <QDebug>
#include "mainwindow.h"
#include "database.h"
#include "config.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    app.setApplicationName(APP_NAME);
    app.setOrganizationName("AlexFinance");
    app.setApplicationVersion(APP_VERSION);

    if (!Database::instance().initialize()) {
        QMessageBox::critical(nullptr, "Ошибка", 
            "Не удалось инициализировать базу данных.\n"
            "Проверьте права доступа к папке приложения.");
        return 1;
    }
    
    MainWindow window;
    window.show();
    
    int result = app.exec();
    
    Database::instance().close();
    
    return result;
}
