
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <libpq-fe.h>  // PostgreSQL library
#include <cstdlib>  // For getenv()
#include <string>

using namespace std;

class Database {
private:
    PGconn* conn; // D�clarer le pointeur pour la connexion avec PostgresQL.

public:
    // Constructeur pour �tablie la connexion avec la base de donn�e.
    Database() {
        // Importer les variables d'environements cr�es.
        string dbName = getenv("PG_DB_NAME");
        string dbUser = getenv("PG_DB_USER");
        string dbPassword = getenv("PG_DB_PASSWORD");
        string dbHost = getenv("PG_DB_HOST");

        // String de Connection a la Base de donn�es PostgresQL.
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

    // Destructeur pour arreter la connexion avec la base de donn�e.
    ~Database() {
        PQfinish(conn); // Arreter la connexion quand l'objet est d�truit.
    }

    // Getter pour acc�der au pointeur de connexion.
    PGconn* getConnexion() {
        return conn;
    }
};

int main()
{
    // Cr�er un Objet de la classe Database qui va �tablir la connexion automatiquement.
    Database db;

    return 0; // La connexion est automatiquement arret�e grace au destructeur quand le code est ex�cut� et termin� et l'Objet est d�truit. 
}

