#include <filesystem>
#include "database/database_manager.hpp"
#include "database/database_api.hpp"
#include "logger/logger_api.hpp"

#define PREF "DB Manager: "
#define DB_MANAGER_LOG_ERROR(fmt, ...) CLogger::getLogger()->msgToLog(log_err, PREF fmt, ##__VA_ARGS__)
#define DB_MANAGER_LOG_DEBUG(fmt, ...) CLogger::getLogger()->msgToLog(log_dbg, PREF fmt, ##__VA_ARGS__)
#define DB_MANAGER_LOG_INFO(fmt, ...) CLogger::getLogger()->msgToLog(log_info, PREF fmt, ##__VA_ARGS__)
#define DB_MANAGER_LOG_TRACE(fmt, ...) CLogger::getLogger()->msgToLog(log_trace, PREF fmt, ##__VA_ARGS__)

#define DB_MANAGER_CHECK_ERR(c, r) {\
    if(!(c)) {                      \
        return (r);                 \
    }                               \
}

#define DB_MANAGER_CHECK_ERR_LOG(c, r, msg, ...) {  \
    if(!(c)) {                                      \
        DB_MANAGER_LOG_ERROR(msg, ##__VA_ARGS__);   \
        return (r);                                 \
    }                                               \
}

#define BUFF_SIZE 255

static const char* s_create_sensors_type_table = \
        "CREATE TABLE IF NOT EXISTS SensorTypes("\
	    "type_id INTEGER PRIMARY KEY AUTOINCREMENT,"\
  	    "type_title TEXT);";

static const char* s_create_sensors_table = \
        "CREATE TABLE IF NOT EXISTS Sensors("\
	    "sensor_id INTEGER PRIMARY KEY AUTOINCREMENT,"\
        "sensor_title TEXT NOT NULL,"\
  	    "sensor_type INTEGER NOT NULL,"\
        "sensor_value INTEGER NOT NULL,"\
        "FOREIGN KEY (sensor_type)"\
        "REFERENCES SensorTypes(type_id));";

char* sensorTypeToString(const sensor_t sensorType) {
    switch (sensorType)
    {
    case temperature_sensor:
        return "temperature";
    case humidity_sensor:
        return "humidity";
    case preasure_sensor:
        return "preasure";
    default:
        return nullptr;
    }
}

static CDatabase* s_db = CDatabase::getDB();

common_status_t DatabaseManagerN::createrDB(const std::string& dbName) {
    DB_MANAGER_LOG_TRACE("%s dbName:%s", __FUNCTION__, dbName.c_str());
    DB_MANAGER_CHECK_ERR_LOG(s_db->openDB(dbName) == cmn_success, error_unknown, "Filed to open DB");
    DB_MANAGER_CHECK_ERR_LOG(s_db->writeCommand(s_create_sensors_type_table) == cmn_success, error_unknown, "Filed to create table");
    DB_MANAGER_CHECK_ERR_LOG(s_db->writeCommand(s_create_sensors_table) == cmn_success, error_unknown, "Filed to create table");
    return cmn_success;
}

common_status_t DatabaseManagerN::addNewSensor(const std::string& sensorTitle, const sensor_t sensorType) {
    DB_MANAGER_LOG_TRACE("%s sensorName:%s sensorType:%s", __FUNCTION__, sensorTitle.c_str(), sensorTypeToString(sensorType));
    CDatabase* s_db = CDatabase::getDB();
    DB_MANAGER_CHECK_ERR_LOG(sensorTypeToString(sensorType), error_inv_arg, "Invalid sensor type");
    char cmndBuff[BUFF_SIZE] = {0};
    sprintf(cmndBuff,\
            "INSERT INTO Sensors(sensor_title, sensor_type, sensor_value)"\
            "VALUES(\"%s\", %d, 0);", sensorTitle.c_str(), sensorType);
    DB_MANAGER_CHECK_ERR_LOG(s_db->writeCommand(cmndBuff) == cmn_success, error_unknown, "Filed to insert sensor into the table");
    return cmn_success;
}

common_status_t DatabaseManagerN::removeSensor(const uint8_t id) {
    DB_MANAGER_LOG_TRACE("%s sensorId:%s", __FUNCTION__, id);
    char cmndBuff[BUFF_SIZE] = {0};
    sprintf(cmndBuff,\
            "DELETE FROM Sensors"\
            "WHERE sensor_id = %d;", id);
    DB_MANAGER_CHECK_ERR_LOG(s_db->writeCommand(cmndBuff) == cmn_success, error_unknown, "Filed to delete sensor");
    return cmn_success;
}

common_status_t DatabaseManagerN::addNewType(const std::string& typeTitle) {
    DB_MANAGER_LOG_TRACE("%s newType:%s", __FUNCTION__, typeTitle.c_str());
    char cmndBuff[BUFF_SIZE] = {0};
    sprintf(cmndBuff,\
            "INSERT INTO SensorTypes(type_title)"\
            "VALUES(\"%s\");", typeTitle.c_str());
    DB_MANAGER_CHECK_ERR_LOG(s_db->writeCommand(cmndBuff) == cmn_success, error_unknown, "Filed to insert new type into the table");
    return cmn_success;
}

common_status_t DatabaseManagerN::removeType(const uint8_t id) {
    DB_MANAGER_LOG_TRACE("%s typeId:%s", __FUNCTION__, id);
    char cmndBuff[BUFF_SIZE] = {0};
    sprintf(cmndBuff,\
            "DELETE FROM Sensors"\
            "WHERE type_id = %d;", id);
    DB_MANAGER_CHECK_ERR_LOG(s_db->writeCommand(cmndBuff) == cmn_success, error_unknown, "Filed to delete type");
    return cmn_success;
}

common_status_t DatabaseManagerN::changeSensorSettings(const uint8_t id, const std::string& title, const sensor_t type) {
    if(!sensorTypeToString(type)) {
        DB_MANAGER_LOG_ERROR("Invalid sensor type");
        return error_inv_arg;
    }
    char cmndBuff[BUFF_SIZE] = {0};
    sprintf(cmndBuff,\
            "UPDATE Sensors"\
            "SET sensor_title = \"%s\","\
            "sensor_type = %d,"\
            "WHERE sensor_id = %d;", title.c_str(), type, id);
    DB_MANAGER_CHECK_ERR_LOG(s_db->writeCommand(cmndBuff) == cmn_success, error_unknown, "Filed to update sensor's settings");
    return cmn_success;
}

common_status_t DatabaseManagerN::updateSensorValue(const uint8_t id, const int value) {
    DB_MANAGER_LOG_TRACE("%s sensorId:%s newValue: %d", __FUNCTION__, id, value);
    char cmndBuff[BUFF_SIZE] = {0};
    sprintf(cmndBuff,\
            "UPDATE Sensors"\
            "SET sensor_value = %d"\
            "WHERE sensor_id = %d;", value, id);
    DB_MANAGER_CHECK_ERR_LOG(s_db->writeCommand(cmndBuff) == cmn_success, error_unknown, "Filed to update sensor's value");
    return cmn_success;
}

common_status_t DatabaseManagerN::getSensorInfo(const uint8_t id) {
    char cmndBuff[BUFF_SIZE] = {0};
    sprintf(cmndBuff,"SELECT  sensor_title, sensor_type, sensor_value FROM Sensors WHERE sensor_id = %d;", id);
    DB_MANAGER_CHECK_ERR_LOG(s_db->writeCommand(cmndBuff) == cmn_success, error_unknown, "Filed to delete sensor");
    return cmn_success;
}