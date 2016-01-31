#include "global.h"

#include "nodeUtilities.h"

#include "oracleObject.h"

///////////////////////////////////////////////////////////////////////////
class OracleObject;
class Config;

///////////////////////////////////////////////////////////////////////////
class OracleBindings : public node::ObjectWrap
{
public:
	static NAN_MODULE_INIT(Init);

private:
	// Constructor/Destructor
	OracleBindings(const Config& config);
	~OracleBindings();

	// Function exported to node
	static NAN_METHOD(New);
	static NAN_METHOD(create);
	static NAN_METHOD(destroy);
	static NAN_METHOD(executeSync);
	static NAN_METHOD(request);


	static void doRequest(uv_work_t* req);
	static void doRequestAfter(uv_work_t* req, int status);

	// Helper
	static std::string getConfig(Nan::NAN_METHOD_ARGS_TYPE args, Config* config);
	static inline OracleBindings* getObject(Nan::NAN_METHOD_ARGS_TYPE args);

	// member variables
	static Nan::Persistent<v8::Function>	m_constructor;
	OracleObject*							m_oracleObject;
};

///////////////////////////////////////////////////////////////////////////
Nan::Persistent<v8::Function> OracleBindings::m_constructor;

///////////////////////////////////////////////////////////////////////////
class RequestWorker : public Nan::AsyncWorker
{
public:
	RequestWorker(Nan::Callback* callback, OracleObject* oracleObject, const std::string& username, const std::string& password, const std::string& procedure,  const parameterListType& parameters, const propertyListType& cgi, const fileListType& files, const std::string& doctablename);
	~RequestWorker();

	// Executed inside the worker-thread.
	// It is not safe to access V8, or V8 data structures here, so everything we need for input and output should go on `this`.
	void Execute();

	// Executed when the async work is complete.
	// This function will be run inside the main event loop so it is safe to use V8 again
	virtual void HandleOKCallback();

	// Callback function with an Error object wrapping the `errmsg` string
	virtual void HandleErrorCallback();

private:
	// Input parameter
	OracleObject*					m_oracleObject;
	std::string						m_username;
	std::string						m_password;
	std::string						m_procedure;
	parameterListType				m_parameters;
	propertyListType				m_cgi;
	fileListType					m_files;
	std::string						m_doctablename;

	// Results
	std::wstring					m_page;
	std::string						m_error;
};

///////////////////////////////////////////////////////////////////////////
RequestWorker::RequestWorker(Nan::Callback* callback, OracleObject* oracleObject, const std::string& username, const std::string& password, const std::string& procedure,  const parameterListType& parameters, const propertyListType& cgi, const fileListType& files, const std::string& doctablename)
	:	Nan::AsyncWorker(callback)
	,	m_oracleObject(oracleObject)
	,	m_username(username)
	,	m_password(password)
	,	m_procedure(procedure)
	,	m_parameters(parameters)
	,	m_cgi(cgi)
	,	m_files(files)
	,	m_doctablename(doctablename)
{
	if (isDebug())
	{
		std::cout << "RequestWorker::RequestWorker" << std::endl << std::flush;
	}
}

///////////////////////////////////////////////////////////////////////////
RequestWorker::~RequestWorker()
{
	if (isDebug())
	{
		std::cout << "RequestWorker::~RequestWorker" << std::endl << std::flush;
	}
}

///////////////////////////////////////////////////////////////////////////
void RequestWorker::Execute()
{
	if (isDebug())
	{
		std::cout << "RequestWorker::Execute: START" << std::endl << std::flush;
	}

	if (!m_oracleObject->request(m_username, m_password, m_cgi, m_files, m_doctablename, m_procedure, m_parameters, &m_page))
	{
		m_error = m_oracleObject->getOracleError().what();
	}

	if (isDebug())
	{
		std::cout << "RequestWorker::Execute: STOP" << std::endl << std::flush;
	}
}

