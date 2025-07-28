#include <slang/slang-com-ptr.h>
#include <slang/slang.h>

#include <stdio.h>
#include <stdlib.h>

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv)
{
	if (argc < 3)
		return 1;
	Slang::ComPtr<slang::IGlobalSession> global_session;
	slang::createGlobalSession(global_session.writeRef());

	slang::CompilerOptionEntry entries[] = {
		{.name = slang::CompilerOptionName::VulkanUseEntryPointName,
	     .value = {.intValue0 = 1}}
    };
	slang::SessionDesc desc {};
	desc.compilerOptionEntries = entries;
	desc.compilerOptionEntryCount = sizeof(entries) / sizeof(slang::CompilerOptionEntry);
	slang::TargetDesc targetDesc = {};
	targetDesc.format = SLANG_SPIRV;
	targetDesc.profile = global_session->findProfile("spirv_1_6");
	targetDesc.flags = SLANG_TARGET_FLAG_GENERATE_SPIRV_DIRECTLY;
	desc.targetCount = 1;
	desc.targets = &targetDesc;

	Slang::ComPtr<slang::ISession> session;
	global_session->createSession(desc, session.writeRef());
	Slang::ComPtr<slang::IBlob>   diagnostics;
	Slang::ComPtr<slang::IModule> mod {
		session->loadModule(argv[1], diagnostics.writeRef())};

	if (diagnostics)
	{
		fprintf(stderr, "%s\n", (char const*)diagnostics->getBufferPointer());
		return 1;
	}

	int32_t entry_points = mod->getDefinedEntryPointCount();

	slang::IComponentType** comps = new slang::IComponentType*[entry_points + 1];
	comps[0] = mod;
	for (int32_t i {0}; i < entry_points; ++i)
	{
		slang::IEntryPoint* entry;
		mod->getDefinedEntryPoint(i, &entry);
		comps[1 + i] = entry;
	}
	Slang::ComPtr<slang::IComponentType> prog;
	session->createCompositeComponentType(comps, entry_points + 1, prog.writeRef());
	for (int32_t i {1}; i < entry_points + 1; ++i)
		comps[i]->release();

	Slang::ComPtr<slang::IComponentType> linkedProgram;

	prog->link(linkedProgram.writeRef(), diagnostics.writeRef());
	if (diagnostics)
	{
		fprintf(stderr, "%s\n", (char const*)diagnostics->getBufferPointer());
		return 1;
	}

	slang::ProgramLayout*       layout = prog->getLayout(0);
	Slang::ComPtr<slang::IBlob> reflect;
	layout->toJson(reflect.writeRef());

	Slang::ComPtr<slang::IBlob> spv;
	linkedProgram->getTargetCode(0, spv.writeRef(), diagnostics.writeRef());
	if (diagnostics)
	{
		fprintf(stderr, "%s\n", (char const*)diagnostics->getBufferPointer());
		return 1;
	}

	FILE* file = fopen(argv[2], "wb");
	if (file)
	{
		uint64_t s = spv->getBufferSize();
		fwrite(spv->getBufferPointer(), 1, s, file);
		fclose(file);
	}
	char* jsonFilePath = new char[strlen(argv[2]) + 6];
	strcpy(jsonFilePath, argv[2]);
	strcat(jsonFilePath, ".json");
	file = fopen(jsonFilePath, "wb");
	if (file)
	{
		fwrite(reflect->getBufferPointer(), 1, reflect->getBufferSize(), file);
		fclose(file);
	}
	delete[] jsonFilePath;
	delete[] comps;

	return 0;
}