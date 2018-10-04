#pragma once

#include "mysql_driver.h"
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

class SQLHandler
{
public:
	SQLHandler();
	~SQLHandler();

	void OpenConnection();
	void WriteTempData(int sensorID, const char sensorDescription[], float Temp);

private:
	sql::Driver *driver;
	sql::Connection *con;
	sql::Statement *stmt;
};

