#ifndef _ots_Iterate_h_
#define _ots_Iterate_h_


#include <string>
#include <map>

namespace ots
{

struct Iterate
{
	static const std::string COMMAND_BEGIN_LABEL;
	static const std::string COMMAND_CONFIGURE_ACTIVE_GROUP;
	static const std::string COMMAND_CONFIGURE_ALIAS;
	static const std::string COMMAND_CONFIGURE_GROUP;
	static const std::string COMMAND_EXECUTE_FE_MACRO;
	static const std::string COMMAND_EXECUTE_MACRO;
	static const std::string COMMAND_MODIFY_ACTIVE_GROUP;
	static const std::string COMMAND_REPEAT_LABEL;
	static const std::string COMMAND_RUN;

	static const std::map<std::string,std::string> commandToTableMap;

	static std::map<std::string,std::string> createCommandToTableMap()
	{
		std::map<std::string,std::string> m;
		m[COMMAND_BEGIN_LABEL] 				=  "IterationCommandBeginLabelConfiguration";
		m[COMMAND_CONFIGURE_ACTIVE_GROUP] 	=  ""; //no parameters
		m[COMMAND_CONFIGURE_ALIAS] 			=  "IterationCommandConfigureAliasConfiguration";
		m[COMMAND_CONFIGURE_GROUP] 			=  "IterationCommandConfigureGroupConfiguration";
		m[COMMAND_EXECUTE_FE_MACRO] 		=  "IterationCommandExecuteFEMacroConfiguration";
		m[COMMAND_EXECUTE_MACRO] 			=  "IterationCommandExecuteMacroConfiguration";
		m[COMMAND_MODIFY_ACTIVE_GROUP] 		=  "IterationCommandModifyGroupConfiguration";
		m[COMMAND_REPEAT_LABEL] 			=  "IterationCommandRepeatLabelConfiguration";
		m[COMMAND_RUN] 						=  "IterationCommandRunConfiguration";
		return m;
	}


	static const std::string PLAN_TABLE_COMMAND_LINK_FIELD;
};

const std::string Iterate::COMMAND_BEGIN_LABEL 				= "BEGIN_LABEL";
const std::string Iterate::COMMAND_REPEAT_LABEL 			= "REPEAT_LABEL";
const std::string Iterate::COMMAND_CONFIGURE_ALIAS 			= "CONFIGURE_ALIAS";
const std::string Iterate::COMMAND_CONFIGURE_GROUP 			= "CONFIGURE_GROUP";
const std::string Iterate::COMMAND_MODIFY_ACTIVE_GROUP 		= "MODIFY_ACTIVE_GROUP";
const std::string Iterate::COMMAND_CONFIGURE_ACTIVE_GROUP 	= "CONFIGURE_GROUP";
const std::string Iterate::COMMAND_EXECUTE_MACRO 			= "EXECUTE_MACRO";
const std::string Iterate::COMMAND_EXECUTE_FE_MACRO 		= "EXECUTE_FE_MACRO";
const std::string Iterate::COMMAND_RUN 						= "RUN";
const std::map<std::string,std::string> Iterate::commandToTableMap = Iterate::createCommandToTableMap();

const std::string Iterate::PLAN_TABLE_COMMAND_LINK_FIELD	= "LinkToCommandUID";
}

#endif
