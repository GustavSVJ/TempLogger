#include "SQLHandler.h"

#include "mysql_driver.h"
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

#include <string.h>

#define SQL_SERVER "localhost:3306"
#define SQL_USER "manager"
#define SQL_PASS "Briobane"

SQLHandler::SQLHandler()
{
}


SQLHandler::~SQLHandler()
{
}

void SQLHandler::OpenConnection() {
	driver = sql::mysql::get_driver_instance();
	con = driver->connect(SQL_SERVER, SQL_USER, SQL_PASS);
}

void SQLHandler::WriteTempData(int sensorID, const char sensorDescription[], float temp) {
	stmt = con->createStatement();
	stmt->execute("USE appartment");

	char buff[100];
	memset(buff, 0, 100);
	sprintf(buff, "INSERT INTO tempData(sensorID, sensorDescription, tempData) VALUES (%d, \"%s\", %f)", sensorID, sensorDescription, temp);

	stmt->execute(buff);
	delete stmt;
}