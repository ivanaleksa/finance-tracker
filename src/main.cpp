#include <QApplication>
#include <QMessageBox>
#include <QDir>
#include <QDebug>
#include <QPalette>
#include "mainwindow.h"
#include "database.h"
#include "config.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // set cumpolsury light theme
    QPalette lightPalette;
    lightPalette.setColor(QPalette::Window, QColor(240, 240, 240));
    lightPalette.setColor(QPalette::WindowText, QColor(0, 0, 0));
    lightPalette.setColor(QPalette::Base, QColor(255, 255, 255));
    lightPalette.setColor(QPalette::AlternateBase, QColor(245, 245, 245));
    lightPalette.setColor(QPalette::ToolTipBase, QColor(255, 255, 220));
    lightPalette.setColor(QPalette::ToolTipText, QColor(0, 0, 0));
    lightPalette.setColor(QPalette::Text, QColor(0, 0, 0));
    lightPalette.setColor(QPalette::Button, QColor(240, 240, 240));
    lightPalette.setColor(QPalette::ButtonText, QColor(0, 0, 0));
    lightPalette.setColor(QPalette::BrightText, QColor(255, 0, 0));
    lightPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    lightPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    lightPalette.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
    app.setPalette(lightPalette);
    
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