///////////////////////////////////////////////////////////////////////////
void RequestWorker::HandleOKCallback()
{
	if (isDebug())
	{
		std::cout << "RequestWorker::HandleOKCallback: START" << std::endl << std::flush;
	}

	Nan::HandleScope scope;

	// Convert to UTF16
	oci_text oci_page(m_page);

	// Prepare arguments
	v8::Local<v8::Value> argv[3];
	argv[0] = Nan::New(m_error).ToLocalChecked();
	argv[1] = Nan::New(reinterpret_cast<const uint16_t*>(oci_page.text())).ToLocalChecked();
	argv[2] = Nan::Null();

	// Invoke callback
	callback->Call(2, argv);

	if (isDebug())
	{
		std::cout << "RequestWorker::HandleOKCallback: END" << std::endl << std::flush;
	}
}

///////////////////////////////////////////////////////////////////////////
void RequestWorker::HandleErrorCallback()
{
	if (isDebug())
	{
		std::cout << "RequestWorker::HandleErrorCallback: START" << std::endl << std::flush;
	}
}

///////////////////////////////////////////////////////////////////////////
void init(v8::Handle<v8::Object> exports)
{
	if (isDebug())
	{
		std::cout << "::init" << std::endl << std::flush;
	}

	OracleBindings::Init(exports);
}

NODE_MODULE(oracleBindings, init)

///////////////////////////////////////////////////////////////////////////
OracleBindings::OracleBindings(const Config& config)
	:	m_oracleObject(0)
{
	if (isDebug())
	{
		std::cout << "OracleBindings::OracleBindings" << std::endl << std::flush;
	}

	m_oracleObject = new OracleObject(config);
}

///////////////////////////////////////////////////////////////////////////
OracleBindings::~OracleBindings()
{
	if (isDebug())
	{
		std::cout << "OracleBindings::~OracleBindings" << std::endl << std::flush;
	}

	delete m_oracleObject;
}

///////////////////////////////////////////////////////////////////////////
NAN_MODULE_INIT(OracleBindings::Init)
{
	if (isDebug())
	{
		std::cout << "OracleBindings::Init" << std::endl << std::flush;
	}

	// Prepare constructor template
	v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
	tpl->SetClassName(Nan::New("OracleBindings").ToLocalChecked());
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	// Prototype
	Nan::SetPrototypeMethod(tpl, "create",		create);
	Nan::SetPrototypeMethod(tpl, "destroy",		destroy);
	Nan::SetPrototypeMethod(tpl, "executeSync",	executeSync);
	Nan::SetPrototypeMethod(tpl, "request",		request);

	m_constructor.Reset(Nan::GetFunction(tpl).ToLocalChecked());
	Nan::Set(target, Nan::New("OracleBindings").ToLocalChecked(), Nan::GetFunction(tpl).ToLocalChecked());
}

///////////////////////////////////////////////////////////////////////////
NAN_METHOD(OracleBindings::New)
{
	if (isDebug())
	{
		std::cout << "OracleBindings::New" << std::endl << std::flush;
	}

	Nan::HandleScope scope;

	// Load the configuration object
	Config config;
	std::string error = getConfig(info, &config);
	if (!error.empty())
	{
		Nan::ThrowTypeError(error.c_str());
		//NanReturnUndefined();
	}

	if (info.IsConstructCall()) {
		// Create the object and warp it
		OracleBindings* obj = new OracleBindings(config);
		obj->Wrap(info.This());
		info.GetReturnValue().Set(info.This());
	} else {
		v8::Local<v8::Function> cons = Nan::New<v8::Function>(m_constructor);
		info.GetReturnValue().Set(cons->NewInstance());
	}
}

///////////////////////////////////////////////////////////////////////////
NAN_METHOD(OracleBindings::create)
{
	if (isDebug())
	{
		std::cout << "OracleBindings::create" << std::endl << std::flush;
	}

	Nan::HandleScope scope;

	OracleBindings* obj = getObject(info);

	if (!obj->m_oracleObject->create())
	{
		Nan::ThrowError(obj->m_oracleObject->getOracleError().what().c_str());
	}

	//NanReturnUndefined();
}

