SMART WEATHER MANAGEMENT — VERSION INTEGREE
==========================================

Contenu
-------
- Gestion des techniciens
- Gestion des stations
- Gestion des équipements
- Gestion des bureaux
- Gestion des utilisateurs (avec écran de connexion)
- Connexion Oracle via Qt SQL / QODBC
- Fichiers CMake et qmake (.pro)

Ouverture avec Qt Creator
-------------------------
1. Extraire entièrement le ZIP dans un dossier.
2. Ouvrir SmartWeather.pro (ou CMakeLists.txt) avec Qt Creator.
3. Sélectionner un kit Qt 6 compatible et un compilateur C++17.
4. Vérifier que le pilote Qt SQL QODBC est installé.
5. Vérifier que la source ODBC Oracle configurée sur le PC correspond à celle
   utilisée dans connection.cpp. Vérifier également le nom d'utilisateur et
   le mot de passe Oracle dans ce fichier avant l'exécution.
6. Exécuter base_oracle.sql uniquement si le schéma n'est pas déjà créé.
   Ne pas réexécuter le script sur une base contenant déjà les tables sans
   sauvegarde préalable.

Compilation avec CMake
----------------------
cmake -S . -B build
cmake --build build

Important
---------
Le projet a été inspecté et son organisation CMake/qmake vérifiée. La
compilation et la connexion Oracle ne peuvent pas être certifiées dans cet
environnement car le SDK Qt 6 et le pilote Oracle ODBC ne sont pas installés.
La réussite dépend aussi de la configuration Oracle/ODBC de votre ordinateur.
