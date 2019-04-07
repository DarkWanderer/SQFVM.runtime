#include "vmstack.h"
#include "value.h"
#include "virtualmachine.h"
#include "callstack.h"
#include "instruction.h"

std::vector<sqf::vmstack::stackdump> sqf::vmstack::dump_callstack_diff(std::shared_ptr<sqf::callstack> target)
{
	std::vector<sqf::vmstack::stackdump> vec;

	auto start = this->mstacks.rbegin();
	for (auto it = start; it != this->mstacks.rend(); it++)
	{
		auto inst = (*it)->last_inst();

		stackdump dump;
		dump.namespace_used = (*it)->getnamespace();
		dump.line = inst->line();
		dump.column = inst->col();
		dump.file = inst->file();
		dump.dbginf = inst->dbginf("STT");
		dump.callstack_name = (*it)->get_name();
		dump.scope_name = (*it)->getscopename();
		vec.push_back(dump);

		if ((*it) == target)
		{
			break;
		}
	}
	return vec;
}

void sqf::vmstack::pushinst(sqf::virtualmachine * vm, std::shared_ptr<instruction> inst)
{
	if (mstacks.empty())
	{
		mstacks.push_back(std::make_shared<callstack>(vm->missionnamespace()));
	}
	mstacks.back()->pushinst(inst);
}

std::shared_ptr<sqf::value> sqf::vmstack::getlocalvar(std::string varname)
{
	for (auto it = stacks_begin(); it != stacks_end(); ++it)
	{
		if (it->get()->containsvar(varname))
		{
			return it->get()->getvar(varname);
		}
	}
	return std::make_shared<sqf::value>();
}

void sqf::vmstack::sleep(std::chrono::milliseconds ms)
{
	mwakeupstamp = sqf::virtualmachine::system_time() + ms;
	misasleep = true;
}
