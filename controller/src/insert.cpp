#include <ctime>
#include <iostream>
#include <pqxx/pqxx>
#include <thread> 
 
void insert(char *device, char *sensor, int reading)
{
	// example: PG_CONNECT_STRING=postgresql://sample:password@localhost/sample bin/controller
	pqxx::connection connection{std::getenv("PG_CONNECT_STRING")};
	long int ts = static_cast<long int> (std::time(0));
	std::string sql = "INSERT INTO readings(device, sensor, ts, reading) VALUES ($1, $2, to_timestamp($3), $4)";
	connection.prepare("readings_insert", sql);
	try
	{
		pqxx::work transaction{connection};
		transaction.exec_prepared("readings_insert", device, sensor, ts, reading);
		transaction.commit();
	}
	catch (pqxx::sql_error const &e)
	{
		std::cerr << "SQL error: " << e.what() << std::endl;
		return;
	}
	catch (std::exception const &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return;
	}
}
