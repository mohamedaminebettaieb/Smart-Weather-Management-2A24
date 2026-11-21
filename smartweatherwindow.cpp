#include "smartweatherwindow.h"

#include "gbureaux.h"
#include "gutilisateur.h"
#include "stationwindow.h"
#include "technicienne.h"
#include "weatherwisetest.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QList>
#include <QPushButton>
#include <QStackedWidget>
#include <QStyle>
#include <QVBoxLayout>

// 1 = saute la connexion (développement), 0 = connexion normale.
// Remettre à 0 avant de pousser sur GitHub.
#define SKIP_LOGIN_DEV 0

SmartWeatherWindow::SmartWeatherWindow(QWidget *parent)
    : QMainWindow(parent),
    stack(new QStackedWidget(this)),
    bureauButton(nullptr),
    equipmentButton(nullptr),
    stationsButton(nullptr),
    techniciensButton(nullptr),
    usersButton(nullptr),
    stationWindow(new StationWindow),
    technicienneWindow(new Technicienne),
    bureauxWindow(new GBureaux),
    equipmentWindow(new WeatherWiseTest),
    usersWindow(new Gutilisateur)
{
    setWindowTitle("SmartWeather");
    resize(1450, 850);
    setMinimumSize(1200, 700);

    QWidget *central = new QWidget(this);

    QHBoxLayout *layout = new QHBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    layout->addWidget(createSidebar());
    layout->addWidget(stack, 1);

    setCentralWidget(central);

    preparePage(stationWindow);
    preparePage(technicienneWindow);
    preparePage(bureauxWindow);
    preparePage(equipmentWindow);
    preparePage(usersWindow);

    stack->addWidget(stationWindow);
    stack->addWidget(technicienneWindow);
    stack->addWidget(bureauxWindow);
    stack->addWidget(equipmentWindow);
    stack->addWidget(usersWindow);

    // Start on the login page and lock every other page
    showPage(usersWindow, usersButton);

    QFrame *sidebar = findChild<QFrame *>("mainSidebar");
    const QList<QPushButton *> navButtons =
        sidebar ? sidebar->findChildren<QPushButton *>("navButton")
                : QList<QPushButton *>();

    for (QPushButton *button : navButtons)
        button->setEnabled(button == usersButton);

    // Unlock the sidebar only when the login page reports a successful login
    connect(usersWindow, &Gutilisateur::loginSucceeded, this,
            [this, navButtons]()
            {
                for (QPushButton *button : navButtons)
                    button->setEnabled(true);

                showPage(stationWindow, stationsButton);
            });

    // Lock the sidebar again when the user logs out
    connect(usersWindow, &Gutilisateur::loggedOut, this,
            [this, navButtons]()
            {
                for (QPushButton *button : navButtons)
                    button->setEnabled(button == usersButton);

                showPage(usersWindow, usersButton);
            });

#if SKIP_LOGIN_DEV
    // Mode développement : pas de connexion, tout le menu est débloqué
    for (QPushButton *button : navButtons)
        button->setEnabled(true);

    showPage(equipmentWindow, equipmentButton);
#endif
}

QFrame *SmartWeatherWindow::createSidebar()
{
    QFrame *sidebar = new QFrame(this);

    sidebar->setObjectName("mainSidebar");
    sidebar->setFixedWidth(285);

    QVBoxLayout *layout = new QVBoxLayout(sidebar);

    layout->setContentsMargins(8, 22, 8, 16);
    layout->setSpacing(8);

    QLabel *logo = new QLabel("*", sidebar);
    logo->setObjectName("mainLogo");
    logo->setAlignment(Qt::AlignCenter);

    QLabel *title = new QLabel("SMART WEATHER\nMANAGEMENT", sidebar);
    title->setObjectName("mainTitle");
    title->setAlignment(Qt::AlignCenter);

    layout->addWidget(logo);
    layout->addWidget(title);
    layout->addSpacing(22);

    QPushButton *homeButton = createNavigationButton("Accueil");
    bureauButton = createNavigationButton("Gestion de bureau");
    equipmentButton = createNavigationButton("Gestion des equipements");
    techniciensButton = createNavigationButton("Gestion des techniciens");
    stationsButton = createNavigationButton("Gestion des stations");
    usersButton = createNavigationButton("Gestion des utilisateurs");

    layout->addWidget(homeButton);
    layout->addWidget(bureauButton);
    layout->addWidget(equipmentButton);
    layout->addWidget(techniciensButton);
    layout->addWidget(stationsButton);
    layout->addWidget(usersButton);
    layout->addStretch();

    connect(bureauButton, &QPushButton::clicked, this,
            [this]() { showPage(bureauxWindow, bureauButton); });

    connect(equipmentButton, &QPushButton::clicked, this,
            [this]() { showPage(equipmentWindow, equipmentButton); });

    connect(stationsButton, &QPushButton::clicked, this,
            [this]() { showPage(stationWindow, stationsButton); });

    connect(techniciensButton, &QPushButton::clicked, this,
            [this]() { showPage(technicienneWindow, techniciensButton); });

    connect(usersButton, &QPushButton::clicked, this,
            [this]() { showPage(usersWindow, usersButton); });

    sidebar->setStyleSheet(R"(
        #mainSidebar {
            background: #17467D;
        }

        #mainLogo {
            color: #FFD21F;
            font-size: 48px;
            font-weight: bold;
            background: transparent;
        }

        #mainTitle {
            color: white;
            font-size: 18px;
            font-weight: bold;
            background: transparent;
        }

        QPushButton#navButton {
            min-height: 48px;
            border: none;
            border-radius: 7px;
            padding-left: 22px;
            text-align: left;
            color: white;
            background: transparent;
            font-size: 14px;
        }

        QPushButton#navButton:hover {
            background: #2468B4;
        }

        QPushButton#navButton:disabled {
            color: rgba(255, 255, 255, 0.35);
            background: transparent;
        }

        QPushButton#navButton[active="true"] {
            background: #2F83D8;
            font-weight: bold;
        }
    )");

    return sidebar;
}

QPushButton *SmartWeatherWindow::createNavigationButton(const QString &text)
{
    QPushButton *button = new QPushButton(text, this);

    button->setObjectName("navButton");
    button->setProperty("active", false);
    button->setCursor(Qt::PointingHandCursor);

    return button;
}

void SmartWeatherWindow::preparePage(QWidget *page)
{
    QWidget *sidebar = page->findChild<QWidget *>("sidebar");

    if (sidebar)
    {
        sidebar->hide();
        sidebar->setFixedWidth(0);
    }

    QWidget *menu = page->findChild<QWidget *>("menu");

    if (menu)
    {
        menu->hide();
        menu->setFixedWidth(0);
    }
}

void SmartWeatherWindow::showPage(QWidget *page, QPushButton *activeButton)
{
    stack->setCurrentWidget(page);
    updateActiveButton(activeButton);
}

void SmartWeatherWindow::updateActiveButton(QPushButton *activeButton)
{
    QPushButton *buttons[] = {
        bureauButton,
        equipmentButton,
        stationsButton,
        techniciensButton,
        usersButton
    };

    for (QPushButton *button : buttons)
    {
        if (!button)
            continue;

        button->setProperty("active", button == activeButton);
        button->style()->unpolish(button);
        button->style()->polish(button);
        button->update();
    }
}