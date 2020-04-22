#include "database/database_api.hpp"
#include "logger/logger_api.hpp"

CDatabase* CDatabase::dbInst = nullptr;

#define PREF "DB: "
#define DB_LOG_ERROR(fmt, ...) CLogger::getLogger()->msgToLog(log_err, PREF fmt, ##__VA_ARGS__)
#define DB_LOG_DEBUG(fmt, ...) CLogger::getLogger()->msgToLog(log_dbg, PREF fmt, ##__VA_ARGS__)
#define DB_LOG_INFO(fmt, ...) CLogger::getLogger()->msgToLog(log_info, PREF fmt, ##__VA_ARGS__)
#define DB_LOG_TRACE(fmt, ...) CLogger::getLogger()->msgToLog(log_trace, PREF fmt, ##__VA_ARGS__)

#define DB_CHECK_ERR(c, r) {\
    if(!(c)) {                      \
        return (r);                 \
    }                               \
}

#define DB_CHECK_ERR_LOG(c, r, msg, ...) {  \
    if(!(c)) {                                      \
        DB_LOG_ERROR(msg, ##__VA_ARGS__);   \
        return (r);                                 \
    }                                               \
}

static int s_db_callback(void* NotUsed, int argc, char** argv, char** azColName) {
	for (int i = 0; i < argc; i++) {
		CLogger::getLogger()->msgToLog(log_info, "%s = %s", azColName[i],
									   argv[i] ? argv[i] : "NULL");
	}
	return 0;
}

CDatabase::CDatabase(/* args */) {}

CDatabase* CDatabase::getDB() {
	if (!dbInst) {
		dbInst = new CDatabase();
		struct Eraser {
			~Eraser() { delete dbInst; };
		};
        static Eraser eraser;
	}
	return dbInst;
}

common_status_t CDatabase::openDB(const std::string& dbName) {
	DB_CHECK_ERR_LOG(sqlite3_open(dbName.c_str(), &db) == SQLITE_OK, error_unknown, "Failed to open DB");
	return cmn_success;
}

common_status_t CDatabase::writeCommand(const std::string& command) {
    char* errMsg = nullptr;
    if(sqlite3_exec(db, command.c_str(), s_db_callback, 0, &errMsg) != SQLITE_OK) {
		DB_LOG_ERROR("Failed to write command: %s", errMsg);
        sqlite3_free(errMsg);
        return error_unknown;
    }
    return cmn_success;
}

CDatabase::~CDatabase() {
    sqlite3_close(db);
	DB_LOG_TRACE("DB deleted successfuly");
}