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

        ExecStatusType status = PQresultStatus(res);
        if (status == PGRES_COMMAND_OK || status == PGRES_TUPLES_OK) {
            cout << "Requete executee avec succes." << endl; // Success message
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
        // Préparer la requête SQL
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

        // Exécuter la requête
        PGresult* res = DB.executeQuery(query);

        // Vérifier les résultats
        if (PQresultStatus(res) != PGRES_TUPLES_OK) {
            cerr << "Erreur lors de l'exécution de la requête : " << PQerrorMessage(DB.getConnection()) << endl;
            PQclear(res);
            return;
        }

        int rows = PQntuples(res);
        if (rows == 0) {
            cout << "Aucun résultat trouvé pour vos enfants." << endl;
        }
        else {
            cout << "Résultats de vos enfants :" << endl;
            for (int i = 0; i < rows; ++i) {
                string enfantNom = PQgetvalue(res, i, 0);
                string examenNom = PQgetvalue(res, i, 1);
                string matiereNom = PQgetvalue(res, i, 2);
                string note = PQgetvalue(res, i, 3);

                cout << "Enfant : " << enfantNom
                    << ", Examen : " << examenNom
                    << ", Matière : " << matiereNom
                    << ", Note : " << note << endl;
            }
        }

        // Libérer les ressources
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
    Examen Creer_Examen(const string& titre, const string& description, const Date& date_examen, int classe_id ) {

        // Vérifier si l'enseignant est assigné à cette classe
        string checkQuery = "SELECT * FROM enseignants_classes WHERE enseignant_id = "
            + to_string(GetUtilisateurId()) + " AND classe_id = " + to_string(classe_id) + ";";

        PGresult* checkRes = DB.executeQuery(checkQuery);

        if (PQntuples(checkRes) == 0) {
            cerr << "Erreur: Vous n etes pas assigne a cette classe." << endl;
            PQclear(checkRes);
            // Retourner un examen vide ou lever une exception
            return Examen(0, "", "", date_examen, vector<Classe>(), vector<Matiere>());
        }
        PQclear(checkRes);

        // Formater la date de l'examen pour PostgreSQL (YYYY-MM-DD)
        string dateStr = to_string(date_examen.annee) + "-" +
            (date_examen.mois < 10 ? "0" : "") + to_string(date_examen.mois) + "-" +
            (date_examen.jour < 10 ? "0" : "") + to_string(date_examen.jour);

        // Créer l'examen dans la base de données
        string query = "WITH inserted_exam AS ("
            "INSERT INTO examens (titre, description, date_examen, duree_minutes) "
            "VALUES ('" + titre + "', '" + description + "', '" + dateStr + "', 60) "
            "RETURNING examen_id)"
            "INSERT INTO examens_classes (examen_id, classe_id) "
            "SELECT examen_id, " + to_string(classe_id) + " FROM inserted_exam;"
            "INSERT INTO examens_matieres (examen_id, matiere_id) "
            "SELECT examen_id, " + to_string(Matiere_assignee.GetMatiereId()) + " FROM inserted_exam;";

        PGresult* res = DB.executeQuery(query);

        if (PQresultStatus(res) != PGRES_TUPLES_OK) {
            cerr << "Erreur lors de la creation de l'examen : "
                << PQerrorMessage(DB.getConnection()) << endl;
            PQclear(res);
            return Examen(0, "", "", date_examen, vector<Classe>(), vector<Matiere>());
        }

        int examen_id = atoi(PQgetvalue(res, 0, 0));
        PQclear(res);
    }

    // AJOUTER UNE NOTE POUR UN ETUDIANT
    void Ajouter_Note(int etudiant_id, int examen_id, float note_val) {
        // 1. Vérifier si l'examen appartient à la matière de l'enseignant
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

        // 2. Vérifier si l'étudiant est dans une classe qui participe à l'examen
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

        // 3. Vérifier si l'étudiant est dans une classe de l'enseignant
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

        // 4. Vérifier si l'étudiant n'a pas déjà une note pour cet examen
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

        // 5. Ajouter la note si toutes les vérifications sont passées
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

        for (const auto& classe : Classes) {
            classe.Affichage();
        }
    }

    // METHODE D'AFFICHAGE POUR QUE L'ETUDIANT PUISSE VOIR TOUTEWS SES INFOS 
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
                << " - Matière: " << PQgetvalue(notesRes, i, 1)
                << " - Note: " << PQgetvalue(notesRes, i, 2) << endl;
        }
        PQclear(notesRes);

        // Calculer et afficher la moyenne générale
        string moyenneQuery =
            "SELECT COALESCE(AVG(note_val), 0) as moyenne "
            "FROM notes "
            "WHERE etudiant_id = " + to_string(GetUtilisateurId()) + ";";

        PGresult* moyenneRes = DB.executeQuery(moyenneQuery);

        cout << "\n=== Moyenne générale ===" << endl;
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
        cout << "Etudiant '" << Nom << "' cree avec succes avec l'ID: " << etudiant_id << "Ne le: " << dateNaissanceStr << endl;

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

        // Requête pour obtenir tous les étudiants d'une classe
        string query =
            "WITH MoyennesEtudiants AS ("
            "   SELECT e.etudiant_id, e.nom, "
            "          COALESCE(AVG(n.note_val), 0) as moyenne "
            "   FROM etudiants e "
            "   JOIN etudiants_classes ec ON e.etudiant_id = ec.etudiant_id "
            "   LEFT JOIN notes n ON e.etudiant_id = n.etudiant_id "
            "   WHERE ec.classe_id = " + to_string(classe_id) + " "
            "   GROUP BY e.etudiant_id, e.nom"
            ") "
            "SELECT * FROM MoyennesEtudiants ORDER BY nom;";

        PGresult* res = DB.executeQuery(query);

        if (PQresultStatus(res) == PGRES_TUPLES_OK) {
            cout << "\n=== Moyennes des etudiants de la classe ===" << endl;

            for (int i = 0; i < PQntuples(res); i++) {
                string nom = PQgetvalue(res, i, 1);
                string moyenne = PQgetvalue(res, i, 2);

                cout << "Etudiant: " << nom << " - Moyenne: "
                    << (moyenne == "0" ? "Pas de notes" : moyenne) << endl;
            }
        }
        PQclear(res);
    }
};

