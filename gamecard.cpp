#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QPainter>
#include <QMessageBox>
#include <algorithm>
#include <random>
#include <chrono>

// Classe Card représentant une carte à jouer
class Card {
public:
    Card(int valeur, int couleur) : valeur(valeur), couleur(couleur) {}
    int getValeur() const { return valeur; }
    int getCouleur() const { return couleur; }

private:
    int valeur;
    int couleur;
};

// Widget pour afficher une carte avec une interface cliquable
class CardWidget : public QWidget {
    Q_OBJECT

public:
    CardWidget(const Card& carte, QWidget* parent = nullptr) : QWidget(parent), carte(carte) {
        setFixedSize(50, 70);
        connect(this, &CardWidget::clicked, this, &CardWidget::handleClick);
    }

protected:
    // Peindre la carte
    void paintEvent(QPaintEvent* event) override {
        Q_UNUSED(event);

        QPainter peintre(this);
        peintre.fillRect(rect(), Qt::white);
        peintre.drawRect(rect());
        peintre.drawText(rect(), Qt::AlignCenter, QString::number(carte.getValeur()));
    }

signals:
    void clicked();

private slots:
    // Gérer les événements de clic sur la carte
    void handleClick() {
        emit clicked();
    }

private:
    Card carte;
};

// Classe Deck représentant un jeu de cartes à jouer
class Deck {
public:
    Deck() {
        for (int i = 0; i < 4; i++) {
            for (int j = 1; j <= 10; j++) {
                cartes.push_back(Card(j, i));
            }
        }
    }
    // Mélanger le jeu de cartes
    void melanger() {
        unsigned graine = std::chrono::system_clock::now().time_since_epoch().count();
        std::shuffle(cartes.begin(), cartes.end(), std::default_random_engine(graine));
    }
    // Tirer une carte du dessus du jeu de cartes
    Card tirer() {
        Card carte = cartes.back();
        cartes.pop_back();
        return carte;
    }
    // Vérifier si le jeu de cartes est vide
    bool vide() const { return cartes.empty(); }

private:
    std::vector<Card> cartes;
};

// Classe CardGame représentant la logique du jeu
class CardGame : public QObject {
    Q_OBJECT

public:
    CardGame(QObject* parent = nullptr) : QObject(parent), scoreJoueur(0), scoreAdversaire(0), tourJoueur(true) {
        commencerNouveauJeu();
    }

public slots:
    // Commencer une nouvelle partie
    void commencerNouveauJeu() {
        jeu = Deck();
        jeu.melanger();

        mainJoueur.clear();
        mainAdversaire.clear();
        scoreJoueur = 0;
        scoreAdversaire = 0;
        tourJoueur = true;

        distribuerCartesInitiales();

        emit jeuMisAJour();
    }

    // Jouer une carte
    void jouerCarte() {
        if (!tourJoueur) {
            return; // Ce n'est pas le tour du joueur, ignorer l'action
        }

        if (!mainJoueur.empty()) {
            Card carteJouee = mainJoueur.back();
            mainJoueur.pop_back();
            scoreJoueur += carteJouee.getValeur();
            tourJoueur = false;

            jouerAdversaire();

            emit jeuMisAJour();

            verifierFinDePartie();
        }
    }

    // Effectuer une action "prendre"
    void actionPrendre() {
        // Implémenter la logique pour l'action "prendre"
        // À des fins de démonstration, supposons que prendre ajoute 5 points au score du joueur
        scoreJoueur += 5;
        tourJoueur = false;

        jouerAdversaire();

        emit jeuMisAJour();

        verifierFinDePartie();
    }

    // Effectuer une action "annoncer"
    void actionAnnoncer() {
        // Implémenter la logique pour l'action "annoncer"
        // À des fins de démonstration, supposons que annoncer ajoute 10 points au score du joueur
        scoreJoueur += 10;
        tourJoueur = false;

        jouerAdversaire();

        emit jeuMisAJour();

        verifierFinDePartie();
    }

signals:
    void jeuMisAJour();
    void finDePartie(QString gagnant);

private:
    Deck jeu;
    std::vector<Card> mainJoueur;
    std::vector<Card> mainAdversaire;
    int scoreJoueur;
    int scoreAdversaire;
    bool tourJoueur;

    // Distribuer des cartes initiales aux joueurs
    void distribuerCartesInitiales() {
        for (int i = 0; i < 5; i++) {
            mainJoueur.push_back(jeu.tirer());
            mainAdversaire.push_back(jeu.tirer());
        }
    }

    // Laisser l'adversaire jouer (logique simple de l'IA)
    void jouerAdversaire() {
        if (!mainAdversaire.empty()) {
            // Logique IA améliorée : l'adversaire joue la carte de la plus grande valeur
            auto maxCarteIt = std::max_element(mainAdversaire.begin(), mainAdversaire.end(),
                                              [](const Card& a, const Card& b) { return a.getValeur() < b.getValeur(); });

            Card carteJouee = *maxCarteIt;
            mainAdversaire.erase(maxCarteIt);
            scoreAdversaire += carteJouee.getValeur();
            tourJoueur = true;
        }
    }

