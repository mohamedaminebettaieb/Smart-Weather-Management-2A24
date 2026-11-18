# Smart Weather Management 2A24 (version intégrée, qmake)

Ouvrir **SmartWeather.pro** avec Qt Creator (kit Qt 6, C++17, module QODBC).
Projet qmake uniquement : pas de CMakeLists.txt.

Gestion des utilisateurs : classe `Utilisateur` (utilisateur.h/.cpp) = CRUD Oracle ;
`Gutilisateur` = interface (connexion, liste, recherche, tri, PDF, stats).

Base : exécuter base_oracle.sql (si pas déjà fait), puis utilisateurs_migration.sql une seule fois.
Vérifier le DSN / utilisateur / mot de passe dans connection.cpp (propres à chaque PC).
