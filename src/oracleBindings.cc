#include "global.h"

#include <node.h>

#include "oracleObject.h"
#include "nodeUtilities.h"

///////////////////////////////////////////////////////////////////////////
class OracleObject;
class Config;

///////////////////////////////////////////////////////////////////////////
class RequestHandle
{
public:
	RequestHandle() : oracleObject(0) {}

	// Input parameter
	OracleObject*					oracleObject;
	std::string						procedure;
	propertyListType				parameters;
	propertyListType				cgi;

	// Results
	std::wstring					page;
	std::string						error;

	// Callback function
	v8::Persistent<v8::Function>	callback;
};

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
	static v8::Handle<v8::Value> executeSync(const v8::Arguments& args);
	static v8::Handle<v8::Value> request(const v8::Arguments& args);

	static void doRequest(uv_work_t* req);
	static void doRequestAfter(uv_work_t* req, int status);

	// Helper
	static std::string requestParseArguments(const v8::Arguments& args, std::string* procedure, propertyListType* parameters, propertyListType* cgi, v8::Local<v8::Function>* cb = 0);
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
	tpl->PrototypeTemplate()->Set(String::NewSymbol("executeSync"),		FunctionTemplate::New(executeSync)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("request"),			FunctionTemplate::New(request)->GetFunction());

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

	if (config.m_debug)
	{
		config.debug();
	}

	// Create the object and warp it
	OracleBindings* obj = new OracleBindings(config);
	obj->Wrap(args.This());

	return args.This();
}

///////////////////////////////////////////////////////////////////////////
Handle<Value> OracleBindings::executeSync(const Arguments& args)
{
	HandleScope scope;
	OracleBindings* obj = getObject(args);

	// Get the sql statement
	std::string sql = nodeUtilities::getArgString(args, 0);
	if (sql.empty())
	{
		nodeUtilities::ThrowError("The sql statement is not allowed to be empty!");
		return scope.Close(Undefined());
	}

	// Execute the sql statement
	if (!obj->itsOracleObject->execute(sql))
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

	// Parse the arguments
	std::string			procedure;
	propertyListType	parameters;
	propertyListType	cgi;
	Local<Function>		cb;
	std::string error = requestParseArguments(args, &procedure, &parameters, &cgi, &cb);
	if (!error.empty())
	{
		nodeUtilities::ThrowTypeError("OracleBindings::request: " + error);
		return scope.Close(Undefined());
	}

	// Allocate the request type
	RequestHandle* rh = new RequestHandle;
	assert(rh);

	// Initialize the request type
	rh->oracleObject	=	obj->itsOracleObject;
	rh->procedure		=	procedure;
	rh->parameters		=	parameters;
	rh->cgi				=	cgi;
	rh->callback		=	Persistent<Function>::New(cb);

	// Invoke function on the thread pool
	uv_work_t* req = new uv_work_t();
	assert(req);
	req->data = rh;
	uv_queue_work(uv_default_loop(), req, doRequest, (uv_after_work_cb)doRequestAfter);

	return v8::Undefined();
}

///////////////////////////////////////////////////////////////////////////
// This function happens on the thread pool.
// (doing v8 things in here will make bad happen)
void OracleBindings::doRequest(uv_work_t* req)
{
	RequestHandle* rh = static_cast<RequestHandle*>(req->data);

	if (!rh->oracleObject->request(rh->cgi, rh->procedure, rh->parameters, &rh->page))
	{
		rh->error = rh->oracleObject->getOracleError().what();
	}
}

///////////////////////////////////////////////////////////////////////////
// This function happens on the thread pool.
// (doing v8 things in here will make bad happen)
void OracleBindings::doRequestAfter(uv_work_t* req, int status)
{
	RequestHandle* rh = static_cast<RequestHandle*>(req->data);

	HandleScope scope;

	// Prepare arguments
	Local<Value> argv[3];
	argv[0] = Local<Value>::New(Null());											//	error
	argv[1] = String::New(reinterpret_cast<const uint16_t*>(rh->page.c_str()));		//	page content

	// Invoke callback
	node::MakeCallback(Context::GetCurrent()->Global(), rh->callback, 2, argv);

	// Properly cleanup, or death by millions of tiny leaks
	rh->callback.Dispose();
	rh->callback.Clear();

	// Unfortunately in v0.10 Buffer::New(char*, size_t) makes a copy and we don't have the Buffer::Use() api yet
	delete rh;
	delete req;
}

///////////////////////////////////////////////////////////////////////////
std::string OracleBindings::requestParseArguments(const v8::Arguments& args, std::string* procedure, propertyListType* parameters, propertyListType* cgi, Local<Function>* cb /*= 0*/)
{
	const int NUMBER_OF_ARGUMENTS = (cb) ? 4 : 3;

	// Check the number and types of arguments
	if (args.Length() != NUMBER_OF_ARGUMENTS)
	{
		return "This function requires exactly 4 arguments!";
	}

	//
	// 1) Get procedure name as first arguments
	//

	if (!nodeUtilities::isArgString(args, 0))
	{
		return "The first argument must be a string!";
	}

	*procedure = nodeUtilities::getArgString(args, 0);

	if (procedure->empty())
	{
		return "The procedure name is not allowed to be empty!";
	}

	//
	// 2) Get the parameters of the procedure
	//

	if (!nodeUtilities::isArgObject(args, 1))
	{
		return "The second argument must be an object!";
	}
	else
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

	if (!nodeUtilities::isArgObject(args, 2))
	{
		return "The third argument must be an object!";
	}
	else
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

	//
	// 4) The optional callback function
	//

	if (cb)
	{
		if (!args[3]->IsFunction())
		{
			return "The fourth parameter must be the callback function!";
		}
		else
		{
  			*cb = Local<Function>::Cast(args[3]);
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
	if (!nodeUtilities::isObjString(object, "database"))
	{
		return "Object must contain a property 'database' of type string!";
	}
	if (!nodeUtilities::isObjBoolean(object, "debug"))
	{
		return "Object must contain a property 'debug' of type boolean!";
	}

	// Get the properties
	config->m_username	= nodeUtilities::getObjString(object,	"username");
	config->m_password	= nodeUtilities::getObjString(object,	"password");
	config->m_database	= nodeUtilities::getObjString(object,	"database");
	config->m_debug		= nodeUtilities::getObjBoolean(object,	"debug");

	return "";
}

///////////////////////////////////////////////////////////////////////////
inline OracleBindings* OracleBindings::getObject(const Arguments& args)
{
	return ObjectWrap::Unwrap<OracleBindings>(args.This());
}
