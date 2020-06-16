#include "TRACE/trace.h"
#define TRACE_NAME "ECLTest"

#include "otsdaq-utilities/ECLWriter/ECLConnection.h"

#include <boost/program_options.hpp>
namespace bpo = boost::program_options;

int main(int argc, char* argv[])
{
	std::ostringstream descstr;
	descstr << argv[0]
	        << " --host <ECL host> --user <Username> --pwd <Password> [--title <Message "
	           "title>] [message]";
	bpo::options_description desc(descstr.str());
	desc.add_options()("host,i", bpo::value<std::string>(), "ECL Instance Address")(
	    "user,u", bpo::value<std::string>(), "ECL Username")(
	    "pwd,p", bpo::value<std::string>(), "ECL Password")(
	    "title,t", bpo::value<std::string>(), "Message title")(
	    "msg,m", bpo::value<std::string>(), "Log Message")("help,h",
	                                                       "produce help message");
	bpo::positional_options_description p;
	p.add("msg", -1);
	bpo::variables_map vm;
	try
	{
		bpo::store(bpo::command_line_parser(argc, argv).options(desc).positional(p).run(),
		           vm);
		bpo::notify(vm);
	}
	catch(bpo::error const& e)
	{
		TLOG(TLVL_ERROR) << "Exception from command line processing in " << argv[0]
		                 << ": " << e.what() << "\n";
		exit(-1);
	}
	if(vm.count("help"))
	{
		std::cout << desc << std::endl;
		exit(1);
	}

	std::string ECLUser = "", ECLPwd = "", ECLHost = "", title = "ECL Test Message",
	            message = "";
	if(vm.count("user"))
	{
		ECLUser = vm["user"].as<std::string>();
	}
	if(vm.count("pwd"))
	{
		ECLPwd = vm["pwd"].as<std::string>();
	}
	if(vm.count("host"))
	{
		ECLHost = vm["host"].as<std::string>();
	}
	if(vm.count("title"))
	{
		title = vm["title"].as<std::string>();
	}
	if(vm.count("msg"))
	{
		message = vm["msg"].as<std::string>();
	}

	ECLEntry_t eclEntry;
	eclEntry.author(ECLUser);
	eclEntry.category("TDAQ");
	eclEntry.subject(title);
	Form_t                 form;
	Field_t                field;
	Form_t::field_sequence fields;

	form.name("default");

	field = Field_t(message, "text");
	fields.push_back(field);

	form.field(fields);

	eclEntry.form(form);

	ECLConnection eclConn(ECLUser, ECLPwd, ECLHost);
	if(!eclConn.Post(eclEntry))
	{
		return -1;
	}

	return 0;
}