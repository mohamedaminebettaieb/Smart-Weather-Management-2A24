SMART WEATHER MANAGEMENT - Integration Equipements (metiers avances) + Techniciens (metiers avances)
===================================================================================================

BASE : projet Equipements fourni (CRUD-FINAL-EQUIPMENT-METIERS) : equipements, QR code, prediction de
maintenance, login en premier, menu verrouille jusqu'a la connexion.
AJOUT : metiers avances Techniciens (maintenance des equipements + recrutement par analyse de CV).

AUCUN SCRIPT SQL : tables EQUIPEMENTS, TECHNICIENS, MAINTENIR deja existantes.

FICHIERS AJOUTES (techniciens)  : maintenance.*, maintenancedialog.*, recrutementcv.*, cvcore.*, cvanalyzer.*
FICHIERS REMPLACES              : technicien.*, technicienne.*   (version avec metiers)
FICHIERS ADAPTES (petits ajouts): weatherwisetest.h/.cpp  -> rechargerDepuisOracle()
                                  smartweatherwindow.cpp  -> connect(equipementsModifies -> rechargerDepuisOracle)
                                  SmartWeather.pro        -> nouveaux fichiers
                                  connection.cpp          -> #include "connection.h" (minuscules) + identifiants
FICHIERS NON TOUCHES            : equipement.*, gbureaux.*, bureau.*, gutilisateur.*, stationwindow.*, utilisateur.*,
                                  main.cpp, qrcodegen.*

COMPILER (Qt Creator) : ouvrir SmartWeather.pro > Executer qmake > Nettoyer tout > Reconstruire tout > Executer.
IDENTIFIANTS ORACLE : connection.cpp (source ODBC Source_Projet2A) - remplacer utilisateur / mot de passe par les votres.