///////////////////////////////////////////////////////////////////////////
NAN_METHOD(OracleBindings::destroy)
{
	if (isDebug())
	{
		std::cout << "OracleBindings::destroy" << std::endl << std::flush;
	}

	Nan::HandleScope scope;

	OracleBindings* obj = getObject(info);

	if (!obj->m_oracleObject->destroy())
	{
		Nan::ThrowError(obj->m_oracleObject->getOracleError().what().c_str());
	}

	//NanReturnUndefined();
}

///////////////////////////////////////////////////////////////////////////
NAN_METHOD(OracleBindings::executeSync)
{
	if (isDebug())
	{
		std::cout << "OracleBindings::executeSync" << std::endl << std::flush;
	}

	Nan::HandleScope scope;

	OracleBindings* obj = getObject(info);

	// Check the number and types of arguments
	if (info.Length() != 3)
	{
		Nan::ThrowError("The function executeSync requires exactly 3 arguments!");
		//NanReturnUndefined();
	}

	// Get the username
	std::string username;
	if (!nodeUtilities::getArgString(info, 0, &username) || username.empty())
	{
		Nan::ThrowError("The parameter username must be a non-empty string!");
		//NanReturnUndefined();
	}

	// Get the password
	std::string password;
	if (!nodeUtilities::getArgString(info, 1, &password))
	{
		Nan::ThrowError("The parameter password must be a string!");
		//NanReturnUndefined();
	}

	// Get the sql statement
	std::string sql;
	if (!nodeUtilities::getArgString(info, 2, &sql) || sql.empty())
	{
		Nan::ThrowError("The parameter sql must be a non-empty string!");
		//NanReturnUndefined();
	}

	// Execute the sql statement
	if (!obj->m_oracleObject->execute(username, password, sql))
	{
		Nan::ThrowError(obj->m_oracleObject->getOracleError().what().c_str());
		//NanReturnUndefined();
	}

	//NanReturnUndefined();
}

