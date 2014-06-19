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
	std::string						username;
	std::string						password;
	std::string						procedure;
	propertyListType				parameters;
	propertyListType				cgi;
	fileListType					files;
	std::string						doctablename;

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

	static NAN_METHOD(New);
	static NAN_METHOD(create);
	static NAN_METHOD(destroy);
	static NAN_METHOD(executeSync);
	static NAN_METHOD(request);

	static void doRequest(uv_work_t* req);
	static void doRequestAfter(uv_work_t* req, int status);

	// Helper
	static std::string requestParseArguments(_NAN_METHOD_ARGS_TYPE args, std::string* username, std::string* password, std::string* procedure, propertyListType* parameters, propertyListType* cgi, fileListType* files, std::string* doctablename, v8::Local<v8::Function>* cb);
	static std::string getConfig(_NAN_METHOD_ARGS_TYPE args, Config* config);
	static inline OracleBindings* getObject(_NAN_METHOD_ARGS_TYPE args);
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
	Local<FunctionTemplate> tpl = NanNew<FunctionTemplate>(New);
	tpl->SetClassName(NanNew<String>("OracleBindings"));
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	// Prototype
	tpl->PrototypeTemplate()->Set(NanNew<String>("create"),			NanNew<FunctionTemplate>(create)->GetFunction());
	tpl->PrototypeTemplate()->Set(NanNew<String>("destroy"),		NanNew<FunctionTemplate>(destroy)->GetFunction());
	tpl->PrototypeTemplate()->Set(NanNew<String>("executeSync"),	NanNew<FunctionTemplate>(executeSync)->GetFunction());
	tpl->PrototypeTemplate()->Set(NanNew<String>("request"),		NanNew<FunctionTemplate>(request)->GetFunction());

	Persistent<Function> constructor = Persistent<Function>::New(tpl->GetFunction());
	target->Set(NanNew<String>("OracleBindings"), constructor);
}

///////////////////////////////////////////////////////////////////////////
NAN_METHOD(OracleBindings::New)
{
	NanScope();

	// Load the configuration object
	Config config;
	std::string error = getConfig(args, &config);
	if (!error.empty())
	{
		NanThrowTypeError(error.c_str());
		NanReturnUndefined();
	}

	// Create the object and warp it
	OracleBindings* obj = new OracleBindings(config);
	obj->Wrap(args.This());

	return args.This();
}

///////////////////////////////////////////////////////////////////////////
NAN_METHOD(OracleBindings::create)
{
	NanScope();
	OracleBindings* obj = getObject(args);

	if (!obj->itsOracleObject->create())
	{
		NanThrowError(obj->itsOracleObject->getOracleError().what().c_str());
	}

	NanReturnUndefined();
}

///////////////////////////////////////////////////////////////////////////
NAN_METHOD(OracleBindings::destroy)
{
	NanScope();
	OracleBindings* obj = getObject(args);

	if (!obj->itsOracleObject->destroy())
	{
		NanThrowError(obj->itsOracleObject->getOracleError().what().c_str());
	}

	NanReturnUndefined();
}

///////////////////////////////////////////////////////////////////////////
NAN_METHOD(OracleBindings::executeSync)
{
	NanScope();
	OracleBindings* obj = getObject(args);

	// Check the number and types of arguments
	if (args.Length() != 3)
	{
		NanThrowError("The function executeSync requires exactly 3 arguments!");
		NanReturnUndefined();
	}

	// Get the username
	std::string username;
	if (!nodeUtilities::getArgString(args, 0, &username) || username.empty())
	{
		NanThrowError("The parameter username must be a non-empty string!");
		NanReturnUndefined();
	}

	// Get the password
	std::string password;
	if (!nodeUtilities::getArgString(args, 1, &password))
	{
		NanThrowError("The parameter password must be a string!");
		NanReturnUndefined();
	}

	// Get the sql statement
	std::string sql;
	if (!nodeUtilities::getArgString(args, 2, &sql) || sql.empty())
	{
		NanThrowError("The parameter sql must be a non-empty string!");
		NanReturnUndefined();
	}

	// Execute the sql statement
	if (!obj->itsOracleObject->execute(username, password, sql))
	{
		NanThrowError(obj->itsOracleObject->getOracleError().what().c_str());
		NanReturnUndefined();
	}

	NanReturnUndefined();
}

