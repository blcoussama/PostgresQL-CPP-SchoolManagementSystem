#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <libpq-fe.h>  // Biblioth�que PostgreSQL
#include <cstdlib>     // Pour getenv()
#include <string>
#include <vector>

using namespace std;

// Structure de Date
struct Date {
    int jour, mois, annee;
};

// Classe Base de Donn�es
class Database {
private:
    PGconn* conn; // Pointeur pour la connexion PostgreSQL

public:
    Database() {
        // R�cup�rer les variables d'environnement
        string dbName = getenv("PG_DB_NAME");
        string dbUser = getenv("PG_DB_USER");
        string dbPassword = getenv("PG_DB_PASSWORD");
        string dbHost = getenv("PG_DB_HOST");

        // Cha�ne de connexion pour PostgreSQL
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
        PQfinish(conn); // Fermer la connexion lorsque l'objet est d�truit
    }

    PGconn* getConnection() {
        return conn;
    }

    PGresult* executeQuery(const string& query) {
        PGresult* res = PQexec(conn, query.c_str());

        ExecStatusType status = PQresultStatus(res);
        if (status == PGRES_COMMAND_OK || status == PGRES_TUPLES_OK) {
            cout << "Requete executee avec succes." << endl;
        }
        else {
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
private:
    Database& DB;

public:
    Parent(int id, const string& nom, const string& email, const string& mdp, Database& database)
        : Utilisateur(id, nom, email, mdp), DB(database) {}

    void Affichage() const override {
        cout << "ID Parent : " << GetUtilisateurId() << ", Nom : " << Nom << ", Email : " << Email << endl;
    }

    void voirResultatsEnfants() {
        string query = R"(
            SELECT 
                et.nom AS enfant_nom, 
                ex.titre AS examen_titre, 
                mat.nom AS matiere_nom, 
                n.note_val AS note
            FROM 
                etudiants et
            INNER JOIN 
                notes n ON et.etudiant_id = n.etudiant_id
            INNER JOIN 
                examens ex ON n.examen_id = ex.examen_id
            INNER JOIN 
                examens_matieres em ON ex.examen_id = em.examen_id
            INNER JOIN 
                matieres mat ON em.matiere_id = mat.matiere_id
            WHERE 
                et.parent_id = )" + to_string(GetUtilisateurId()) + R"(
            ORDER BY 
                et.nom, ex.titre, mat.nom;
        )";

        PGresult* res = DB.executeQuery(query);

        if (PQresultStatus(res) != PGRES_TUPLES_OK) {
            cerr << "Erreur lors de l'execution de la requete : " << PQerrorMessage(DB.getConnection()) << endl;
            PQclear(res);
            return;
        }

        int rows = PQntuples(res);
        if (rows == 0) {
            cout << "Aucun resultat trouve pour vos enfants." << endl;
        }
        else {
            cout << "Resultats de(s) etudiant(s) :" << endl;
            for (int i = 0; i < rows; ++i) {
                string enfantNom = PQgetvalue(res, i, 0);
                string examenNom = PQgetvalue(res, i, 1);
                string matiereNom = PQgetvalue(res, i, 2);
                string note = PQgetvalue(res, i, 3);

                cout << "Enfant : " << enfantNom
                    << ", Examen : " << examenNom
                    << ", Matiere : " << matiereNom
                    << ", Note : " << note << endl;
            }
        }

        PQclear(res);
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

    string GetNom() const {
        return Nom;
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

    int GetExamenId() const {
        return Examen_id;
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
    Examen Creer_Examen(const string& titre, const string& description, const Date& date_examen, int classe_id) {
        // 1. Verifier si l'enseignant est assignee a la classe
        string checkQuery = "SELECT * FROM enseignants_classes WHERE enseignant_id = "
            + to_string(GetUtilisateurId()) + " AND classe_id = " + to_string(classe_id) + ";";

        PGresult* checkRes = DB.executeQuery(checkQuery);
        if (PQntuples(checkRes) == 0) {
            cerr << "Erreur: Vous n'�tes pas assign� � cette classe." << endl;
            PQclear(checkRes);
            return Examen(0, "", "", date_examen, {}, {});
        }
        PQclear(checkRes);

        // 2. date en format POSTGRESQL
        string dateStr = to_string(date_examen.annee) + "-"
            + (date_examen.mois < 10 ? "0" : "") + to_string(date_examen.mois) + "-"
            + (date_examen.jour < 10 ? "0" : "") + to_string(date_examen.jour)
            + " 00:00:00";

        // 3. Inserer dans examens
        const string insertExamQuery =
            "INSERT INTO examens (titre, description, date_examen, duree_minutes) "
            "VALUES ($1::text, $2::text, $3::timestamp, 60) RETURNING examen_id;";

        string examen_id_str;
        try {
            // Insere dans examens
            const char* examParams[3] = {
                titre.c_str(),
                description.c_str(),
                dateStr.c_str()
            };

            PGresult* examRes = PQexecParams(DB.getConnection(),
                insertExamQuery.c_str(),
                3,
                NULL,
                examParams,
                NULL,
                NULL,
                0
            );

            if (PQresultStatus(examRes) != PGRES_TUPLES_OK) {
                cerr << "Erreur cr�ation examen: " << PQerrorMessage(DB.getConnection()) << endl;
                PQclear(examRes);
                return Examen(0, "", "", date_examen, {}, {});
            }

            // Recuperer l'id genere
            examen_id_str = PQgetvalue(examRes, 0, 0);
            int examen_id = stoi(examen_id_str);
            PQclear(examRes);

            // 4. Inserer dans examens_classes
            string insertClasseQuery =
                "INSERT INTO examens_classes (examen_id, classe_id) "
                "VALUES ($1::integer, $2::integer);";

            string classe_id_str = to_string(classe_id);
            const char* classeParams[2] = {
                examen_id_str.c_str(),
                classe_id_str.c_str()
            };

            PGresult* classeRes = PQexecParams(DB.getConnection(),
                insertClasseQuery.c_str(),
                2,
                NULL,
                classeParams,
                NULL,
                NULL,
                0
            );

            if (PQresultStatus(classeRes) != PGRES_COMMAND_OK) {
                cerr << "Erreur liaison classe: " << PQerrorMessage(DB.getConnection()) << endl;
                PQclear(classeRes);
                return Examen(0, "", "", date_examen, {}, {});
            }
            PQclear(classeRes);

            // 5. Inserer dans examens_matieres
            string insertMatiereQuery =
                "INSERT INTO examens_matieres (examen_id, matiere_id) "
                "VALUES ($1::integer, $2::integer);";

            string matiere_id_str = to_string(Matiere_assignee.GetMatiereId());
            const char* matiereParams[2] = {
                examen_id_str.c_str(),
                matiere_id_str.c_str()
            };

            PGresult* matiereRes = PQexecParams(DB.getConnection(),
                insertMatiereQuery.c_str(),
                2,
                NULL,
                matiereParams,
                NULL,
                NULL,
                0
            );

            if (PQresultStatus(matiereRes) != PGRES_COMMAND_OK) {
                cerr << "Erreur liaison mati�re: " << PQerrorMessage(DB.getConnection()) << endl;
                PQclear(matiereRes);
                return Examen(0, "", "", date_examen, {}, {});
            }
            PQclear(matiereRes);

            // 6. Retrouner l'objet Examen
            vector<Classe> classes;
            classes.emplace_back(classe_id, "");
            vector<Matiere> matieres;
            matieres.push_back(Matiere_assignee);

            // Success
            string successMsg = "[SUCCESS] Examen cr��: '" + titre + "'\n"
                + "-> Mati�re: " + Matiere_assignee.GetNom() + "\n"
                + "-> Classe: ID " + to_string(classe_id) + "\n"
                + "-> Date: " + to_string(date_examen.jour) + "/"
                + to_string(date_examen.mois) + "/"
                + to_string(date_examen.annee);

            cout << successMsg << endl;

            return Examen(examen_id, titre, description, date_examen, classes, matieres);

        }
        catch (const exception& e) {
            cerr << "Erreur critique: " << e.what() << endl;
            return Examen(0, "", "", date_examen, {}, {});
        }
    }

    // AJOUTER UNE NOTE POUR UN ETUDIANT
    void Ajouter_Note(int etudiant_id, int examen_id, float note_val) {
        // 1. V�rifier si l'examen appartient � la mati�re de l'enseignant
        string checkQuery =
            "SELECT em.examen_id "
            "FROM examens_matieres em "
            "WHERE em.examen_id = " + to_string(examen_id) + " "
            "AND em.matiere_id = " + to_string(Matiere_assignee.GetMatiereId()) + ";";

        PGresult* checkRes = DB.executeQuery(checkQuery);
        if (PQntuples(checkRes) == 0) {
            cerr << "Erreur: Cet examen n appartient pas a votre matiere." << endl;
            PQclear(checkRes);
            return;
        }
        PQclear(checkRes);

        // 2. V�rifier si l'�tudiant est dans une classe qui participe � l'examen
        string checkExamClassQuery =
            "SELECT ec.etudiant_id "
            "FROM etudiants_classes ec "
            "JOIN examens_classes exc ON ec.classe_id = exc.classe_id "
            "WHERE ec.etudiant_id = " + to_string(etudiant_id) + " "
            "AND exc.examen_id = " + to_string(examen_id) + ";";

        PGresult* checkExamRes = DB.executeQuery(checkExamClassQuery);
        if (PQntuples(checkExamRes) == 0) {
            cerr << "Erreur: Cet etudiant n est pas dans une classe participant a cet examen." << endl;
            PQclear(checkExamRes);
            return;
        }
        PQclear(checkExamRes);

        // 3. V�rifier si l'�tudiant est dans une classe de l'enseignant
        string checkStudentQuery =
            "SELECT ec.etudiant_id "
            "FROM etudiants_classes ec "
            "JOIN enseignants_classes enc ON ec.classe_id = enc.classe_id "
            "WHERE ec.etudiant_id = " + to_string(etudiant_id) + " "
            "AND enc.enseignant_id = " + to_string(GetUtilisateurId()) + ";";

        PGresult* checkStudentRes = DB.executeQuery(checkStudentQuery);
        if (PQntuples(checkStudentRes) == 0) {
            cerr << "Erreur: Cet etudiant n est pas dans une de vos classes." << endl;
            PQclear(checkStudentRes);
            return;
        }
        PQclear(checkStudentRes);

        // 4. V�rifier si l'�tudiant n'a pas d�j� une note pour cet examen
        string checkNoteQuery =
            "SELECT note_id "
            "FROM notes "
            "WHERE etudiant_id = " + to_string(etudiant_id) + " "
            "AND examen_id = " + to_string(examen_id) + ";";

        PGresult* checkNoteRes = DB.executeQuery(checkNoteQuery);
        if (PQntuples(checkNoteRes) > 0) {
            cerr << "Erreur: Cet etudiant a deja une note pour cet examen." << endl;
            PQclear(checkNoteRes);
            return;
        }
        PQclear(checkNoteRes);

        // 5. Ajouter la note si toutes les v�rifications sont pass�es
        string query =
            "INSERT INTO notes (note_val, etudiant_id, examen_id) "
            "VALUES (" + to_string(note_val) + ", " +
            to_string(etudiant_id) + ", " + to_string(examen_id) + ");";

        PGresult* res = DB.executeQuery(query);
        if (PQresultStatus(res) == PGRES_COMMAND_OK) {
            cout << "Note ajoutee avec succes." << endl;
        }
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
    }

    // METHODE D'AFFICHAGE POUR QUE L'ETUDIANT PUISSE VOIR TOUTES SES INFOS 
    void Afficher_Informations_Etudiant(Database& DB) const {
        // Afficher les informations de base
        Affichage();

        // Obtenir et afficher les classes
        string classesQuery =
            "SELECT c.nom "
            "FROM classes c "
            "JOIN etudiants_classes ec ON c.classe_id = ec.classe_id "
            "WHERE ec.etudiant_id = " + to_string(GetUtilisateurId()) + ";";

        PGresult* classesRes = DB.executeQuery(classesQuery);

        cout << "\n=== Classes ===" << endl;
        for (int i = 0; i < PQntuples(classesRes); i++) {
            cout << "- " << PQgetvalue(classesRes, i, 0) << endl;
        }
        PQclear(classesRes);

        // Obtenir et afficher les notes par examen
        string notesQuery =
            "SELECT e.titre, m.nom as matiere, n.note_val "
            "FROM notes n "
            "JOIN examens e ON n.examen_id = e.examen_id "
            "JOIN examens_matieres em ON e.examen_id = em.examen_id "
            "JOIN matieres m ON em.matiere_id = m.matiere_id "
            "WHERE n.etudiant_id = " + to_string(GetUtilisateurId()) + " "
            "ORDER BY e.date_examen;";

        PGresult* notesRes = DB.executeQuery(notesQuery);

        cout << "\n=== Notes aux examens ===" << endl;
        for (int i = 0; i < PQntuples(notesRes); i++) {
            cout << "Examen: " << PQgetvalue(notesRes, i, 0)
                << " - Matiere: " << PQgetvalue(notesRes, i, 1)
                << " - Note: " << PQgetvalue(notesRes, i, 2) << endl;
        }
        PQclear(notesRes);

        // Calculer et afficher la moyenne g�n�rale
        string moyenneQuery =
            "SELECT COALESCE(ROUND(AVG(note_val)::numeric, 2), 0.00) as moyenne "
            "FROM notes "
            "WHERE etudiant_id = " + to_string(GetUtilisateurId()) + ";";

        PGresult* moyenneRes = DB.executeQuery(moyenneQuery);

        cout << "\n=== Moyenne generale ===" << endl;
        string moyenne = PQgetvalue(moyenneRes, 0, 0);
        cout << "Moyenne: " << (moyenne == "0" ? "Pas de notes" : moyenne) << endl;

        PQclear(moyenneRes);
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

        // Success
        cout << "Classe '" << Nom << "' cree avec succes avec l'ID: " << classe_id << endl;

        return Classe(classe_id, Nom);
    }

    // CREER UNE NOUVELLE MATIERE
    Matiere Creer_Matiere(const string& Nom) {
        string query = "INSERT INTO matieres (nom) VALUES ('" + Nom + "') RETURNING matiere_id;";
        PGresult* res = DB.executeQuery(query);

        int matiere_id = atoi(PQgetvalue(res, 0, 0));
        PQclear(res);

        // Success
        cout << "Matiere '" << Nom << "' cree avec succes avec l'ID: " << matiere_id << endl;

        return Matiere(matiere_id, Nom);
    }

    // CREER UN PARENT
    Parent Creer_Parent(const string& Nom, const string& Email, const string& Mdp) {
        string query = "INSERT INTO parents (nom, email, mdp) VALUES ('" + Nom + "', '" + Email + "', '" + Mdp + "') RETURNING parent_id;";
        PGresult* res = DB.executeQuery(query);

        int parent_id = atoi(PQgetvalue(res, 0, 0));
        PQclear(res);

        // Success
        cout << "Parent '" << Nom << "' cree avec succes avec l'ID: " << parent_id << endl;

        return Parent(parent_id, Nom, Email, Mdp, DB);
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

        // Success
        cout << "Etudiant '" << Nom << "' cree avec succes avec l'ID: " << etudiant_id << ", Ne le: " << dateNaissanceStr << endl;

        return Etudiant(etudiant_id, Nom, Email, Mdp, DateNaissance, Parent(0, "", "", "", DB), vector<Classe>());
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

        // V�rifier si l'�tudiant est d�j� dans cette classe
        string checkQuery = "SELECT * FROM etudiants_classes WHERE etudiant_id = "
            + to_string(etudiant_id) + " AND classe_id = " + to_string(classe_id) + ";";

        PGresult* checkRes = DB.executeQuery(checkQuery);

        if (PQntuples(checkRes) > 0) {
            cout << "L'etudiant est deja assigne a cette classe." << endl;
            PQclear(checkRes);
            return;
        }

        PQclear(checkRes);

        // Ins�rer l'�tudiant dans la table de liaison avec la classe
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

        // Success
        cout << "Enseignant '" << Nom << "' cree avec succes avec l'ID: " << enseignant_id << endl;

        return Enseignant(enseignant_id, Nom, Email, Mdp, Matiere(0, ""), vector<Classe>(), DB);
    }

    // R�cup�rer une mati�re par son Nom
    Matiere Recuperer_Matiere(const string& nom) {
        string query = "SELECT * FROM matieres WHERE nom = '" + nom + "';";
        PGresult* res = DB.executeQuery(query);

        if (PQntuples(res) > 0) {
            int matiere_id = stoi(PQgetvalue(res, 0, 0)); 
            string nom_matiere = PQgetvalue(res, 0, 1); 
            PQclear(res);
            return Matiere(matiere_id, nom_matiere); 
        }
        PQclear(res);
        throw runtime_error("Matiere non trouvee.");
    }
    // R�cup�rer une mati�re par son ID
    Matiere Recuperer_Matiere_par_ID(int matiere_id) {
        string query = "SELECT * FROM matieres WHERE matiere_id = " + to_string(matiere_id) + ";";
        PGresult* res = DB.executeQuery(query);

        if (PQntuples(res) > 0) {
            string nom_matiere = PQgetvalue(res, 0, 1); 
            PQclear(res);
            return Matiere(matiere_id, nom_matiere); 
        }

        PQclear(res);
        throw runtime_error("Matiere non trouvee.");
    }
    // R�cup�rer un enseignant par son ID
    Enseignant Recuperer_Enseignant_par_ID(int enseignant_id) {
        string query = "SELECT * FROM enseignants WHERE enseignant_id = " + to_string(enseignant_id) + ";";
        PGresult* res = DB.executeQuery(query);

        if (PQntuples(res) > 0) {
            int id = stoi(PQgetvalue(res, 0, 0)); 
            string nom = PQgetvalue(res, 0, 1);  
            string email = PQgetvalue(res, 0, 2); 
            string mdp = PQgetvalue(res, 0, 3);   
            int matiere_id = stoi(PQgetvalue(res, 0, 4)); 
            PQclear(res);

            // R�cup�rer la mati�re associ�e � cet enseignant en utilisant l'ID de la mati�re
            Matiere matiere = Recuperer_Matiere_par_ID(matiere_id); 

            return Enseignant(id, nom, email, mdp, matiere, vector<Classe>(), DB);
        }

        PQclear(res);
        throw runtime_error("Enseignant non trouve.");
    }


    // ASSIGNER UNE MATIERE A UN ENSEIGNANT
    void Assigner_Matiere_a_Enseignant(int enseignant_id, const Matiere& matiere) {
        // V�rifier si l'enseignant enseigne d�j� cette mati�re
        string checkQuery = "SELECT * FROM enseignants WHERE enseignant_id = "
            + to_string(enseignant_id) + " AND matiere_id = " + to_string(matiere.GetMatiereId()) + ";";
        PGresult* checkRes = DB.executeQuery(checkQuery);

        if (PQntuples(checkRes) > 0) {
            cout << "Cet enseignant enseigne deja cette mati�re." << endl;
            PQclear(checkRes);
            return;
        }

        PQclear(checkRes);

        // Mettre � jour la table des enseignants pour assigner la mati�re
        string query = "UPDATE enseignants SET matiere_id = " + to_string(matiere.GetMatiereId()) +
            " WHERE enseignant_id = " + to_string(enseignant_id) + ";";

        PGresult* res = DB.executeQuery(query);

        if (PQresultStatus(res) == PGRES_COMMAND_OK) {
            cout << "La matiere a ete assignee avec succes a l'enseignant." << endl;
        }
        else {
            cerr << "Erreur lors de l'assignation de la matiere a l'enseignant." << endl;
        }

        PQclear(res);
    }

    // Recuperer un parent par son ID
    Parent Recuperer_Parent_par_ID(int parent_id) {
        string query = "SELECT nom, email, mdp FROM parents WHERE parent_id = " + to_string(parent_id) + ";";
        PGresult* res = DB.executeQuery(query);

        if (PQntuples(res) > 0) {
            string nom = PQgetvalue(res, 0, 0);
            string email = PQgetvalue(res, 0, 1);
            string mdp = PQgetvalue(res, 0, 2);
            PQclear(res);
            return Parent(parent_id, nom, email, mdp, DB);
        }

        PQclear(res);
        throw runtime_error("Parent non trouve.");
    }

    // Recuperer les classe d'un etudiant par son ID
    vector<Classe> Recuperer_Classes_Etudiant(int etudiant_id) {
        vector<Classe> classes;
        string query =
            "SELECT c.classe_id, c.nom "
            "FROM classes c "
            "JOIN etudiants_classes ec ON c.classe_id = ec.classe_id "
            "WHERE ec.etudiant_id = " + to_string(etudiant_id) + ";";

        PGresult* res = DB.executeQuery(query);

        if (PQresultStatus(res) == PGRES_TUPLES_OK) {
            for (int i = 0; i < PQntuples(res); i++) {
                int classe_id = stoi(PQgetvalue(res, i, 0));
                string nom_classe = PQgetvalue(res, i, 1);
                classes.emplace_back(classe_id, nom_classe);
            }
        }

        PQclear(res);
        return classes;
    }
    // Recuperer un etudiant par son ID
    Etudiant Recuperer_Etudiant_par_ID(int etudiant_id) {
        string query =
            "SELECT nom, email, mdp, date_naissance, parent_id "
            "FROM etudiants WHERE etudiant_id = " + to_string(etudiant_id) + ";";

        PGresult* res = DB.executeQuery(query);

        if (PQntuples(res) > 0) {
            // R�cup�ration des donn�es
            string nom = PQgetvalue(res, 0, 0);
            string email = PQgetvalue(res, 0, 1);
            string mdp = PQgetvalue(res, 0, 2);
            string date_naissance = PQgetvalue(res, 0, 3);
            int parent_id = stoi(PQgetvalue(res, 0, 4));

            // Conversion de la date SQL (YYYY-MM-DD) en struct Date
            Date dateNaissance{};
            sscanf(date_naissance.c_str(), "%d-%d-%d",
                &dateNaissance.annee, &dateNaissance.mois, &dateNaissance.jour);

            // R�cup�ration du parent associ�
            Parent parent = Recuperer_Parent_par_ID(parent_id);

            // R�cup�ration des classes 
            vector<Classe> classes = Recuperer_Classes_Etudiant(etudiant_id);

            PQclear(res);
            return Etudiant(etudiant_id, nom, email, mdp, dateNaissance, parent, classes);
        }

        PQclear(res);
        throw runtime_error("Etudiant non trouve.");
    }

    // ASSIGNER UN ENSEIGNANT A UNE CLASSE 
    void Assigner_Enseigant_a_Classe(int enseignant_id, int classe_id) {

        // V�rifier si l'enseignant est d�j� dans cette classe
        string checkQuery = "SELECT * FROM enseignants_classes WHERE enseignant_id = "
            + to_string(enseignant_id) + " AND classe_id = " + to_string(classe_id) + ";";

        PGresult* checkRes = DB.executeQuery(checkQuery);

        if (PQntuples(checkRes) > 0) {
            cout << "L'enseignant est d�j� assign� � cette classe." << endl;
            PQclear(checkRes);
            return;
        }

        PQclear(checkRes);

        // Ins�rer l'enseignant dans la table de liaison avec la classe
        string query = "INSERT INTO enseignants_classes (enseignant_id, classe_id) VALUES ("
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
     
    // CALCULER LA MOYENNE DE CHAQUE ETUDIANT
    void Calculer_Moyennes_Etudiants(int classe_id) {
        // R�cup�rer le nom de la classe
        string classeQuery =
            "SELECT nom FROM classes WHERE classe_id = " + to_string(classe_id) + ";";
        PGresult* classeRes = DB.executeQuery(classeQuery);

        if (PQntuples(classeRes) == 0) {
            PQclear(classeRes);
            throw runtime_error("Classe introuvable avec l'ID " + to_string(classe_id));
        }

        string nom_classe = PQgetvalue(classeRes, 0, 0);
        PQclear(classeRes);

        // Requ�te pour les moyennes
        string query =
            "WITH MoyennesEtudiants AS ("
            "   SELECT e.nom, "
            "          COALESCE(ROUND(AVG(n.note_val)::numeric, 2), 0.00) as moyenne "
            "   FROM etudiants e "
            "   JOIN etudiants_classes ec ON e.etudiant_id = ec.etudiant_id "
            "   LEFT JOIN notes n ON e.etudiant_id = n.etudiant_id "
            "   WHERE ec.classe_id = " + to_string(classe_id) + " "
            "   GROUP BY e.etudiant_id, e.nom"
            ") "
            "SELECT * FROM MoyennesEtudiants ORDER BY nom;";

        PGresult* res = DB.executeQuery(query);

        if (PQresultStatus(res) == PGRES_TUPLES_OK) {
            cout << "\n=== Moyennes de la classe " << nom_classe << "===" << endl; 

            for (int i = 0; i < PQntuples(res); i++) {
                string nom = PQgetvalue(res, i, 0);
                string moyenne = PQgetvalue(res, i, 1);

                cout << nom << " : "
                    << (moyenne == "0.00" ? "Pas de notes" : moyenne) << endl; 
            }
        }
        PQclear(res);
    }
};

// Fonction main
int main() {
    // Cr�er un objet Database pour �tablir automatiquement la connexion � la Base de donn�es
    Database DB;

    //---------------------------------------------------------------------------------------------------------------------------------//

    // 1. Gestion des administrateurs via la classe Database
    cout << "\n=== Tests des fonctionnalites Admin via Database ===" << endl;
    DB.Ajouter_Admin("Admin Principal", "admin@emsi.ma", "admin123");

    //---------------------------------------------------------------------------------------------------------------------------------//

    // 2. Selectionner l'Admin cre� pour utiliser ses m�thodes
    Admin admin(1, "Admin Principal", "admin@emsi.ma", "admin123", DB);

    //---------------------------------------------------------------------------------------------------------------------------------//

    // Cr�ation d'une classe
    cout << "\nCreation d'une classe:" << endl;

    Classe classe1 = admin.Creer_Classe("3IIR G3");

    Classe classe2 = admin.Creer_Classe("4IIR G4");

    //---------------------------------------------------------------------------------------------------------------------------------//

    // Cr�ation d'une mati�re
    cout << "\nCreation des matieres:" << endl;

    Matiere POO = admin.Creer_Matiere("Programmation Oriente Objet");

    Matiere JAVA = admin.Creer_Matiere("Programmation JAVA");

    //---------------------------------------------------------------------------------------------------------------------------------//

    // Cr�ation d'un parent
    cout << "\nCreation d'un parent:" << endl;

    Parent parent1 = admin.Creer_Parent("Mohamed BELCADI", "mohamed.belcadi@email.com", "mohamed123");

    Parent parent2 = admin.Creer_Parent("Mariam BELCADI", "mariam.belcadi@email.com", "mariam456");

    //---------------------------------------------------------------------------------------------------------------------------------//

    // Cr�ation d'un �tudiant
    cout << "\nCreation d'un etudiant:" << endl;

    Date dateNaissance = { 03, 4, 2003 };
    Etudiant etudiant1 = admin.Creer_Etudiant("Oussama BELCADI", "oussama.belcadi@email.com", "oussama123", dateNaissance);

    Date dateNaissance2 = { 01, 12, 2000 };
    Etudiant etudiant2 = admin.Creer_Etudiant("Ayoub BELCADI", "ayoub.belcadi@email.com", "ayoub456", dateNaissance2);

    //---------------------------------------------------------------------------------------------------------------------------------//

    // Assignation parent-�tudiant
    int parent1_id = 1; 
    int parent2_id = 2; 

    int etudiant1_id = 1; 
    int etudiant2_id = 2; 

    cout << "\nAssignation parent-etudiant:" << endl;
    admin.Assigner_Parent_a_Etudiant(etudiant1_id, parent1_id);
    admin.Assigner_Parent_a_Etudiant(etudiant2_id, parent2_id);

    //---------------------------------------------------------------------------------------------------------------------------------//

    // Assignation �tudiant-classe
    int etudiant1_id = 1; 
    int etudiant2_id = 2; 

    int classe1_id = 1;
    int classe2_id = 2;

    cout << "\nAssignation etudiant-classe:" << endl;
    admin.Assigner_Etudiant_a_Classe(etudiant1_id, classe1_id);
    admin.Assigner_Etudiant_a_Classe(etudiant2_id, classe2_id);

    //---------------------------------------------------------------------------------------------------------------------------------//

    // Cr�ation d'un enseignant
    cout << "\nCreation d'un enseignant:" << endl;

    Enseignant enseignant1 = admin.Creer_Enseignant("Houssam BAZZA", "Houssam.bazza@emsi.ma", "houssam123");

    Enseignant enseignant2 = admin.Creer_Enseignant("Ahmed RABHI", "prof.francais@emsi.ma", "ahmed456");

    //---------------------------------------------------------------------------------------------------------------------------------//

    // Assignation mati�re-enseignant
    cout << "\nAssignation matiere-enseignant:" << endl;

    // R�cup�rer les mati�res existantes par leur nom
    Matiere POO = admin.Recuperer_Matiere("Programmation Oriente Objet");
    Matiere JAVA = admin.Recuperer_Matiere("Programmation JAVA");

    // R�cup�rer les enseignants existants par leur ID
    int enseignant1_id = 1;
    int enseignant2_id = 2;

    admin.Assigner_Matiere_a_Enseignant(enseignant1_id, POO);
    admin.Assigner_Matiere_a_Enseignant(enseignant2_id, JAVA);

    //---------------------------------------------------------------------------------------------------------------------------------//

    // Assignation enseignant-classe
    int classe1_id = 1;
    int classe2_id = 2;

    int enseignant1_id = 1;
    int enseignant2_id = 2;

    cout << "\nAssignation enseignant-classe:" << endl;
    admin.Assigner_Enseigant_a_Classe(enseignant1_id, classe1_id);
    admin.Assigner_Enseigant_a_Classe(enseignant2_id, classe2_id);

    //---------------------------------------------------------------------------------------------------------------------------------//

    // Test suppression d'admin par Database
    cout << "\nSuppression d'un administrateur:" << endl;
    DB.Supprimer_Admin(1);

    //---------------------------------------------------------------------------------------------------------------------------------//

    // Cr�ation d'un examen par l'enseignant pour une classe
    cout << "\nCreation d'un examen:" << endl;

    int enseignant1_id = 1;
    int enseignant2_id = 2;

    Enseignant enseignant1 = admin.Recuperer_Enseignant_par_ID(enseignant1_id);
    Enseignant enseignant2 = admin.Recuperer_Enseignant_par_ID(enseignant2_id);

    int classe1_id = 1;
    int classe2_id = 2;

    Date dateExamen1 = { 30, 1, 2025 };
    Examen examen1 = enseignant1.Creer_Examen("Controle de P.O.O", "Chapitre 5: Fonctions Virutelles", dateExamen1, classe1_id);

    Date dateExamen2 = { 15, 2, 2025 };
    Examen examen2 = enseignant2.Creer_Examen("Controle de JAVA", "Chapitre 3: Classes", dateExamen2, classe2_id);

    //---------------------------------------------------------------------------------------------------------------------------------//

    // Ajout d'une note
    cout << "\nAjout d'une note:" << endl;

    int enseignant1_id = 1;
    int enseignant2_id = 2;

    Enseignant enseignant1 = admin.Recuperer_Enseignant_par_ID(enseignant1_id);
    Enseignant enseignant2 = admin.Recuperer_Enseignant_par_ID(enseignant2_id);

    int etudiant1_id = 1;
    int etudiant2_id = 2;

    enseignant1.Ajouter_Note(etudiant1_id, 1, 15.5); 
    enseignant2.Ajouter_Note(etudiant2_id, 2, 17.0); 

    //---------------------------------------------------------------------------------------------------------------------------------//

    // Consultation des r�sultats des enfants
    cout << "\nConsultation des resultats des enfants:" << endl;

    int parent1_id = 1;
    int parent2_id = 2;

    Parent parent2 = admin.Recuperer_Parent_par_ID(parent2_id);
    Parent parent1 = admin.Recuperer_Parent_par_ID(parent1_id);

    parent1.voirResultatsEnfants();
    parent2.voirResultatsEnfants();

    //---------------------------------------------------------------------------------------------------------------------------------//

    // Affichage des informations de l'�tudiant
    cout << "\nAffichage des informations de l'etudiant:" << endl;

    int etudiant1_id = 1;
    int etudiant2_id = 2;
    
    Etudiant etudiant1 = admin.Recuperer_Etudiant_par_ID(etudiant1_id);
    Etudiant etudiant2 = admin.Recuperer_Etudiant_par_ID(etudiant2_id);

    etudiant1.Afficher_Informations_Etudiant(DB);
    etudiant2.Afficher_Informations_Etudiant(DB);

    //---------------------------------------------------------------------------------------------------------------------------------//

    // 5. Calcul des moyennes par l'admin
    cout << "\n=== Calcul des moyennes ===" << endl;

    int classe1_id = 1;
    int classe2_id = 2;

    admin.Calculer_Moyennes_Etudiants(classe1_id);
    admin.Calculer_Moyennes_Etudiants(classe2_id);
        
    //---------------------------------------------------------------------------------------------------------------------------------//

    return 0; // La connexion est automatiquement ferm�e lorsque l'objet Database est d�truit
}
