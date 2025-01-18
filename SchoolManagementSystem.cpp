#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <libpq-fe.h>  // Bibliothèque PostgreSQL
#include <cstdlib>     // Pour getenv()
#include <string>
#include <vector>

using namespace std;

// Classe Base de Données
class Database {
private:
    PGconn* conn; // Pointeur pour la connexion PostgreSQL

public:
    Database() {
        // Récupérer les variables d'environnement
        string dbName = getenv("PG_DB_NAME");
        string dbUser = getenv("PG_DB_USER");
        string dbPassword = getenv("PG_DB_PASSWORD");
        string dbHost = getenv("PG_DB_HOST");

        // Chaîne de connexion pour PostgreSQL
        string connectionString = "dbname=" + dbName + " user=" + dbUser + " password=" + dbPassword + " hostaddr=" + dbHost + " port=5000";
        conn = PQconnectdb(connectionString.c_str());

        if (PQstatus(conn) == CONNECTION_BAD) {
            cerr << "Echec de la connexion a la base de donnees : " << PQerrorMessage(conn) << endl;
        }
        else {
            cout << "Connexion a la base de donnees PostgreSQL reussie." << endl;
        }
    }

    ~Database() {
        PQfinish(conn); // Fermer la connexion lorsque l'objet est détruit
    }

    PGconn* getConnection() {
        return conn;
    }

    PGresult* executeQuery(const string& query) {
        PGresult* res = PQexec(conn, query.c_str());
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            cerr << "Echec de l'execution de la requete : " << PQerrorMessage(conn) << endl;
        }
        return res;
    }

    void Ajouter_Admin(const string& Nom, const string& Email, const string& Mdp) {
        string query = "INSERT INTO admins (nom, email, mdp) VALUES ('" + Nom + "', '" + Email + "', '" + Mdp + "');";
        PGresult* res = executeQuery(query);
        if (PQresultStatus(res) == PGRES_COMMAND_OK) {
            cout << "Admin ajoute avec succes." << endl;
        }
        PQclear(res);
    }

    void Supprimer_Admin(int ID) {
        string query = "DELETE FROM admins WHERE admin_id = " + to_string(ID) + ";";
        PGresult* res = executeQuery(query);
        if (PQresultStatus(res) == PGRES_COMMAND_OK) {
            cout << "Admin supprime avec succes." << endl;
        }
        else {
            cerr << "Echec de la suppression de l'admin : " << PQerrorMessage(conn) << endl;
        }
        PQclear(res);
    }
};

// Structure de Date
struct Date {
    int jour, mois, annee;
};


// CLASS MATIERE
class Matiere {
private:
    int Matiere_id;
    string Nom;

public:
    Matiere(int id, const string& nom) : Matiere_id(id), Nom(nom) {}

    void Affichage() const {
        cout << "ID Matiere : " << Matiere_id << ", Nom : " << Nom << endl;
    }
};

// CLASS CLASSE
class Classe {
private:
    int Classe_id;
    string Nom;
    vector<Matiere> Matieres;

public:
    Classe(int id, const string& nom) : Classe_id(id), Nom(nom) {}

    void Affichage() const {
        cout << "ID Classe : " << Classe_id << ", Nom : " << Nom << endl;
        for (const auto& matiere : Matieres) {
            matiere.Affichage();
        }
    }
};

// CLASS EXAMEN
class Examen {
private:
    int Examen_id;
    string Titre, Description;
    Date Date_examen;
    vector<Classe> Classes;

public:
    Examen(int id, const string& titre, const string& description, const Date& date, const vector<Classe>& classes)
        : Examen_id(id), Titre(titre), Description(description), Date_examen(date), Classes(classes) {
    }

    void Affichage() const {
        cout << "ID Examen : " << Examen_id << endl;
        cout << "Titre : " << Titre << endl;
        cout << "Description : " << Description << endl;
        cout << "Date de l'examen : " << Date_examen.jour << "/" << Date_examen.mois << "/" << Date_examen.annee << endl;
        for (const auto& classe : Classes) {
            classe.Affichage();
        }
    }
};

