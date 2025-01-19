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

// CLASS CLASSE
class Classe {
private:
    int Classe_id;
    string Nom;

public:
    Classe(int id, const string& nom) : Classe_id(id), Nom(nom) {}

    void Affichage() const {
        cout << "ID Classe : " << Classe_id << ", Nom : " << Nom << endl;
    }

    int GetClasseId() const {
        return Classe_id;
    }
};

// CLASS EXAMEN
class Examen {
private:
    int Examen_id;
    string Titre, Description;
    Date Date_examen;
    vector<Classe> Classes;
    vector<Matiere> Matieres;

public:
    Examen(int id, const string& titre, const string& description, const Date& date, const vector<Classe>& classes, const vector<Matiere> matieres)
        : Examen_id(id), Titre(titre), Description(description), Date_examen(date), Classes(classes), Matieres(matieres) {
    }

    void Affichage() const {
        cout << "ID Examen : " << Examen_id << endl;
        cout << "Titre : " << Titre << endl;
        cout << "Description : " << Description << endl;
        cout << "Date de l'examen : " << Date_examen.jour << "/" << Date_examen.mois << "/" << Date_examen.annee << endl;

        for (const auto& classe : Classes) {
            classe.Affichage();
        }

        for (const auto& matiere : Matieres) {
            matiere.Affichage();
        }
    }
};


// CLASS ENSEIGNANT
class Enseignant : public Utilisateur {
private:
    Matiere Matiere_assignee;
    vector<Classe> Classes;
    Database& DB;

public:
    Enseignant(int id, const string& nom, const string& email, const string& mdp, const Matiere& matiere, const vector<Classe>& classes, Database& database)
        : Utilisateur(id, nom, email, mdp), Matiere_assignee(matiere), Classes(classes), DB(database) {}

    void Affichage() const override {
        cout << "ID Enseignant : " << GetUtilisateurId() << ", Nom : " << Nom << ", Email : " << Email << endl;

        Matiere_assignee.Affichage();

        for (const auto& classe : Classes) {
            classe.Affichage();
        }
    }

    // CREER UN EXAMEN POUR UNE CLASSE
    Examen Creer_Examen(const string& titre, const string& description, const Date& date_examen, int classe_id ) {

        // Vérifier si l'enseignant est assigné à cette classe
        string checkQuery = "SELECT * FROM enseignants_classes WHERE enseignant_id = "
            + to_string(GetUtilisateurId()) + " AND classe_id = " + to_string(classe_id) + ";";

        PGresult* checkRes = DB.executeQuery(checkQuery);

        if (PQntuples(checkRes) == 0) {
            cerr << "Erreur: Vous n'êtes pas assigné à cette classe." << endl;
            PQclear(checkRes);
            // Retourner un examen vide ou lever une exception
            return Examen(0, "", "", date_examen, vector<Classe>(), vector<Matiere>());
        }
        PQclear(checkRes);

        // Formater la date pour PostgreSQL (YYYY-MM-DD)
        string dateStr = to_string(date_examen.annee) + "-" +
            (date_examen.mois < 10 ? "0" : "") + to_string(date_examen.mois) + "-" +
            (date_examen.jour < 10 ? "0" : "") + to_string(date_examen.jour);

        // Créer l'examen dans la base de données
        string query = "INSERT INTO examens (titre, description, date_examen, classe_id, matiere_id) "
            "VALUES ('" + titre + "', '" + description + "', '" + dateStr + "', " +
            to_string(classe_id) + ", " + to_string(Matiere_assignee.GetMatiereId()) + ") "
            "RETURNING examen_id;";

        PGresult* res = DB.executeQuery(query);

        if (PQresultStatus(res) != PGRES_TUPLES_OK) {
            cerr << "Erreur lors de la création de l'examen : "
                << PQerrorMessage(DB.getConnection()) << endl;
            PQclear(res);
            return Examen(0, "", "", date_examen, vector<Classe>(), vector<Matiere>());
        }

        int examen_id = atoi(PQgetvalue(res, 0, 0));
        PQclear(res);
    }