///////////////////////////////////////////////////////////////////////////
NAN_METHOD(OracleBindings::request)
{
	NanScope();
	OracleBindings* obj = getObject(args);

	// Parse the arguments
	std::string			username;
	std::string			password;
	std::string			procedure;
	propertyListType	parameters;
	propertyListType	cgi;
	fileListType		files;
	std::string			doctablename;
	Local<Function>		cb;
	std::string error = requestParseArguments(args, &username, &password, &procedure, &parameters, &cgi, &files, &doctablename, &cb);
	if (!error.empty())
	{
		std::string text("OracleBindings::request: " + error);
		NanThrowTypeError(text.c_str());
		NanReturnUndefined();
	}

	// Allocate the request type
	RequestHandle* rh = new RequestHandle;
	assert(rh);

	// Initialize the request type
	rh->oracleObject	=	obj->itsOracleObject;
	rh->username		=	username;
	rh->password		=	password;
	rh->procedure		=	procedure;
	rh->parameters		=	parameters;
	rh->cgi				=	cgi;
	rh->files			=	files;
	rh->doctablename	=	doctablename;
	rh->callback		=	Persistent<Function>::New(cb);

	// Invoke function on the thread pool
	uv_work_t* req = new uv_work_t();
	assert(req);
	req->data = rh;
	uv_queue_work(uv_default_loop(), req, doRequest, (uv_after_work_cb)doRequestAfter);

	NanReturnUndefined();
}

