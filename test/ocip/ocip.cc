#include "global.h"

#include "oracleObject.h"

#if defined(_MSC_VER)
# include <tchar.h>
#endif

///////////////////////////////////////////////////////////////////////////
static const std::string	username	= "sample";
static const std::string	password	= "sample";
static const std::string	database	= "(DESCRIPTION=(ADDRESS_LIST=(ADDRESS=(PROTOCOL=TCP)(Host=localhost)(Port=1521)))(CONNECT_DATA=(SID=TEST)))";
static const bool			debug		= false;

///////////////////////////////////////////////////////////////////////////
static void testInteger(ocip::Connection* connection);
static void testStringInput(ocip::Connection* connection);
static void testStringOutput(ocip::Connection* connection);
static void testIntegerArray(ocip::Connection* connection);
static void testStringArray(ocip::Connection* connection);

///////////////////////////////////////////////////////////////////////////
#if defined(_MSC_VER)
int _tmain(int argc, _TCHAR* argv[])
#else
int main()
#endif
{
	oracleError error;

	// Create the Oracle enviroment
	std::cout << "Create Oracle environment..." << std::endl << std::flush;
	ocip::Environment* environment = new ocip::Environment(OCI_THREADED, debug);
	assert(environment);

	// Create the connection pool
	std::cout << "Create Oracle connection pool..." << std::endl << std::flush;
	ocip::ConnectionPool* connectionPool = new ocip::ConnectionPool(environment);
	assert(connectionPool);
	if (!connectionPool->create(username, password, database, 10, 100, 10))
	{
		error = connectionPool->reportError("create connection pool", __FILE__, __LINE__);
		std::cout << error.what() << std::endl << std::flush;
		return 1;
	}

	// Create connection from pool
	std::cout << "Create connection from pool..." << std::endl << std::flush;
	ocip::Connection* connection = new ocip::Connection(connectionPool);
	assert(connection);
	if (!connection->connect(username, password))
	{
		error = connectionPool->reportError("create connection from pool", __FILE__, __LINE__);
		std::cout << error.what() << std::endl << std::flush;
		return 1;
	}

	// peform the tests
	testInteger(connection);
	testStringInput(connection);
	testStringOutput(connection);
	testIntegerArray(connection);
	testStringArray(connection);

	// Disconnect from the connection pool
	std::cout << "Disconnect from the connection pool..." << std::endl << std::flush;
	if (!connection->disconnect())
	{
		error = connection->reportError("disconnect from the connection pool", __FILE__, __LINE__);
		return 1;
	}
	delete connection;
	connection = 0;

	// Destroy connection pool
	std::cout << "Destroy the connection pool..." << std::endl << std::flush;
	if (!connectionPool->destroy())
	{
		error = connection->reportError("destroy the connection pool", __FILE__, __LINE__);
		return 1;
	}
	delete connectionPool;
	connectionPool = 0;

	// Destroy environment
	std::cout << "Destroy the Oracle environment..." << std::endl << std::flush;
	delete environment;
	environment = 0;

	return 0;
}

///////////////////////////////////////////////////////////////////////////
void testInteger(ocip::Connection* connection)
{
	std::cout << "START: testInteger" << std::endl << std::flush;

	oracleError error;

	// Prepare statement
	std::cout << "Prepare statement..." << std::endl << std::flush;
	ocip::Statement statement(connection);
	if (!statement.prepare("BEGIN :p1 := :p1 + 1; END;"))
	{
		error = statement.reportError("oci_statement_prepare", __FILE__, __LINE__);
		std::cout << error.what() << std::endl << std::flush;
		return;
	}

	ocip::ParameterValue* b1 = new ocip::ParameterValue("p1", ocip::Integer, ocip::Output);
	statement.addParameter(b1);
	b1->value(10);

	// Execute statement
	std::cout << "Execute statement..." << std::endl << std::flush;
	if (!statement.execute(1))
	{
		error = statement.reportError("oci_statement_execute", __FILE__, __LINE__);
		std::cout << error.what() << std::endl << std::flush;
		return;
	}

	int p1 = b1->getInteger();
	std::cout << p1 << std::endl << std::flush;
	assert(p1 == 11);

	std::cout << "END: testInteger" << std::endl << std::flush;
}

///////////////////////////////////////////////////////////////////////////
void testStringInput(ocip::Connection* connection)
{
	std::cout << "START: testStringInput" << std::endl << std::flush;

	oracleError error;

	// Prepare statement
	std::cout << "Prepare statement..." << std::endl << std::flush;
	ocip::Statement statement(connection);
	if (!statement.prepare("BEGIN IF (:p1 <> '1') THEN RAISE VALUE_ERROR; END IF; END;"))
	{
		error = statement.reportError("oci_statement_prepare", __FILE__, __LINE__);
		std::cout << error.what() << std::endl << std::flush;
		return;
	}

	ocip::ParameterValue* b1 = new ocip::ParameterValue("p1", ocip::String, ocip::Output, 2000);
	statement.addParameter(b1);
	b1->value("1");

	// Execute statement
	std::cout << "Execute statement..." << std::endl << std::flush;
	if (!statement.execute(1))
	{
		error = statement.reportError("oci_statement_execute", __FILE__, __LINE__);
		std::cout << error.what() << std::endl << std::flush;
		return;
	}

	std::cout << "END: testStringInput" << std::endl << std::flush;
}

