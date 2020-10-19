/**
 * DB Insert Functions
 */

#include "insert.hpp"
#include "config.hpp"
#include <ctime>
#include <iostream>
#include <pqxx/pqxx>

/**
 *  Function: insert_reading
 *  Description:
 *    Handle writing device readings to DB
 */
void insert_reading(appConfig *config, const char *location, const char *device_type, const char *device_id, const char *sensor, int reading)
{
	// Mark insert with a timestamp
	long int ts = static_cast<long int> (std::time(0));
	cout << "DEBUG [insert] Prepare readings for location: " << location << ", device_type: " << device_type << ", device_id: " << device_id << ", sensor: " << sensor << ", ts: " << ts << ", reading: " << reading << endl;

	try
	{
		// Create new PG connection
		pqxx::connection connection{config->pg_connection};

		// Prepare Reading
		std::string sql = "INSERT INTO readings(location, device_type, device_id, sensor, ts, reading) VALUES ($1, $2, $3, $4, to_timestamp($5), $6)";
		connection.prepare("readings_insert", sql);

		// Perform transaction
		pqxx::work transaction{connection};
		transaction.exec_prepared("readings_insert", location, device_type, device_id, sensor, ts, reading);
		transaction.commit();
	}
	catch (pqxx::sql_error const &e)
	{
		// Handle SQL errors
		std::cerr << "ERROR [insert] SQL: " << e.what() << std::endl;
		return;
	}
	catch (std::exception const &e)
	{
		// Handle other errors
		std::cerr << "ERROR [insert] Other: " << e.what() << std::endl;
		return;
	}
}