///////////////////////////////////////////////////////////////////////////
// This function happens on the thread pool.
// (doing v8 things in here will make bad happen)
void OracleBindings::doRequest(uv_work_t* req)
{
	RequestHandle* rh = static_cast<RequestHandle*>(req->data);

	if (!rh->oracleObject->request(rh->username, rh->password, rh->cgi, rh->files, rh->doctablename, rh->procedure, rh->parameters, &rh->page))
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

	NanScope();

	// Convert to UTF16
	oci_text page(rh->page);

	// Prepare arguments
	Local<Value> argv[3];
	argv[0] = Local<Value>::New(NanNew<String>(rh->error.c_str()));											//	error
	argv[1] = String::New(reinterpret_cast<const uint16_t*>(page.text()));								//	page content

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
std::string OracleBindings::requestParseArguments(_NAN_METHOD_ARGS_TYPE args, std::string* username, std::string* password, std::string* procedure, propertyListType* parameters, propertyListType* cgi, fileListType* files, std::string* doctablename, Local<Function>* cb)
{
	// Check the number and types of arguments
	if (args.Length() != 8)
	{
		return "This function requires exactly 8 arguments! (username, password, procedure, args, cgi, files, doctablename, callback)";
	}

	//
	// 1) Get username
	//

	if (!nodeUtilities::getArgString(args, 0, username))
	{
		return "The first argument must be a string!";
	}

	//
	// 2) Get password
	//

	if (!nodeUtilities::getArgString(args, 1, password))
	{
		return "The second argument must be a string!";
	}

	//
	// 3) Get procedure name
	//

	if (!nodeUtilities::getArgString(args, 2, procedure) || procedure->empty())
	{
		return "The third argument must be a non-empty string!";
	}

	//
	// 4) Get the parameters of the procedure
	//

	if (!nodeUtilities::isArgObject(args, 3))
	{
		return "The fourth argument must be an object!";
	}
	else
	{
		Local<Object> object;
		if (!nodeUtilities::getArgObject(args, 3, &object))
		{
			return "The fourth parameter must be an object!";
		}
		if (!nodeUtilities::objectAsStringLists(object, parameters))
		{
			return "The fourth parameter must be an object with all properties of type string!";
		}
	}

	//
	// 5) Get the CGI environment
	//

	if (!nodeUtilities::isArgObject(args, 4))
	{
		return "The fifth argument must be an object!";
	}
	else
	{
		Local<Object> object;
		if (!nodeUtilities::getArgObject(args, 4, &object))
		{
			return "The fifth parameter must be an object!";
		}
		if (!nodeUtilities::objectAsStringLists(object, cgi))
		{
			return "The fifth parameter must be an object with all properties of type string!";
		}
		if (cgi->size() < 1)
		{
			return "The fifth parameter must be an object with at least one property!";
		}
	}

	//
	// 6) The array of files to upload
	//

	if (!nodeUtilities::isArgArray(args, 5))
	{
		return "The sixt argument must be an array of objects!";
	}
	else
	{
		// Get the array
		Local<Array> array;
		if (!nodeUtilities::getArgArray(args, 5, &array))
		{
			return "The sixt parameter must be an array of objects!";
		}

		// Get the number of array entries
		uint32_t length = array->Length();

		// Process the array
		for (uint32_t i = 0; i < length; i++)
		{
			// Get the file object
			v8::Local<v8::Value> value = array->Get(i);
			if (!value->IsObject())
			{
				return "The sixt parameter must be an array of objects!";
			}

			// Cast the value to an object
			v8::Local<Object> object = v8::Local<v8::Object>::Cast(value);

			// Now get the properties of the object

			// "fieldValue"
			std::string fieldValue;
			if (!nodeUtilities::getObjString(object, "fieldValue", &fieldValue) || fieldValue.empty())
			{
				return "The sixt parameter must be an array of objects with a non-empty string property \"fieldValue\"!";
			}

			// "filename"
			std::string filename;
			if (!nodeUtilities::getObjString(object, "filename", &filename) || filename.empty())
			{
				return "The sixt parameter must be an array of objects with a no9n-empty string property \"filename\"!";
			}

			// "physicalFilename"
			std::string physicalFilename;
			if (!nodeUtilities::getObjString(object, "physicalFilename", &physicalFilename) || physicalFilename.empty())
			{
				return "The sixt parameter must be an array of objects with a non-empty string property \"physicalFilename\"!";
			}

			// "encoding"
			std::string encoding;
			if (!nodeUtilities::getObjString(object, "encoding", &encoding))
			{
				return "The sixt parameter must be an array of objects with a string property \"encoding\"!";
			}

			// "mimetype"
			std::string mimetype;
			if (!nodeUtilities::getObjString(object, "mimetype", &mimetype))
			{
				return "The sixt parameter must be an array of objects with a string property \"mimetype\"!";
			}

			// Add the new file entry
			fileType file = fileType(fieldValue, filename, physicalFilename, encoding, mimetype);
			files->push_back(file);
		}
	}

	//
	// 7) Get the table name
	//

	if (!nodeUtilities::getArgString(args, 6, doctablename) || doctablename->empty())
	{
		return "The seventh argument must be a non-empty string!";
	}

	//
	// 8) The callback function
	//
	if (!args[7]->IsFunction())
	{
		return "The eight parameter must be the callback function!";
	}
	else
	{
		*cb = Local<Function>::Cast(args[7]);
	}

	return "";
}

///////////////////////////////////////////////////////////////////////////
std::string OracleBindings::getConfig(_NAN_METHOD_ARGS_TYPE args, Config* config)
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

	// Get the properties
	if (!nodeUtilities::getObjString(object, "username", &config->m_username))
	{
		return "Object must contain a property 'username' of type string!";
	}
	if (!nodeUtilities::getObjString(object, "password", &config->m_password))
	{
		return "Object must contain a property 'password' of type string!";
	}
	if (!nodeUtilities::getObjString(object, "database", &config->m_database))
	{
		return "Object must contain a property 'database' of type string!";
	}
	if (!nodeUtilities::getObjBoolean(object, "oracleConnectionPool", &config->m_conPool))
	{
		return "Object must contain a property 'oracleConnectionPool' of type boolean!";
	}
	if (!nodeUtilities::getObjBoolean(object, "oracleDebug", &config->m_debug))
	{
		return "Object must contain a property 'oracleDebug' of type boolean!";
	}

	//std::cout << "OracleBindings::getConfig" << std::endl << config->asString() << std::endl << std::flush;

	return "";
}

///////////////////////////////////////////////////////////////////////////
inline OracleBindings* OracleBindings::getObject(_NAN_METHOD_ARGS_TYPE args)
{
	return ObjectWrap::Unwrap<OracleBindings>(args.This());
}
