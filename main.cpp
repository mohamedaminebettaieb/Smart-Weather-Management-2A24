#include <QApplication>
#include <QMessageBox>
#include "smartweatherwindow.h"
#include "connection.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName("SmartWeather");
    app.setApplicationDisplayName("SmartWeather");
    Connection& connection = Connection::createInstance();
    if (!connection.createConnection())
    {
        QMessageBox::critical(nullptr, "Erreur de connexion", "Impossible de se connecter à la base de données.\n", "Vérifiez la source ODBC, le nom d'utilisateur et le mot de passe.");
    return -1;
    }
    QMessageBox::information(nullptr, "Connexion", "Connexion à la base de données réussie.");


    SmartWeatherWindow window;
    window.show();

    return app.exec();
}
