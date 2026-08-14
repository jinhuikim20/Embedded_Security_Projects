#include "MyDatabank.h"

// Globale Variablen hier tatsächlich anlegen
sqlite3 *db1 = nullptr;
sqlite3 *db2 = nullptr;
char *zErrMsg = 0;
int rc = 0;

// Funktionen mit Inhalt füllen
const char* data = "Callback function called";
int callback(void *data, int argc, char **argv, char **azColName){
   int i;
   Serial.printf("%s: ", (const char*)data);
   for (i = 0; i<argc; i++){
       Serial.printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   Serial.printf("\n");
   return 0;
}

// Callback specifically for extracting a single String value (like DBkey)
static int extractStringCallback(void *data, int argc, char **argv, char **azColName) {
    String *outStr = (String*)data;
    if (argc > 0 && argv[0] != nullptr) {
        *outStr = String(argv[0]); // Write the retrieved database column value into the caller's String
    }
    return 0;
}

int openDb(const char *filename, sqlite3 **db) {
   int rc = sqlite3_open(filename, db);
   if (rc) {
       Serial.printf("Can't open database: %s\n", sqlite3_errmsg(*db));
       return rc;
   } else {
       Serial.printf("Opened database successfully\n");
   }
   return rc;
}

int db_exec(sqlite3 *db, const char *sql) {
   Serial.println(sql);
   long start = micros();
   int rc = sqlite3_exec(db, sql, callback, (void*)data, &zErrMsg);
   if (rc != SQLITE_OK) {
       Serial.printf("SQL error: %s\n", zErrMsg);
       sqlite3_free(zErrMsg);
   } else {
       Serial.printf("Operation done successfully\n");
   }
   Serial.print(F("Time taken:"));
   Serial.println(micros()-start);
   return rc;
}

// Custom execution helper that populates a target String variable
int db_exec_fetch(sqlite3 *db, const char *sql, String &outResult) {
   Serial.println(sql);
   long start = micros();
   outResult = ""; // Reset before execution
   
   // Pass reference to outResult as the user context pointer (void*)&outResult
   int rc = sqlite3_exec(db, sql, extractStringCallback, (void*)&outResult, &zErrMsg);
   
   if (rc != SQLITE_OK) {
       Serial.printf("SQL error: %s\n", zErrMsg);
       sqlite3_free(zErrMsg);
   } else {
       Serial.printf("Operation done successfully\n");
   }
   Serial.print(F("Time taken:"));
   Serial.println(micros()-start);
   return rc;
}

void log(const char* time, const char* IP, const char* person,const char* command){
  if (openDb("/sd/log.db", &db1))
    return;

  char sql[128];
  snprintf(sql, sizeof(sql), "INSERT INTO Control VALUES ('%s', '%s', '%s', '%s');", time, IP, person, command);

  rc = db_exec(db1, sql);

  if (rc != SQLITE_OK) {
      sqlite3_close(db1);
      return;
  }
  sqlite3_close(db1);
}


void LoginDB(const char* ID, String &outKey){
  if (openDb("/sd/user.db", &db2))
    return;

  char sql[128];
  snprintf(sql, sizeof(sql), "SELECT key FROM LoginTable WHERE ID = '%s';", ID);

  // Use the fetching function instead of standard db_exec
  rc = db_exec_fetch(db2, sql, outKey);

  sqlite3_close(db2);
}