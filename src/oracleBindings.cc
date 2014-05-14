#include "global.h"

#include <node.h>

#include "oracleObject.h"
#include "nodeUtilities.h"

///////////////////////////////////////////////////////////////////////////
class OracleObject;
class Config;

///////////////////////////////////////////////////////////////////////////
class OracleBindings : public node::ObjectWrap
{
public:
	static void Init(v8::Handle<v8::Object> target);

private:
	// Constructor/Destructor
	OracleBindings(const Config& config);
	~OracleBindings();

	// member variables
	OracleObject*	itsOracleObject;

	// Function exported to node
	static v8::Handle<v8::Value> New(const v8::Arguments& args);
	static v8::Handle<v8::Value> prepare(const v8::Arguments& args);
	static v8::Handle<v8::Value> cleanup(const v8::Arguments& args);

	static v8::Handle<v8::Value> connect(const v8::Arguments& args);
	static v8::Handle<v8::Value> disconnect(const v8::Arguments& args);

	static v8::Handle<v8::Value> request(const v8::Arguments& args);

	// Helper
	static std::string requestParseArguments(const v8::Arguments& args, std::string* procedure, propertyListType* parameters, propertyListType* cgi);
	static std::string getConfig(const v8::Arguments& args, Config* config);
	static inline OracleBindings* getObject(const v8::Arguments& args);
};

///////////////////////////////////////////////////////////////////////////
void init(v8::Handle<v8::Object> exports)
{
	OracleBindings::Init(exports);
}

NODE_MODULE(oracleBindings, init)

///////////////////////////////////////////////////////////////////////////
using namespace v8;

///////////////////////////////////////////////////////////////////////////
OracleBindings::OracleBindings(const Config& config)
	:	itsOracleObject(0)
{
	itsOracleObject = new OracleObject(config);
}

///////////////////////////////////////////////////////////////////////////
OracleBindings::~OracleBindings()
{
	delete itsOracleObject;
}

///////////////////////////////////////////////////////////////////////////
void OracleBindings::Init(Handle<Object> target)
{
	// Prepare constructor template
	Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
	tpl->SetClassName(String::NewSymbol("OracleBindings"));
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	// Prototype
	tpl->PrototypeTemplate()->Set(String::NewSymbol("prepare"),				FunctionTemplate::New(prepare)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("cleanup"),				FunctionTemplate::New(cleanup)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("connect"),				FunctionTemplate::New(connect)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("disconnect"),			FunctionTemplate::New(disconnect)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("request"),				FunctionTemplate::New(request)->GetFunction());

	Persistent<Function> constructor = Persistent<Function>::New(tpl->GetFunction());
	target->Set(String::NewSymbol("OracleBindings"), constructor);
}

///////////////////////////////////////////////////////////////////////////
Handle<Value> OracleBindings::New(const Arguments& args)
{
	HandleScope scope;

	// Load the configuration object
	Config config;
	std::string error = getConfig(args, &config);
	if (!error.empty())
	{
		nodeUtilities::ThrowTypeError(error);
		return scope.Close(Undefined());
	}

	if (config.isDebug)
	{
		config.debug();
	}

	// Create the object and warp it
	OracleBindings* obj = new OracleBindings(config);
	obj->Wrap(args.This());

	return args.This();
}

///////////////////////////////////////////////////////////////////////////
Handle<Value> OracleBindings::prepare(const Arguments& args)
{
	HandleScope scope;
	//OracleBindings* obj = getObject(args);

	return scope.Close(Undefined());
}

///////////////////////////////////////////////////////////////////////////
Handle<Value> OracleBindings::cleanup(const Arguments& args)
{
	HandleScope scope;
	//OracleBindings* obj = getObject(args);

	return scope.Close(Undefined());
}

///////////////////////////////////////////////////////////////////////////
Handle<Value> OracleBindings::connect(const Arguments& args)
{
	HandleScope scope;
	OracleBindings* obj = getObject(args);

	// Connect with Oracle server
	if (!obj->itsOracleObject->connect())
	{
		nodeUtilities::ThrowError(obj->itsOracleObject->getOracleError().what());
		return scope.Close(Undefined());
	}

	return scope.Close(Undefined());
}

///////////////////////////////////////////////////////////////////////////
Handle<Value> OracleBindings::disconnect(const Arguments& args)
{
	HandleScope scope;
	OracleBindings* obj = getObject(args);

	// Disconnect
	if (!obj->itsOracleObject->disconnect())
	{
		nodeUtilities::ThrowError(obj->itsOracleObject->getOracleError().what());
		return scope.Close(Undefined());
	}

	return scope.Close(Undefined());
}

///////////////////////////////////////////////////////////////////////////
Handle<Value> OracleBindings::request(const Arguments& args)
{
	HandleScope scope;
	OracleBindings* obj = getObject(args);

	// Check and decode the arguments
	std::string procedure;
	propertyListType parameters;
	propertyListType cgi;
	std::string error = requestParseArguments(args, &procedure, &parameters, &cgi);
	if (!error.empty())
	{
		nodeUtilities::ThrowTypeError(error);
		return scope.Close(Undefined());
	}

	//
	// 1. Initialize the request
	//

	if (!obj->itsOracleObject->requestInit(cgi))
	{
		nodeUtilities::ThrowError(obj->itsOracleObject->getOracleError().what());
		return scope.Close(Undefined());
	}

	//
	// 2. Invoke the procedure
	//

	if (!obj->itsOracleObject->requestRun(procedure, parameters))
	{
		nodeUtilities::ThrowError(obj->itsOracleObject->getOracleError().what());
		return scope.Close(Undefined());
	}

	//
	// 3. Retrieve the page content
	//

	std::wstring page;
	if (!obj->itsOracleObject->requestPage(&page))
	{
		nodeUtilities::ThrowError(obj->itsOracleObject->getOracleError().what());
		return scope.Close(Undefined());
	}

	// Return the page contents
	return scope.Close(String::New(reinterpret_cast<const uint16_t*>(page.c_str())));
}