///////////////////////////////////////////////////////////////////////////
NAN_METHOD(OracleBindings::request)
{
	if (isDebug())
	{
		std::cout << "OracleBindings::request" << std::endl << std::flush;
	}

	Nan::HandleScope scope;

	OracleBindings* obj = getObject(info);

	// Parse the arguments
	std::string				username;
	std::string				password;
	std::string				procedure;
	parameterListType		parameters;
	propertyListType		cgi;
	fileListType			files;
	std::string				doctablename;

	// Check the number and types of arguments
	if (info.Length() != 8)
	{
		Nan::ThrowError("This function requires exactly 8 arguments! (username, password, procedure, info, cgi, files, doctablename, callback)");
		//NanReturnUndefined();
	}

	// 1. Get username
	if (!nodeUtilities::getArgString(info, 0, &username))
	{
		Nan::ThrowError("Argument 1 must be the username!");
		//NanReturnUndefined();
	}

	// 2. Get password
	if (!nodeUtilities::getArgString(info, 1, &password))
	{
		Nan::ThrowError("Argument 2 must be the password!");
		//NanReturnUndefined();
	}

	// 3. Get procedure name
	if (!nodeUtilities::getArgString(info, 2, &procedure) || procedure.empty())
	{
		Nan::ThrowError("Argument 3 must be a non-empty procedure!");
		//NanReturnUndefined();
	}

	// 4. Get the parameters of the procedure
	v8::Local<v8::Object> parametersObject;
	if (!nodeUtilities::getArgObject(info, 3, &parametersObject))
	{
		Nan::ThrowError("Argument 4 must be an object with the parameters of the procedure procedure!");
		//NanReturnUndefined();
	}
	if (!nodeUtilities::objectAsParameterList(parametersObject, &parameters))
	{
		Nan::ThrowError("Argument 4 must be an object with all properties of type string!");
		//NanReturnUndefined();
	}

	// 5. Get the CGI environment
	v8::Local<v8::Object> cgiObject;
	if (!nodeUtilities::getArgObject(info, 4, &cgiObject))
	{
		Nan::ThrowError("Argument 5 must be an object with the CGI environment!");
		//NanReturnUndefined();
	}
	if (!nodeUtilities::objectAsStringLists(cgiObject, &cgi))
	{
		Nan::ThrowError("Argument 5 must be an object with all properties of type string!");
		//NanReturnUndefined();
	}
	if (cgi.size() < 1)
	{
		Nan::ThrowError("Argument 5 must be an object with at least one property!");
		//NanReturnUndefined();
	}

	// 6. The array of files to upload
	v8::Local<v8::Array> fileArray;
	if (!nodeUtilities::getArgArray(info, 5, &fileArray))
	{
		Nan::ThrowError("Argument 6 must be an array with the files to be uploaded!");
		//NanReturnUndefined();
	}
	// Get the number of array entries
	uint32_t length = fileArray->Length();
	// Process the array
	for (uint32_t i = 0; i < length; i++)
	{
		// Get the file object
		v8::Local<v8::Value> value = fileArray->Get(i);
		if (!value->IsObject())
		{
			Nan::ThrowError("Argument 6 must only contain objects!");
			//NanReturnUndefined();
		}

		// Cast the value to an object
		v8::Local<v8::Object> object = v8::Local<v8::Object>::Cast(value);

		// Now get the properties of the object

		// "fieldValue"
		std::string fieldValue;
		if (!nodeUtilities::getObjString(object, "fieldValue", &fieldValue) || fieldValue.empty())
		{
			Nan::ThrowError("Argument 6 must contain objects with a non-empty string property \"fieldValue\"!");
			//NanReturnUndefined();
		}

		// "filename"
		std::string filename;
		if (!nodeUtilities::getObjString(object, "filename", &filename) || filename.empty())
		{
			Nan::ThrowError("Argument 6 must contain objects with a non-empty string property \"filename\"!");
			//NanReturnUndefined();
		}

		// "physicalFilename"
		std::string physicalFilename;
		if (!nodeUtilities::getObjString(object, "physicalFilename", &physicalFilename) || physicalFilename.empty())
		{
			Nan::ThrowError("Argument 6 must contain objects with a non-empty string property \"physicalFilename\"!");
			//NanReturnUndefined();
		}

		// "encoding"
		std::string encoding;
		if (!nodeUtilities::getObjString(object, "encoding", &encoding))
		{
			Nan::ThrowError("Argument 6 must contain objects with a string property \"encoding\"!");
			//NanReturnUndefined();
		}

		// "mimetype"
		std::string mimetype;
		if (!nodeUtilities::getObjString(object, "mimetype", &mimetype))
		{
			Nan::ThrowError("Argument 6 must contain objects with a string property \"mimetype\"!");
			//NanReturnUndefined();
		}

		// Add the new file entry
		fileType file = fileType(fieldValue, filename, physicalFilename, encoding, mimetype);
		files.push_back(file);
	}

	// 7. Get the table name
	if (!nodeUtilities::getArgString(info, 6, &doctablename) || doctablename.empty())
	{
		Nan::ThrowError("Argument must be a non-empty document table name!");
		//NanReturnUndefined();
	}

	// 8. The callback function
	if (!info[7]->IsFunction())
	{
		Nan::ThrowError("Argument 8 must be the callback function!");
		//NanReturnUndefined();
	}
	Nan::Callback* callback = new Nan::Callback(info[7].As<v8::Function>());

	// Invoke async worker
	Nan::AsyncQueueWorker(new RequestWorker(callback, obj->m_oracleObject, username, password, procedure, parameters, cgi, files, doctablename));
	
	//NanReturnUndefined();
}

///////////////////////////////////////////////////////////////////////////
std::string OracleBindings::getConfig(Nan::NAN_METHOD_ARGS_TYPE args, Config* config)
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
inline OracleBindings* OracleBindings::getObject(Nan::NAN_METHOD_ARGS_TYPE args)
{
	return ObjectWrap::Unwrap<OracleBindings>(args.This());
}
