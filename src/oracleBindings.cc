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
	static void Init(v8::Handle<v8::Object> exports);

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

	static v8::Persistent<v8::Function> constructor;

	static void doRequest(uv_work_t* req);
	static void doRequestAfter(uv_work_t* req, int status);

	// Helper
	static std::string requestParseArguments(_NAN_METHOD_ARGS_TYPE args, std::string* username, std::string* password, std::string* procedure, propertyListType* parameters, propertyListType* cgi, fileListType* files, std::string* doctablename, v8::Local<v8::Function>* cb);
	static std::string getConfig(_NAN_METHOD_ARGS_TYPE args, Config* config);
	static inline OracleBindings* getObject(_NAN_METHOD_ARGS_TYPE args);
};

///////////////////////////////////////////////////////////////////////////
v8::Persistent<v8::Function> OracleBindings::constructor;

///////////////////////////////////////////////////////////////////////////
class RequestWorker : public NanAsyncWorker
{
public:
	RequestWorker(NanCallback* callback, OracleObject* oracleObject, const std::string& username, const std::string& password, const std::string& procedure,  const propertyListType& parameters, const propertyListType& cgi, const fileListType& files, const std::string& doctablename);
	~RequestWorker();

	// Executed inside the worker-thread.
	// It is not safe to access V8, or V8 data structures here, so everything we need for input and output should go on `this`.
	void Execute();

	// Executed when the async work is complete.
	// This function will be run inside the main event loop so it is safe to use V8 again
	void HandleOKCallback();

private:
	// Input parameter
	OracleObject*					m_oracleObject;
	std::string						m_username;
	std::string						m_password;
	std::string						m_procedure;
	propertyListType				m_parameters;
	propertyListType				m_cgi;
	fileListType					m_files;
	std::string						m_doctablename;

	// Results
	std::wstring					m_page;
	std::string						m_error;
};

///////////////////////////////////////////////////////////////////////////
RequestWorker::RequestWorker(NanCallback* callback, OracleObject* oracleObject, const std::string& username, const std::string& password, const std::string& procedure,  const propertyListType& parameters, const propertyListType& cgi, const fileListType& files, const std::string& doctablename)
	:	NanAsyncWorker(callback)
	,	m_oracleObject(oracleObject)
	,	m_username(username)
	,	m_password(password)
	,	m_procedure(procedure)
	,	m_parameters(parameters)
	,	m_cgi(cgi)
	,	m_files(files)
	,	m_doctablename(doctablename)
{
}

///////////////////////////////////////////////////////////////////////////
RequestWorker::~RequestWorker()
{
}

///////////////////////////////////////////////////////////////////////////
void RequestWorker::Execute()
{
	if (!m_oracleObject->request(m_username, m_password, m_cgi, m_files, m_doctablename, m_procedure, m_parameters, &m_page))
	{
		m_error = m_oracleObject->getOracleError().what();
	}
}

///////////////////////////////////////////////////////////////////////////
void RequestWorker::HandleOKCallback()
{
	NanScope();

	// Convert to UTF16
	oci_text page(m_page);

	v8::Local<v8::Value> argv[] = {
		NanNull(),
		NanNew<v8::String>(m_error.c_str()),
		NanNew<v8::String>(reinterpret_cast<const uint16_t*>(page.text()))
    };

	callback->Call(3, argv);
}

///////////////////////////////////////////////////////////////////////////
void init(v8::Handle<v8::Object> exports)
{
	OracleBindings::Init(exports);
}

NODE_MODULE(oracleBindings, init)

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
void OracleBindings::Init(v8::Handle<v8::Object> exports)
{
	// Prepare constructor template
	v8::Local<v8::FunctionTemplate> tpl = NanNew<v8::FunctionTemplate>(New);
	tpl->SetClassName(NanNew<v8::String>("OracleBindings"));
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	// Prototype
	tpl->PrototypeTemplate()->Set(NanNew<v8::String>("create"),			NanNew<v8::FunctionTemplate>(create)->GetFunction());
	tpl->PrototypeTemplate()->Set(NanNew<v8::String>("destroy"),		NanNew<v8::FunctionTemplate>(destroy)->GetFunction());
	tpl->PrototypeTemplate()->Set(NanNew<v8::String>("executeSync"),	NanNew<v8::FunctionTemplate>(executeSync)->GetFunction());
	tpl->PrototypeTemplate()->Set(NanNew<v8::String>("request"),		NanNew<v8::FunctionTemplate>(request)->GetFunction());

	NanAssignPersistent<v8::Function>(constructor, tpl->GetFunction());
	exports->Set(NanNew<v8::String>("OracleBindings"), tpl->GetFunction());
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

	if (args.IsConstructCall()) {
		// Create the object and warp it
		OracleBindings* obj = new OracleBindings(config);
		obj->Wrap(args.This());
		NanReturnValue(args.This());
	} else {
		v8::Local<v8::Function> cons = NanNew<v8::Function>(constructor);
		NanReturnValue(cons->NewInstance());
	}
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
	std::string				username;
	std::string				password;
	std::string				procedure;
	propertyListType		parameters;
	propertyListType		cgi;
	fileListType			files;
	std::string				doctablename;
	v8::Local<v8::Function>	cb;
	std::string error = requestParseArguments(args, &username, &password, &procedure, &parameters, &cgi, &files, &doctablename, &cb);
	if (!error.empty())
	{
		std::string text("OracleBindings::request: " + error);
		NanThrowTypeError(text.c_str());
		NanReturnUndefined();
	}

	NanCallback* callback = new NanCallback(cb);
	NanAsyncQueueWorker(new RequestWorker(callback, obj->itsOracleObject, username, password, procedure, parameters, cgi, files, doctablename));
	NanReturnUndefined();
}

///////////////////////////////////////////////////////////////////////////
std::string OracleBindings::requestParseArguments(_NAN_METHOD_ARGS_TYPE args, std::string* username, std::string* password, std::string* procedure, propertyListType* parameters, propertyListType* cgi, fileListType* files, std::string* doctablename, v8::Local<v8::Function>* cb)
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
		v8::Local<v8::Object> object;
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
		v8::Local<v8::Object> object;
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
		v8::Local<v8::Array> array;
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
			v8::Local<v8::Object> object = v8::Local<v8::Object>::Cast(value);

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
		*cb = v8::Local<v8::Function>::Cast(args[7]);
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
	v8::Local<v8::Object> object;
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