///////////////////////////////////////////////////////////////////////////
std::string OracleBindings::requestParseArguments(const v8::Arguments& args, std::string* procedure, propertyListType* parameters, propertyListType* cgi)
{
	// Check the number and types of arguments
	if (args.Length() != 3 || !nodeUtilities::isArgString(args, 0) || !nodeUtilities::isArgObject(args, 1) || !nodeUtilities::isArgObject(args, 2))
	{
		return "Wrong number or types of arguments!";
	}

	//
	// 1) Get procedure name as first arguments
	//

	*procedure = nodeUtilities::getArgString(args, 0);
	if (procedure->empty())
	{
		return "The procedure name is not allowed to be empty!";
	}

	//
	// 2) Get the parameters of the procedure
	//

	{

	Local<Object> object;
	if (!nodeUtilities::getArgObject(args, 1, &object))
	{
		return "The second parameter must be an object!";
	}
	if (!nodeUtilities::objectAsStringLists(object, parameters))
	{
		return "The second parameter must be an object with all properties of type string!";
	}

	}

	//
	// 3) Get the CGI environment
	//

	/*
		var_name(2) := 'SERVER_NAME';
		var_name(3) := 'GATEWAY_INTERFACE';
		var_name(4) := 'REMOTE_HOST';
		var_name(5) := 'REMOTE_ADDR';
		var_name(6) := 'AUTH_TYPE';
		var_name(7) := 'REMOTE_USER';
		var_name(8) := 'REMOTE_IDENT';
		var_name(9) := 'HTTP_ACCEPT';
		var_name(10) := 'HTTP_USER_AGENT';
		var_name(11) := 'SERVER_PROTOCOL';
		var_name(12) := 'SERVER_PORT';
		var_name(13) := 'SCRIPT_NAME';
		var_name(14) := 'PATH_INFO';
		var_name(15) := 'PATH_TRANSLATED';
		var_name(16) := 'HTTP_REFERER';
		var_name(17) := 'HTTP_COOKIE';
	*/

	{

	Local<Object> object;
	if (!nodeUtilities::getArgObject(args, 2, &object))
	{
		return "The third parameter must be an object!";
	}
	if (!nodeUtilities::objectAsStringLists(object, cgi))
	{
		return "The third parameter must be an object with all properties of type string!";
	}
	if (cgi->size() < 1)
	{
		return "The third parameter must be an object with at least one property!";
	}

	}

	return "";
}

///////////////////////////////////////////////////////////////////////////
std::string OracleBindings::getConfig(const Arguments& args, Config* config)
{
	bool retCode;

	// Check the arguments
	if (!nodeUtilities::isArgObject(args, 0))
	{
		return "The first parameter must be an object!";
	}

	// Get the configuration object
	Local<Object> object;
	retCode = nodeUtilities::getArgObject(args, 0, &object);
	if (!retCode)
	{
		return "The first parameter must be an object!";
	}

	// Check the properties
	if (!nodeUtilities::isObjString(object, "username"))
	{
		return "Object must contain a property 'username' of type string!";
	}
	if (!nodeUtilities::isObjString(object, "password"))
	{
		return "Object must contain a property 'password' of type string!";
	}
	if (!nodeUtilities::isObjBoolean(object, "sysdba"))
	{
		return "Object must contain a property 'sysdba' of type boolean!";
	}
	if (!nodeUtilities::isObjString(object, "hostname"))
	{
		return "Object must contain a property 'hostname' of type string!";
	}
	if (!nodeUtilities::isObjInteger(object, "port"))
	{
		return "Object must contain a property 'port' of type number!";
	}
	if (!nodeUtilities::isObjString(object, "database"))
	{
		return "Object must contain a property 'database' of type string!";
	}
	if (!nodeUtilities::isObjBoolean(object, "debug"))
	{
		return "Object must contain a property 'debug' of type boolean!";
	}

	// Get the properties
	config->itsUsername	= nodeUtilities::getObjString(object,	"username");
	config->itsPassword	= nodeUtilities::getObjString(object,	"password");
	config->isSYSDBA	= nodeUtilities::getObjBoolean(object,	"sysdba");
	config->itsHostname	= nodeUtilities::getObjString(object,	"hostname");
	config->itsPort		= nodeUtilities::getObjInteger(object,	"port");
	config->itsDatabase	= nodeUtilities::getObjString(object,	"database");
	config->isDebug		= nodeUtilities::getObjBoolean(object,	"debug");

	return "";
}

///////////////////////////////////////////////////////////////////////////
inline OracleBindings* OracleBindings::getObject(const Arguments& args)
{
	return ObjectWrap::Unwrap<OracleBindings>(args.This());
}