// CLASS UTILISATEUR
class Utilisateur {
private:
    int Utilisateur_id;
    string Mdp;

protected:
    string Nom, Email;

public:
    Utilisateur(int id, const string& nom, const string& email, const string& mdp)
        : Utilisateur_id(id), Nom(nom), Email(email), Mdp(mdp) {
    }

    virtual void Affichage() const = 0;

    int GetUtilisateurId() const {
        return Utilisateur_id;
    }

    string GetMdp() const {
        return Mdp;
    }
};

// CLASS ADMIN
class Admin : public Utilisateur {
public:
    Admin(int id, const string& nom, const string& email, const string& mdp)
        : Utilisateur(id, nom, email, mdp) {
    }
    void Affichage() const override {
        cout << "Admin:" << endl;
        cout << "ID: " << GetUtilisateurId() << ", Nom: " << Nom << ", Email: " << Email << ", Mot de passe: " << GetMdp() << endl;
    }
};

// CLASS PARENT
class Parent : public Utilisateur {
public:
    Parent(int id, const string& nom, const string& email, const string& mdp)
        : Utilisateur(id, nom, email, mdp) {
    }

    void Affichage() const override {
        cout << "ID Parent : " << GetUtilisateurId() << ", Nom : " << Nom << ", Email : " << Email << endl;
    }
};

// CLASS ENSEIGNANT
class Enseignant : public Utilisateur {
private:
    Matiere Matiere_assignee;

public:
    Enseignant(int id, const string& nom, const string& email, const string& mdp, const Matiere& matiere)
        : Utilisateur(id, nom, email, mdp), Matiere_assignee(matiere) {
    }

    void Affichage() const override {
        cout << "ID Enseignant : " << GetUtilisateurId() << ", Nom : " << Nom << ", Email : " << Email << endl;
        Matiere_assignee.Affichage();
    }
};

// CLASS ETUDIANT
class Etudiant : public Utilisateur {
private:
    Date Date_Naissance;
    Parent Parent_Etudiant;
    vector<Classe> Classes;

public:
    Etudiant(int id, const string& nom, const string& email, const string& mdp, const Date& dateNaissance, const Parent& parent, const vector<Classe>& classes)
        : Utilisateur(id, nom, email, mdp), Date_Naissance(dateNaissance), Parent_Etudiant(parent), Classes(classes) {
    }

    void Affichage() const override {
        cout << "ID Etudiant : " << GetUtilisateurId() << ", Nom : " << Nom << ", Email : " << Email << endl;
        cout << "Date de naissance : " << Date_Naissance.jour << "/" << Date_Naissance.mois << "/" << Date_Naissance.annee << endl;
        Parent_Etudiant.Affichage();
        for (const auto& classe : Classes) {
            classe.Affichage();
        }
    }
};

// CLASS NOTE
class Note {
private:
    int Note_id;
    float Note_val;
    Etudiant Etudiant_assignee;
    Examen Examen_assignee;

public:
    Note(int id, float note, const Etudiant& etudiant, const Examen& examen)
        : Note_id(id), Note_val(note), Etudiant_assignee(etudiant), Examen_assignee(examen) {}

    void Affichage() const {
        cout << "ID Note: " << Note_id << ", Note: " << Note_val << endl;
        cout << "Etudiant :" << endl;
        Etudiant_assignee.Affichage();
        cout << "Examen: " << endl;
        Examen_assignee.Affichage();
    }
};

// Fonction principale
int main() {
    // Créer un objet Database pour établir automatiquement la connexion
    Database DB;

    // Ajouter un admin
    /*string adminNom, adminEmail, adminMdp;
    cout << "Entrez le nom de l'admin : ";
    cin >> adminNom;
    cout << "Entrez l'email de l'admin : ";
    cin >> adminEmail;
    cout << "Entrez le mot de passe de l'admin : ";
    cin >> adminMdp;

    DB.Ajouter_Admin(adminNom, adminEmail, adminMdp);*/

    // Supprimer un admin
    /*int adminID;
    cout << "Entrez l'ID de l'admin à supprimer : ";
    cin >> adminID;

    DB.Supprimer_Admin(adminID);*/

    return 0; // La connexion est automatiquement fermée lorsque l'objet Database est détruit
}
