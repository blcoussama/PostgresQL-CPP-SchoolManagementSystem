#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <libpq-fe.h>  // Bibliothèque PostgreSQL
#include <cstdlib>     // Pour getenv()
#include <string>
#include <vector>

using namespace std;

// Structure de Date
struct Date {
    int jour, mois, annee;
};

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

    // ajouter un admin
    void Ajouter_Admin(const string& Nom, const string& Email, const string& Mdp) {
        string query = "INSERT INTO admins (nom, email, mdp) VALUES ('" + Nom + "', '" + Email + "', '" + Mdp + "');";
        PGresult* res = executeQuery(query);
        if (PQresultStatus(res) == PGRES_COMMAND_OK) {
            cout << "Admin ajoute avec succes." << endl;
        }
        PQclear(res);
    }

    // supprimer un admin
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

// CLASS PARENT
class Parent : public Utilisateur {
public:
    Parent(int id, const string& nom, const string& email, const string& mdp)
        : Utilisateur(id, nom, email, mdp) {
    }

    void Affichage() const override {
        cout << "ID Parent : " << GetUtilisateurId() << ", Nom : " << Nom << ", Email : " << Email << endl;
    }

    int GetParentId() const {
        return GetUtilisateurId();
    }
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

    int GetMatiereId() const {
        return Matiere_id;
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

    Matiere GetMatiere() const {
        return Matiere_assignee;
    }
};

// CLASS CLASSE
class Classe {
private:
    int Classe_id;
    string Nom;
    vector<Enseignant> Enseignants;

public:
    Classe(int id, const string& nom) : Classe_id(id), Nom(nom) {}

    void Affichage() const {
        cout << "ID Classe : " << Classe_id << ", Nom : " << Nom << endl;
        for (const auto& enseignant : Enseignants) {
            enseignant.Affichage();
        }
    }

    int GetClasseId() const {
        return Classe_id;
    }
};

// CLASS ETUDIANT
class Etudiant : public Utilisateur {
private:
    Date Date_Naissance;
    Parent Parent_Etudiant;
    Classe Classe_Etudiant;

public:
    Etudiant(int id, const string& nom, const string& email, const string& mdp, const Date& dateNaissance, const Parent& parent, const Classe& classe)
        : Utilisateur(id, nom, email, mdp), Date_Naissance(dateNaissance), Parent_Etudiant(parent), Classe_Etudiant(classe) {
    }