// Fonction main
int main() {
    // Créer un objet Database pour établir automatiquement la connexion à la Base de données
    Database DB;

    // 1. Gestion des administrateurs via la classe Database
    /*cout << "\n=== Tests des fonctionnalites Admin via Database ===" << endl;
    DB.Ajouter_Admin("Admin Principal", "admin@emsi.ma", "admin123");*/

    // 2. Création d'un Admin pour utiliser ses méthodes
    Admin admin(1, "Admin Principal", "admin@emsi.ma", "admin123", DB);

    //// Création d'une classe
    /*cout << "\nCreation d'une classe:" << endl;
    Classe classe3IIRG3 = admin.Creer_Classe("3eme annee informatique Groupe 3");
    Classe classe4IIRG4 = admin.Creer_Classe("4eme annee informatique Groupe 4");*/

    //// Création d'une matière
    cout << "\nCreation des matieres:" << endl;
    /*Matiere POO = admin.Creer_Matiere("Programmation Oriente Objet");
    Matiere JAVA = admin.Creer_Matiere("Programmation JAVA");*/

    //// Création d'un parent
    /*cout << "\nCreation d'un parent:" << endl;
    Parent parent1 = admin.Creer_Parent("Mohamed BELCADI", "mohamed.belcadi@email.com", "mohamed123");
    Parent parent2 = admin.Creer_Parent("Mariam BELCADI", "mariam.belcadi@email.com", "mariam456");*/

    //// Création d'un étudiant
    /*cout << "\nCreation d'un etudiant:" << endl;
    Date dateNaissance = { 03, 4, 2003 };
    Etudiant etudiant1 = admin.Creer_Etudiant("Oussama BELCADI", "oussama.belcadi@email.com", "oussama123", dateNaissance);
    Date dateNaissance2 = { 01, 12, 2000 };
    Etudiant etudiant2 = admin.Creer_Etudiant("Ayoub BELCADI", "ayoub.belcadi@email.com", "ayoub456", dateNaissance2);*/

    //// Assignation parent-étudiant
    int parent1_id = 1; // ID of 'Mohamed BELCADI' from the database
    int parent2_id = 2; // ID of 'Mariam BELCADI' from the database

    int etudiant1_id = 1; // ID of 'Oussama BELCADI' from the database
    int etudiant2_id = 2; // ID of 'Ayoub BELCADI' from the database

    cout << "\nAssignation parenteétudiant:" << endl;
    admin.Assigner_Parent_a_Etudiant(etudiant1_id, parent1_id);
    admin.Assigner_Parent_a_Etudiant(etudiant2_id, parent2_id);

    //// Assignation étudiant-classe
    //cout << "\nAssignation etudiant-classe:" << endl;
    //admin.Assigner_Etudiant_a_Classe(etudiant1.GetUtilisateurId(), classe3IIRG3.GetClasseId());
    //admin.Assigner_Etudiant_a_Classe(etudiant2.GetUtilisateurId(), classe4IIRG4.GetClasseId());

    //// Création d'un enseignant
    //cout << "\nCreation d'un enseignant:" << endl;
    //Enseignant enseignant1 = admin.Creer_Enseignant("Houssam BAZZA", "Houssam.bazza@emsi.ma", "houssam123");
    //Enseignant enseignant2 = admin.Creer_Enseignant("Ahmed RABHI", "prof.francais@emsi.ma", "ahmed456");

    //// Assignation matière-enseignant
    //cout << "\nAssignation matiere-enseignant:" << endl;
    //admin.Assigner_Matiere_a_Enseignant(enseignant1.GetUtilisateurId(), POO);
    //admin.Assigner_Matiere_a_Enseignant(enseignant2.GetUtilisateurId(), JAVA);

    //// Assignation enseignant-classe
    //cout << "\nAssignation enseignant-classe:" << endl;
    //admin.Assigner_Enseigant_a_Classe(enseignant1.GetUtilisateurId(), classe3IIRG3.GetClasseId());
    //admin.Assigner_Enseigant_a_Classe(enseignant2.GetUtilisateurId(), classe4IIRG4.GetClasseId());

    //// Test suppression d'admin par Database
    //cout << "\nSuppression d'un administrateur:" << endl;
    //DB.Supprimer_Admin(1);

    //// 2. Test des fonctionnalités Enseignant
    //cout << "\n=== Tests des fonctionnalités Enseignant ===" << endl;

    //// Création d'un examen par l'enseignant
    //cout << "\nCreation d'un examen:" << endl;
    //Date dateExamen1 = { 30, 01, 2025 };
    //Examen examen1 = enseignant1.Creer_Examen("Contrôle de P.O.O", "Chapitre 5: Fonctions Virutelles", dateExamen1, classe3IIRG3.GetClasseId());
    //Date dateExamen2 = { 15, 02, 2025 };
    //Examen examen2 = enseignant2.Creer_Examen("Contrôle de JAVA", "Chapitre 3: Classes", dateExamen2, classe4IIRG4.GetClasseId());

    //// Ajout d'une note
    //cout << "\nAjout d'une note:" << endl;
    //enseignant1.Ajouter_Note(etudiant1.GetUtilisateurId(), 1, 15.5); 
    //enseignant2.Ajouter_Note(etudiant2.GetUtilisateurId(), 1, 17.0); 

    //// 3. Test des fonctionnalités Parent
    //cout << "\n=== Tests des fonctionnalites Parent ===" << endl;

    //// Consultation des résultats des enfants
    //cout << "\nConsultation des resultats des enfants:" << endl;
    //parent1.voirResultatsEnfants();
    //parent2.voirResultatsEnfants();

    //// 4. Test des fonctionnalités Étudiant
    //cout << "\n=== Tests des fonctionnalités Etudiant ===" << endl;

    //// Affichage des informations de l'étudiant
    //cout << "\nAffichage des informations de l'etudiant:" << endl;
    //etudiant1.Afficher_Informations_Etudiant(DB);
    //etudiant2.Afficher_Informations_Etudiant(DB);

    //// 5. Calcul des moyennes par l'admin
    //cout << "\n=== Calcul des moyennes ===" << endl;
    //admin.Calculer_Moyennes_Etudiants(classe3IIRG3.GetClasseId());
    //admin.Calculer_Moyennes_Etudiants(classe4IIRG4.GetClasseId());

    return 0; // La connexion est automatiquement fermée lorsque l'objet Database est détruit
}
