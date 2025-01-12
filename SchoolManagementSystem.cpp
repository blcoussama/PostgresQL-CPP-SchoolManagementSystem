
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <libpq-fe.h>  // PostgreSQL library
#include <cstdlib>  // For getenv()
#include <string>
#include <vector>

using namespace std;

class Database {
private:
    PGconn* conn; // Déclarer le pointeur pour la connexion avec PostgresQL.

public:
    // Constructeur pour établie la connexion avec la base de donnée.
    Database() {
        // Importer les variables d'environements crées.
        string dbName = getenv("PG_DB_NAME");
        string dbUser = getenv("PG_DB_USER");
        string dbPassword = getenv("PG_DB_PASSWORD");
        string dbHost = getenv("PG_DB_HOST");

        // String de Connection a la Base de données PostgresQL.
        string connexionString = ("dbname=" + dbName + " user=" + dbUser + " password=" + dbPassword + " hostaddr=" + dbHost + " port=5000");

        // Utiliser le connInfo String pour etablir la connexion.
        conn = PQconnectdb(connexionString.c_str()); /* Utiliser "c_str()" pour convertir le string de connexion a un string de Style C,
                                                        qui est requis par la fonction PQconnectdb de PostgresQL.*/

        if (PQstatus(conn) == CONNECTION_BAD) {
            cerr << "La Connexion a la Base de donnee a echoue!" << endl;
        }
        else {
            cout << "La Base de Donnee PostgresQL a ete Connectee avec Success." << endl;
        }
    }

    // Destructeur pour arreter la connexion avec la base de donnée.
    ~Database() {
        PQfinish(conn); // Arreter la connexion quand l'objet est détruit.
    }

    // Getter pour accéder au pointeur de connexion.
    PGconn* getConnexion() {
        return conn;
    }

    // Méthode pour exécuter une requête SQL générique.
    PGresult* executeQuery(const string& query) {
        PGconn* conn = getConnexion();
        PGresult* res = PQexec(conn, query.c_str());
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            cerr << "Échec de l'exécution de la requête : " << PQerrorMessage(conn) << endl;
        }
        return res;

    }

    // Method Pour ajouter les admins dans la base des donnees
    void Ajouter_Admin(const string& Nom, const string& Email, const string& Mdp) {
        string query = "INSERT INTO admins (nom, email, mdp) VALUES ('" + Nom + "', '" + Email + "', '" + Mdp + "');";
        executeQuery(query);
        cout << "Admin Ajoute avec Success." << endl;
    }

};

struct Date {
    int jour;
    int mois;
    int annee;
};

class Utilisateur {
private:
    int Utilisateur_id;
    string Mdp;

protected:
    string Nom, Email;

public: 
    Utilisateur(int id, const string& nom, const string& email, const string& mdp) 
        : Utilisateur_id(id), Nom(nom), Email(email), Mdp(mdp) {}

    virtual void Affichage() const = 0; // Méthode virtuelle pure

    
    int GetUtilisateurId() const {
        return Utilisateur_id;
    }

    string GetMdp() const {
        return Mdp;
    }
};

class Admin : public Utilisateur {

public:
    Admin(int id, const string& nom, const string& email, const string& mdp)
        : Utilisateur(id, nom, email, mdp) {}

    void Affichage() const override {
        cout << "Admin:" << endl;
        cout << "ID: " << GetUtilisateurId() << ", Nom: " << Nom << ", Email: " << Email << ", Mot de passe: " << GetMdp() << endl;
    }
};

class Enseignant : public Utilisateur {

protected:
    Matiere Matiere_assignee;

public:
    Enseignant(int id, const string& nom, const string& email, const string& mdp, const Matiere& matiere)
        : Utilisateur(id, nom, email, mdp), Matiere_assignee(matiere) {}

    void Affichage() const override {
        cout << "Enseignant:" << endl;
        cout << "ID: " << GetUtilisateurId() << ", Nom: " << Nom << ", Email: " << Email << ", Mot de passe: " << GetMdp() << endl;
        Matiere_assignee.Affichage();
    }

};

class Etudiant : public Utilisateur {

protected:
    Date Date_Naissance;
    vector<Classe> Classes;
    Parent Parent_Etudiant;

public:

    Etudiant(int id, const string& nom, const string& email, const string& mdp, const Date date_naissence, const Parent& parent, const vector<Classe>& classes)
        : Utilisateur(id, nom, email, mdp), Date_Naissance(date_naissence), Parent_Etudiant(parent), Classes(classes) {}

    void Affichage() const override {
        cout << "Etudiant:" << endl;
        cout << "ID: " << GetUtilisateurId() << ", Nom: " << Nom << ", Email: " << Email << ", Mot de passe: " << GetMdp() << endl;
        cout << "Date de naissance: " << Date_Naissance.jour << "/" << Date_Naissance.mois << "/" << Date_Naissance.annee << endl;
    }

};

class Parent : public Utilisateur {

public:
    Parent(int id, const string& nom, const string& email, const string& mdp)
        : Utilisateur(id, nom, email, mdp) {}

    void Affichage() const override {
        cout << "Parent:" << endl;
        cout << "ID: " << GetUtilisateurId() << ", Nom: " << Nom << ", Email: " << Email << ", Mot de passe: " << GetMdp() << endl;
    }
};

class Classe {

private:
    int Classe_id;

protected:
    string Nom;
    vector<Enseignant> Enseignants;

public:
    Classe(int id, const string& nom) : Classe_id(id), Nom(nom) {}

    void Affichage() const {
        cout << "Classe:" << endl;
        cout << "ID:" << Classe_id << "Nom:" << Nom << endl;

        for (const auto& enseignant : Enseignants) {
            enseignant.Affichage();
        }
    }
};

class Matiere {

private:
    int Matiere_id;

protected:
    string Nom;

public:
    Matiere(int id, const string& nom) : Matiere_id(id), Nom(nom) {}

    void Affichage() const {
        cout << "Matiere:" << endl;
        cout << "ID:" << Matiere_id << "Nom:" << Nom << endl;
    }
};

class Examen {
private:
    int Examen_id;

protected:
    string Titre, Description;
    Date Date_examen;
    vector<Classe> Classes;

public:
    Examen(int id, const string& titre, const string& description, const Date date, const vector<Classe>& classes)
    : Examen_id(id), Titre(titre), Description(description), Date_examen(date), Classes(classes) {}

    void Affichage() const {

        cout << "Examen ID: " << Examen_id << endl;
        cout << "Titre: " << Titre << endl;
        cout << "Description: " << Description << endl;
        cout << "Date de l'examen: " << Date_examen.jour << "/" << Date_examen.mois << "/" << Date_examen.annee << endl;

        for (const auto& classe : Classes) {
            classe.Affichage();
        }
    }
};

int main()
{
    // Créer un Objet de la classe Database qui va établir la connexion automatiquement.
    Database DB;

    // Ajouter un Admin
    string adminNom, adminEmail, adminMdp;
    cout << "Entrez le nom de  l'admin: ";
    cin >> adminNom;
    cout << "Entez l'email de l'admin: ";
    cin >> adminEmail;
    cout << "Entez le mot de passe de l'admin: ";
    cin >> adminMdp;

    DB.Ajouter_Admin(adminNom, adminEmail, adminMdp);



    return 0; // La connexion est automatiquement arretée grace au destructeur quand le code est exécuté et terminé et l'Objet est détruit. 
}

