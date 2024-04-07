#include "Shader.h"

#include <fstream>
#include <sstream>
#include <filesystem>


#include <volk.h>

#include <debug/Console.h>

#include "filesystem/Utils.h"


Shader::Shader(VkDevice device, const std::string& filepath, VkShaderStageFlagBits stage)
{
	this->device = device;

	this->directory = filepath.substr(0, filepath.find_last_of('/') + 1);
	auto slash = filepath.find_last_of('/');
	auto dot = filepath.find('.');
	this->fileName = filepath.substr(slash + 1, (dot - slash) - 1);
	
	this->stage = stage;

	this->ioTable[ShaderTableGroup::Inputs] = {};
	this->ioTable[ShaderTableGroup::Outputs] = {};
	this->ioTable[ShaderTableGroup::Uniforms] =  {};

}

Shader::~Shader()
{
}

void Shader::AddInput(int location, ShaderVarType type, const std::string& name)
{
	ShaderTableElement elm{
		.name = "i_" + name,
		.type = type,
		.location = location,
	};

	ioTable[ShaderTableGroup::Inputs].push_back(elm);
}

void Shader::AddOutput(ShaderVarType type, const std::string& name)
{
	ShaderTableElement elm{
	.name = "o_" + name,
	.type = type,
	};

	ioTable[ShaderTableGroup::Outputs].push_back(elm);
}

void Shader::AddUniform(ShaderVarType type, const std::string& name)
{
	ShaderTableElement elm{
	.name = "u_" + name,
	.type = type,
	};

	ioTable[ShaderTableGroup::Uniforms].push_back(elm);
}


bool Shader::Compile()
{
	std::string hlslFilePath = directory + fileName + Shader::exension_GLSL;
	FileSystem::CreateIFFNoneExist(hlslFilePath);
	GenerateShaderData();
	return ConvertSourceToSPIRV() && LoadData() && CreateModule();
}
VkShaderModule Shader::Get()
{
	return shaderModule;
}

bool Shader::ConvertSourceToSPIRV() {

	std::string hlslFilePath = directory + fileName + Shader::exension_GLSL;
	std::string spvFilePath = directory + fileName + Shader::exension_SPIRV;

	std::string cmd = "glslangValidator -S vert " + hlslFilePath + " -V -o " + spvFilePath;
	int result = std::system(cmd.c_str());

	if (result != 0) {
		Console::Warn("System Command Failed!");
		return false;
	}

	return true;
}

bool Shader::LoadData() {
	std::string spvFilePath = directory + fileName + Shader::exension_SPIRV;

	std::string fileData = FileSystem::ReadFileFromDisc(spvFilePath);

	if (fileData.size() <= 0)
		return false;

	data.assign(reinterpret_cast<const unsigned int*>(fileData.data()), reinterpret_cast<const unsigned int*>(fileData.data() + fileData.size()));

	if (data.size() <= 0)
		return false;


	return true;
}

std::string ShaderVarTypeToGLSLTypeString(ShaderVarType type) {
	switch (type)
	{
	case ShaderVarType::_BOOL_:
		return "bool";
	case ShaderVarType::_INT_:
		return "int";
	case ShaderVarType::_UINT_:
		return "uint";
	case ShaderVarType::_FLOAT_:
		return "float";
	case ShaderVarType::_DOUBLE_:
		return "double";
	case ShaderVarType::_BVEC2_:
		return "bvec2";
	case ShaderVarType::_BVEC3_:
		return "bvec3";
	case ShaderVarType::_BVEC4_:
		return "bvec4";
	case ShaderVarType::_IVEC2_:
		return "ivec2";
	case ShaderVarType::_IVEC3_:
		return "ivec3";
	case ShaderVarType::_IVEC4_:
		return "ivec4";
	case ShaderVarType::_DVEC2_:
		return "dvec2";
	case ShaderVarType::_DVEC3_:
		return "dvec3";
	case ShaderVarType::_DVEC4_:
		return "dvec4";
	case ShaderVarType::_VEC2_:
		return "vec2";
	case ShaderVarType::_VEC3_:
		return "vec3";
	case ShaderVarType::_VEC4_:
		return "vec4";
	case ShaderVarType::_MAT3x3_:
		return "mat3";
	case ShaderVarType::_MAT4x4_:
		return "mat4";
	}
}


void Shader::GenerateShaderData()
{
	std::stringstream ss;

	ss << "// ------------------------------------------------------------------ \n";
	ss << "// | *** THIS FILE IS GENERATED USING TEMPORAL SHADER GENERATION *** | \n";
	ss << "// ------------------------------------------------------------------- \n";
	ss << "\n";

	ss << "#version 450" << std::endl;
	ss << "\n";

	// generate inputs
	{
		auto elements_list = ioTable[ShaderTableGroup::Inputs];

		for (auto i = elements_list.begin(); i != elements_list.end(); i++)
		{
			auto elm = *i;
			ss << "location(layout =" << elm.location << ") in " << ShaderVarTypeToGLSLTypeString(elm.type) << " " << elm.name << ";\n";
		}
	}
	ss << "\n";

	// generate uniforms
	{
		auto elements_list = ioTable[ShaderTableGroup::Uniforms];

		for (auto i = elements_list.begin(); i != elements_list.end(); i++)
		{
			auto elm = *i;
			ss << "uniform " << ShaderVarTypeToGLSLTypeString(elm.type) << " " << elm.name << ";\n";
		}
	}
	ss << "\n";

	// generate outputs

	{
		auto elements_list = ioTable[ShaderTableGroup::Outputs];

		for (auto i = elements_list.begin(); i != elements_list.end(); i++)
		{
			auto elm = *i;
			ss << "output " << ShaderVarTypeToGLSLTypeString(elm.type) << " " << elm.name << ";\n";
		}
	}
	ss << "\n";

	ss << "void main()\n{\n";

	// ------------------------------------
	// CODE GRAPH DECOMPOSITION
	// ------------------------------------

	ss << "}\n";

	// write stream to file.
	auto filepath = directory + fileName + Shader::exension_GLSL;

	FileSystem::WriteToFile(filepath, ss.str());

}

bool Shader::CreateModule() {

	VkShaderModuleCreateInfo create{
			.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.codeSize = data.size() * sizeof(unsigned int),
			.pCode = data.data(),
	};

	return vkCreateShaderModule(device, &create, nullptr, &shaderModule) == VK_SUCCESS;
}