    void Affichage() const override {
        cout << "ID Etudiant : " << GetUtilisateurId() << ", Nom : " << Nom << ", Email : " << Email << endl;
        cout << "Date de naissance : " << Date_Naissance.jour << "/" << Date_Naissance.mois << "/" << Date_Naissance.annee << endl;
        Parent_Etudiant.Affichage();
        Classe_Etudiant.Affichage();
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

// CLASS NOTE
class Note {
private:
    int Note_id;
    float Note_val;
    Etudiant Etudiant_assignee;
    Examen Examen_assignee;

public:
    Note(int id, float note, const Etudiant& etudiant, const Examen& examen)
        : Note_id(id), Note_val(note), Etudiant_assignee(etudiant), Examen_assignee(examen) {
    }

    void Affichage() const {
        cout << "ID Note: " << Note_id << ", Note: " << Note_val << endl;
        cout << "Etudiant :" << endl;
        Etudiant_assignee.Affichage();
        cout << "Examen: " << endl;
        Examen_assignee.Affichage();
    }
};

// CLASS ADMIN
class Admin : public Utilisateur {
private:
    Database& DB; // Reference to database connection

public:
    Admin(int id, const string& nom, const string& email, const string& mdp, Database& database)
        : Utilisateur(id, nom, email, mdp), DB(database) {
    }

    void Affichage() const override {
        cout << "Admin:" << endl;
        cout << "ID: " << GetUtilisateurId() << ", Nom: " << Nom << ", Email: " << Email << ", Mot de passe: " << GetMdp() << endl;
    }

    // CREER UNE NOUVELLE MATIERE
    Matiere Creer_Matiere(const string& Nom) {
        string query = "INSERT INTO matieres (nom) VALUES ('" + Nom + "') RETURNING matiere_id;";
        PGresult* res = DB.executeQuery(query);

        // recuperer le ID genere par PostgresQL pour la nouvelle matiere
        int matiere_id = atoi(PQgetvalue(res, 0, 0));
        PQclear(res);

        // retourner la matiere cree
        return Matiere(matiere_id, Nom);
    }

    //CREER UNE NOUVEAU ENSEIGNANT
    Enseignant Creer_Enseignant(const string& Nom, const string& Email, const string& Mdp, const Matiere& matiere) {
        string query = "INSERT INTO enseignants (nom, email, mdp, matiere_id) VALUES ('" 
                            + Nom + "', '" 
                            + Email + "', '"
                            + Mdp + "', '"
                            + to_string(matiere.GetMatiereId()) + 
                       ") RETURNING enseignant_id;";

        PGresult* res = DB.executeQuery(query);
        int enseignant_id = atoi(PQgetvalue(res, 0, 0));
        PQclear(res);

        return Enseignant(enseignant_id, Nom, Email, Mdp, matiere);
    }

    // CREER UNE NOUVELLE CLASSE
    Classe Creer_Classe(const string& Nom, Enseignant& enseignant ) {
        string query = "INSERT INTO classes (nom, enseignant_id) VALUES ('" + Nom + "') RETURNING classe_id;";
        PGresult* res = DB.executeQuery(query);

        int classe_id = atoi(PQgetvalue(res, 0, 0));
        PQclear(res);

        return Classe(classe_id, Nom);
    }

    // CREER UN NOUVEAU ETUDIANT
    Etudiant Creer_Etudiant(const string& Nom, const string& Email, const string& Mdp, const Classe& classe, const Parent& parent, const Date& DateNaissance) {
        // Convertir la date en un format SQL-compatible
        string dateNaissanceStr = to_string(DateNaissance.annee) + "-" +
            (DateNaissance.mois < 10 ? "0" : "") + to_string(DateNaissance.mois) + "-" +
            (DateNaissance.jour < 10 ? "0" : "") + to_string(DateNaissance.jour);

        string query = "INSERT INTO etudiants (nom, email, mdp, classe_id, parent_id, date_naissance) VALUES ('"
            + Nom + "', '"
            + Email + "', '"
            + Mdp + "', '"
            + to_string(classe.GetClasseId()) + ", "
            + to_string(parent.GetParentId()) + ", "
            + dateNaissanceStr +
            ") RETURNING etudiant_id;";

        PGresult* res = DB.executeQuery(query);
        int etudiant_id = atoi(PQgetvalue(res, 0, 0));
        PQclear(res);

        return Etudiant(etudiant_id, Nom, Email, Mdp, DateNaissance, parent, classe);
    }

    // ASSIGNER UN ENSEIGNANT A UNE CLASSE
    void Assigner_Enseignant_a_Classe(int ID, Enseignant& enseignant) {
        // mettre à jour l'enseignant dans la classe
        string query = "UPDATE classes SET enseignant_id = " + to_string(enseignant.GetUtilisateurId()) +
            " WHERE classe_id = " + to_string(ID) + ";";
        // Exécuter la requête
        PGresult* res = DB.executeQuery(query);

        // Vérification du succès de l'opération
        if (PQresultStatus(res) == PGRES_COMMAND_OK) {
            cout << "Enseignant assigne avec succes a la classe." << endl;
        }
        else {
            cerr << "Erreur lors de l'assignation de l'enseignant a la classe : " << PQerrorMessage(DB.getConnection()) << endl;
        }

        PQclear(res);
    }


};

// Fonction main
int main() {
    // Créer un objet Database pour établir automatiquement la connexion
    Database DB;

    return 0; // La connexion est automatiquement fermée lorsque l'objet Database est détruit
}