    // Vérifier si la partie est terminée
    void verifierFinDePartie() {
        if (mainJoueur.empty() && mainAdversaire.empty()) {
            QString gagnant;
            if (scoreJoueur > scoreAdversaire) {
                gagnant = "Joueur";
            } else if (scoreJoueur < scoreAdversaire) {
                gagnant = "Adversaire";
            } else {
                gagnant = "C'est une égalité !";
            }

            emit finDePartie(gagnant);
        }
    }
};

// Classe MainWindow représentant l'interface utilisateur
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr) : QMainWindow(parent), jeu(new CardGame(this)) {
        miseEnPagePrincipale = new QVBoxLayout();

        etiquetteMainJoueur = new QLabel("Main du Joueur :", this);
        etiquetteMainAdversaire = new QLabel("Main de l'Adversaire :", this);
        etiquetteScores = new QLabel("Scores", this);

        boutonJouerCarte = new QPushButton("Jouer Carte", this);
        boutonPrendre = new QPushButton("Prendre", this);
        boutonAnnoncer = new QPushButton("Annoncer", this);
        boutonNouvellePartie = new QPushButton("Nouvelle Partie", this);

        connect(boutonJouerCarte, &QPushButton::clicked, this, &MainWindow::onJouerCarteClique);
        connect(boutonPrendre, &QPushButton::clicked, this, &MainWindow::onPrendreClique);
        connect(boutonAnnoncer, &QPushButton::clicked, this, &MainWindow::onAnnoncerClique);
        connect(boutonNouvellePartie, &QPushButton::clicked, this, &MainWindow::onNouvellePartieClique);

        miseEnPagePrincipale->addWidget(etiquetteMainJoueur);
        miseEnPagePrincipale->addWidget(etiquetteMainAdversaire);
        miseEnPagePrincipale->addWidget(etiquetteScores);
        miseEnPagePrincipale->addWidget(boutonJouerCarte);
        miseEnPagePrincipale->addWidget(boutonPrendre);
        miseEnPagePrincipale->addWidget(boutonAnnoncer);
        miseEnPagePrincipale->addWidget(boutonNouvellePartie);

        QWidget* widgetCentral = new QWidget(this);
        widgetCentral->setLayout(miseEnPagePrincipale);
        setCentralWidget(widgetCentral);

        mettreAJourUI();
    }

private slots:
    // Gérer le clic sur le bouton "Jouer Carte"
    void onJouerCarteClique() {
        jeu->jouerCarte();
        mettreAJourUI();
    }

    // Gérer le clic sur le bouton "Prendre"
    void onPrendreClique() {
        jeu->actionPrendre();
        mettreAJourUI();
    }

    // Gérer le clic sur le bouton "Annoncer"
    void onAnnoncerClique() {
        jeu->actionAnnoncer();
        mettreAJourUI();
    }

    // Gérer le clic sur le bouton "Nouvelle Partie"
    void onNouvellePartieClique() {
        jeu->commencerNouveauJeu();
        mettreAJourUI();
    }

    // Mettre à jour les composants de l'interface utilisateur
    void mettreAJourUI() {
        // Effacer les widgets de carte existants
        QLayoutItem* enfant;
        while ((enfant = miseEnPagePrincipale->takeAt(0)) != nullptr) {
            delete enfant->widget();
            delete enfant;
        }

        // Afficher la main du joueur en utilisant CardWidget
        etiquetteMainJoueur->setText("Main du Joueur :");
        for (const Card& carte : jeu->getMainJoueur()) {
            CardWidget* widgetCarte = new CardWidget(carte, this);
            miseEnPagePrincipale->addWidget(widgetCarte);
        }

        // Afficher la main de l'adversaire en utilisant CardWidget
        etiquetteMainAdversaire->setText("Main de l'Adversaire :");
        for (const Card& carte : jeu->getMainAdversaire()) {
            CardWidget* widgetCarte = new CardWidget(carte, this);
            miseEnPagePrincipale->addWidget(widgetCarte);
        }

        // Afficher les scores
        QString scoresStr = "Scores - Joueur : " + QString::number(jeu->getScoreJoueur()) +
                            " | Adversaire : " + QString::number(jeu->getScoreAdversaire());
        etiquetteScores->setText(scoresStr);

        // Activer ou désactiver les actions du joueur en fonction du tour
        activerActionsJoueur(jeu->estTourJoueur());
    }

    // Activer ou désactiver les actions du joueur en fonction du tour
    void activerActionsJoueur(bool activer) {
        boutonJouerCarte->setEnabled(activer);
        boutonPrendre->setEnabled(activer);
        boutonAnnoncer->setEnabled(activer);
    }

private:
    QVBoxLayout* miseEnPagePrincipale;
    QLabel* etiquetteMainJoueur;
    QLabel* etiquetteMainAdversaire;
    QLabel* etiquetteScores;
    QPushButton* boutonJouerCarte;
    QPushButton* boutonPrendre;
    QPushButton* boutonAnnoncer;
    QPushButton* boutonNouvellePartie;
    CardGame* jeu;
};

// Point d'entrée de l'application
int main(int argc, char** argv) {
    QApplication application(argc, argv);

    // Créer et afficher la fenêtre principale
    MainWindow fenetrePrincipale;
    fenetrePrincipale.show();

    // Exécuter la boucle d'événements de l'application
    return application.exec();
}

#include "moroccancardgame.moc"