    Matiere GetMatiere() const {
        return Matiere_assignee;
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

    // CREER UNE NOUVELLE CLASSE
    Classe Creer_Classe(const string& Nom) {
        string query = "INSERT INTO classes (nom) VALUES ('" + Nom + "') RETURNING classe_id;";
        PGresult* res = DB.executeQuery(query);

        int classe_id = atoi(PQgetvalue(res, 0, 0));
        PQclear(res);

        return Classe(classe_id, Nom);
    }

    // CREER UNE NOUVELLE MATIERE
    Matiere Creer_Matiere(const string& Nom) {
        string query = "INSERT INTO matieres (nom) VALUES ('" + Nom + "') RETURNING matiere_id;";
        PGresult* res = DB.executeQuery(query);

        int matiere_id = atoi(PQgetvalue(res, 0, 0));
        PQclear(res);

        return Matiere(matiere_id, Nom);
    }

    // CREER UN PARENT
    Parent Creer_Parent(const string& Nom, const string& Email, const string& Mdp) {
        string query = "INSERT INTO parents (nom, email, mdp) VALUES ('" + Nom + "', '" + Email + "', '" + Mdp + "') RETURNING parent_id;";
        PGresult* res = DB.executeQuery(query);

        int parent_id = atoi(PQgetvalue(res, 0, 0));
        PQclear(res);

        return Parent(parent_id, Nom, Email, Mdp);
    }


    // CREER UN NOUVEAU ETUDIANT
    Etudiant Creer_Etudiant(const string& Nom, const string& Email, const string& Mdp, const Date& DateNaissance) {
        string dateNaissanceStr = to_string(DateNaissance.annee) + "-" +
            (DateNaissance.mois < 10 ? "0" : "") + to_string(DateNaissance.mois) + "-" +
            (DateNaissance.jour < 10 ? "0" : "") + to_string(DateNaissance.jour);

        string query = "INSERT INTO etudiants (nom, email, mdp, date_naissance) VALUES ('"
            + Nom + "', '" + Email + "', '" + Mdp + "', '" + dateNaissanceStr + "') RETURNING etudiant_id;";

        PGresult* res = DB.executeQuery(query); 

        int etudiant_id = atoi(PQgetvalue(res, 0, 0));
        PQclear(res);

        return Etudiant(etudiant_id, Nom, Email, Mdp, DateNaissance, Parent(0, "", "", ""), vector<Classe>());
    }
    // ASSIGNER UN PARENT A UN ETUDIANT
    void Assigner_Parent_a_Etudiant(int etudiant_id, int parent_id) {
        string query = "UPDATE etudiants SET parent_id = " + to_string(parent_id) + " WHERE etudiant_id = " + to_string(etudiant_id) + ";";
        PGresult* res = DB.executeQuery(query);
        if (PQresultStatus(res) == PGRES_COMMAND_OK) {
            cout << "Parent assigne avec succes a l'etudiant." << endl;
        }
        else {
            cerr << "Erreur lors de l'assignation du parent a l'etudiant." << endl;
        }
        PQclear(res);
    }

    // ASSIGNER UN ETUDIANT A UNE CLASSE
    void Assigner_Etudiant_a_Classe(int etudiant_id, int classe_id) {

        // Vérifier si l'étudiant est déjà dans cette classe
        string checkQuery = "SELECT * FROM etudiants_classes WHERE etudiant_id = "
            + to_string(etudiant_id) + " AND classe_id = " + to_string(classe_id) + ";";

        PGresult* checkRes = DB.executeQuery(checkQuery);

        if (PQntuples(checkRes) > 0) {
            cout << "L'etudiant est deja assigne a cette classe." << endl;
            PQclear(checkRes);
            return;
        }

        PQclear(checkRes);

        // Insérer l'étudiant dans la table de liaison avec la classe
        string query = "INSERT INTO etudiants_classes (etudiant_id, classe_id) VALUES ("
            + to_string(etudiant_id) + ", " + to_string(classe_id) + ");";

        PGresult* res = DB.executeQuery(query);

        if (PQresultStatus(res) == PGRES_COMMAND_OK) {
            cout << "L'etudiant a ete assigne a la classe avec succes." << endl;
        }
        else {
            cerr << "Erreur lors de l'assignation de l'etudiant a la classe." << endl;
        }

        PQclear(res);
    }

    //CREER UN NOUVEAU ENSEIGNANT
    Enseignant Creer_Enseignant(const string& Nom, const string& Email, const string& Mdp) {
        string query = "INSERT INTO enseignants (nom, email, mdp) VALUES ('"
            + Nom + "', '" + Email + "', '" + Mdp + "') RETURNING enseignant_id;";

        PGresult* res = DB.executeQuery(query);

        int enseignant_id = atoi(PQgetvalue(res, 0, 0));
        PQclear(res);

        return Enseignant(enseignant_id, Nom, Email, Mdp, Matiere(0, ""), vector<Classe>(), DB);
    }


    // ASSIGNER UNE MATIERE A UN ENSEIGNANT
    void Assigner_Matiere_a_Enseignant(int enseignant_id, const Matiere& matiere) {
        string query = "UPDATE enseignants SET matiere_id = " + to_string(matiere.GetMatiereId()) +
            " WHERE enseignant_id = " + to_string(enseignant_id) + ";";

        PGresult* res = DB.executeQuery(query);

        if (PQresultStatus(res) == PGRES_COMMAND_OK) {
            cout << "La matiere a ete assignee avec succes a l'enseignant." << endl;
        }
        else {
            cerr << "Erreur lors de l'assignation de la matiere : " << PQerrorMessage(DB.getConnection()) << endl;
        }

        PQclear(res);
    }

    // ASSIGNER UN ENSEIGNANT A UNE CLASSE 
    void Assigner_Enseigant_a_Classe(int enseignant_id, int classe_id) {

        // Vérifier si l'enseignant est déjà dans cette classe
        string checkQuery = "SELECT * FROM enseignants_classes WHERE enseignant_id = "
            + to_string(enseignant_id) + " AND classe_id = " + to_string(classe_id) + ";";

        PGresult* checkRes = DB.executeQuery(checkQuery);

        if (PQntuples(checkRes) > 0) {
            cout << "L'enseignant est déjà assigné à cette classe." << endl;
            PQclear(checkRes);
            return;
        }

        PQclear(checkRes);

        // Insérer l'enseignant dans la table de liaison avec la classe
        string query = "INSERT INTO enseignant_classes (enseignant_id, classe_id) VALUES ("
            + to_string(enseignant_id) + ", " + to_string(classe_id) + ");";

        PGresult* res = DB.executeQuery(query);

        if (PQresultStatus(res) == PGRES_COMMAND_OK) {
            cout << "L'enseignant a ete assigne a la classe avec succes." << endl;
        }
        else {
            cerr << "Erreur lors de l'assignation de l'enseignant a la classe." << endl;
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
