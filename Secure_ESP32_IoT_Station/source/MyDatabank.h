#ifndef MY_DATABANK_H
#define MY_DATABANK_H

#include <Arduino.h>
#include <sqlite3.h> // Falls Sie eine SQLite-Bibliothek nutzen

// Globale Variablen für andere Dateien sichtbar machen
extern sqlite3 *db1;
extern char *zErrMsg;
extern int rc;

// Funktionen deklarieren
int callback(void *data, int argc, char **argv, char **azColName);
static int extractStringCallback(void *data, int argc, char **argv, char **azColName);
int openDb(const char *filename, sqlite3 **db);
int db_exec(sqlite3 *db, const char *sql);
int db_exec_fetch(sqlite3 *db, const char *sql, String &outResult);
void log(const char* time, const char* IP, const char* person,const char* command); // "log" umbenannt wegen Namenskonflikten
void LoginDB(const char* ID, String &outKey);
#endif