///////////////////////////////////////////////////////////////////////////
void testStringOutput(ocip::Connection* connection)
{
	std::cout << "START: testStringOutput" << std::endl << std::flush;

	oracleError error;

	// Prepare statement
	std::cout << "Prepare statement..." << std::endl << std::flush;
	ocip::Statement statement(connection);
	if (!statement.prepare("BEGIN :p1 := '(' || :p1 || ')'; END;"))
	{
		error = statement.reportError("oci_statement_prepare", __FILE__, __LINE__);
		std::cout << error.what() << std::endl << std::flush;
		return;
	}

	ocip::ParameterValue* b1 = new ocip::ParameterValue("p1", ocip::String, ocip::Output, 2000);
	statement.addParameter(b1);
	b1->value("1");

	// Execute statement
	std::cout << "Execute statement..." << std::endl << std::flush;
	if (!statement.execute(1))
	{
		error = statement.reportError("oci_statement_execute", __FILE__, __LINE__);
		std::cout << error.what() << std::endl << std::flush;
		return;
	}

	std::string p1 = b1->getString();
	std::cout << p1 << std::endl << std::flush;
	assert(p1 == "(1)");

	std::cout << "END: testStringOutput" << std::endl << std::flush;
}

///////////////////////////////////////////////////////////////////////////
void testIntegerArray(ocip::Connection* connection)
{
	std::cout << "START: testIntegerArray" << std::endl << std::flush;

	oracleError error;

	// Prepare statement
	std::cout << "Prepare statement..." << std::endl << std::flush;
	ocip::Statement statement(connection);
	if (!statement.prepare(
"DECLARE\n"
"   TYPE num_array_type IS TABLE OF NUMBER INDEX BY BINARY_INTEGER;\n"
"   a num_array_type;\n"
"BEGIN\n"
"   a := :p1;\n"
"   FOR i IN 1 .. a.COUNT LOOP\n"
"      a(i) := a(i) * 2;\n"
"   END LOOP;\n"
"   :p1 := a;\n"
"END;"))
	{
		error = statement.reportError("oci_statement_prepare", __FILE__, __LINE__);
		std::cout << error.what() << std::endl << std::flush;
		return;
	}

	std::list<long> list;
	list.push_back(10);
	list.push_back(20);
	list.push_back(30);
	ocip::ParameterArray* b1 = new ocip::ParameterArray("p1", ocip::Integer, ocip::Output, list.size());
	statement.addParameter(b1);
	b1->value(list);

	// Execute statement
	std::cout << "Execute statement..." << std::endl << std::flush;
	if (!statement.execute(1))
	{
		error = statement.reportError("oci_statement_execute", __FILE__, __LINE__);
		std::cout << error.what() << std::endl << std::flush;
		return;
	}

	list = b1->getInteger();
	std::list<long>::iterator it;
	int total = 0;
	for (it = list.begin(); it != list.end(); ++it)
	{
		std::cout << *it << std::endl << std::flush;
		total += *it;
	}
	assert(total == 120);

	std::cout << "END: testIntegerArray" << std::endl << std::flush;
}

///////////////////////////////////////////////////////////////////////////
void testStringArray(ocip::Connection* connection)
{
	std::cout << "START: testStringArray" << std::endl << std::flush;

	oracleError error;

	// Prepare statement
	std::cout << "Prepare statement..." << std::endl << std::flush;
	ocip::Statement statement(connection);
	if (!statement.prepare(
"DECLARE\n"
"   TYPE vc_array_type IS TABLE OF VARCHAR2(2000) INDEX BY BINARY_INTEGER;\n"
"   a vc_array_type;\n"
"BEGIN\n"
"   a := :p1;\n"
"   FOR i IN 1 .. a.COUNT LOOP\n"
"      a(i) := '(' || a(i) || ')';\n"
"   END LOOP;\n"
"   :p1 := a;\n"
"END;"))
	{
		error = statement.reportError("oci_statement_prepare", __FILE__, __LINE__);
		std::cout << error.what() << std::endl << std::flush;
		return;
	}

	ocip::ParameterArray* b1 = new ocip::ParameterArray("p1", ocip::String, ocip::Output, 2000);
	statement.addParameter(b1);
	std::list<std::string> list;
	list.push_back("a");
	list.push_back("b");
	list.push_back("c");
	b1->value(list);

	// Execute statement
	std::cout << "Execute statement..." << std::endl << std::flush;
	if (!statement.execute(1))
	{
		error = statement.reportError("oci_statement_execute", __FILE__, __LINE__);
		std::cout << error.what() << std::endl << std::flush;
		return;
	}

	list = b1->getString();
	std::list<std::string>::iterator it;
	std::string total;
	for (it = list.begin(); it != list.end(); ++it)
	{
		std::cout << *it << std::endl << std::flush;
		total += *it;
	}
	assert(total == "(a)(b)(c)");

	std::cout << "END: testStringArray" << std::endl << std::flush;
}
