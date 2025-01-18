#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <libpq-fe.h>  // PostgreSQL library
#include <cstdlib>     // For getenv()
#include <string>
#include <vector>

using namespace std;

//// Forward Declarations
//class Matiere;
//class Classe;
//class Parent;

// Database Class
class Database {
private:
    PGconn* conn; // Pointer for PostgreSQL connection

public:
    Database() {
        // Fetch environment variables
        string dbName = getenv("PG_DB_NAME");
        string dbUser = getenv("PG_DB_USER");
        string dbPassword = getenv("PG_DB_PASSWORD");
        string dbHost = getenv("PG_DB_HOST");

        // Connection string for PostgreSQL
        string connectionString = "dbname=" + dbName + " user=" + dbUser + " password=" + dbPassword + " hostaddr=" + dbHost + " port=5000";
        conn = PQconnectdb(connectionString.c_str());

        if (PQstatus(conn) == CONNECTION_BAD) {
            cerr << "Connection to the database failed: " << PQerrorMessage(conn) << endl;
        }
        else {
            cout << "Connected to the PostgreSQL database successfully." << endl;
        }
    }

    ~Database() {
        PQfinish(conn); // Close the connection when object is destroyed
    }

    PGconn* getConnection() {
        return conn;
    }

    PGresult* executeQuery(const string& query) {
        PGresult* res = PQexec(conn, query.c_str());
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            cerr << "Query execution failed: " << PQerrorMessage(conn) << endl;
        }
        return res;
    }

    void Ajouter_Admin(const string& Nom, const string& Email, const string& Mdp) {
        string query = "INSERT INTO admins (nom, email, mdp) VALUES ('" + Nom + "', '" + Email + "', '" + Mdp + "');";
        PGresult* res = executeQuery(query);
        if (PQresultStatus(res) == PGRES_COMMAND_OK) {
            cout << "Admin added successfully." << endl;
        }
        PQclear(res);
    }

    void Supprimer_Admin(int ID) {
        string query = "DELETE FROM admins WHERE admin_id = " + to_string(ID) + ";";
        PGresult* res = executeQuery(query);
        if (PQresultStatus(res) == PGRES_COMMAND_OK) {
            cout << "Admin deleted successfully." << endl;
        }
        else {
            cerr << "Failed to delete admin: " << PQerrorMessage(conn) << endl;
        }
        PQclear(res);
    }
};

// Other Classes
class Matiere {
private:
    int Matiere_id;
    string Nom;

public:
    Matiere(int id, const string& nom) : Matiere_id(id), Nom(nom) {}

    void Affichage() const {
        cout << "Matiere ID: " << Matiere_id << ", Nom: " << Nom << endl;
    }
};

class Classe {
private:
    int Classe_id;
    string Nom;
    vector<Matiere> Matieres;

public:
    Classe(int id, const string& nom) : Classe_id(id), Nom(nom) {}

    void Affichage() const {
        cout << "Classe ID: " << Classe_id << ", Nom: " << Nom << endl;
        for (const auto& matiere : Matieres) {
            matiere.Affichage();
        }
    }
};

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

class Parent : public Utilisateur {
public:
    Parent(int id, const string& nom, const string& email, const string& mdp)
        : Utilisateur(id, nom, email, mdp) {
    }

    void Affichage() const override {
        cout << "Parent ID: " << GetUtilisateurId() << ", Nom: " << Nom << ", Email: " << Email << endl;
    }
};

class Enseignant : public Utilisateur {
private:
    Matiere Matiere_assignee;

public:
    Enseignant(int id, const string& nom, const string& email, const string& mdp, const Matiere& matiere)
        : Utilisateur(id, nom, email, mdp), Matiere_assignee(matiere) {
    }

    void Affichage() const override {
        cout << "Enseignant ID: " << GetUtilisateurId() << ", Nom: " << Nom << ", Email: " << Email << endl;
        Matiere_assignee.Affichage();
    }
};

struct Date {
    int jour, mois, annee;
};

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
        cout << "Etudiant ID: " << GetUtilisateurId() << ", Nom: " << Nom << ", Email: " << Email << endl;
        cout << "Date de naissance: " << Date_Naissance.jour << "/" << Date_Naissance.mois << "/" << Date_Naissance.annee << endl;
        Parent_Etudiant.Affichage();
        for (const auto& classe : Classes) {
            classe.Affichage();
        }
    }
};

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
        cout << "Examen ID: " << Examen_id << endl;
        cout << "Titre: " << Titre << endl;
        cout << "Description: " << Description << endl;
        cout << "Date de l'examen: " << Date_examen.jour << "/" << Date_examen.mois << "/" << Date_examen.annee << endl;
        for (const auto& classe : Classes) {
            classe.Affichage();
        }
    }
};

// Main Function
int main() {
    // Create a Database object to establish the connection automatically
    Database DB;

    // Adding an Admin
    string adminNom, adminEmail, adminMdp;
    cout << "Entrez le nom de l'admin: ";
    cin >> adminNom;
    cout << "Entrez l'email de l'admin: ";
    cin >> adminEmail;
    cout << "Entrez le mot de passe de l'admin: ";
    cin >> adminMdp;

    DB.Ajouter_Admin(adminNom, adminEmail, adminMdp);

    // Deleting an Admin
    /*int adminID;
    cout << "Entrez l'ID de l'admin à supprimer: ";
    cin >> adminID;

    DB.Supprimer_Admin(adminID);*/

    return 0; // Connection is automatically closed when the Database object is destroyed
}
