#include "ShaderGraph.h"

#include <fstream>
#include <iostream>  // For console logging
#include <sstream>   // For stringstream
#include <cstdlib>   // For system function
#include <cstdio>    // For popen, pclose
#include <string>    // For string manipulation
#include <cstdio>  // For pipe, feof, fgets

#include <filesystem>

#include <debug/Console.h>

#include "filesystem/Utils.h"

#include "../Textures/TextureFactory.h"


ShaderGraph::ShaderGraph(const std::string& filepath, VkShaderStageFlagBits stage)
{

	this->directory = filepath.substr(0, filepath.find_last_of('/') + 1);
	auto slash = filepath.find_last_of('/');
	auto dot = filepath.find('.');
	this->fileName = filepath.substr(slash + 1, (dot - slash) - 1);
	
	this->stage = stage;

	this->ioTable[ShaderTableGroup::Inputs] = {};
	this->ioTable[ShaderTableGroup::Outputs] = {};
	this->ioTable[ShaderTableGroup::Uniforms] =  {};

}

ShaderGraph::~ShaderGraph()
{
	-VulkanAPI::DestroyShaderModule(shaderModule);

}

void ShaderGraph::AddInput(int location, ShaderVarType type, const std::string& name, int binding)
{
	ShaderTableElement elm{
		.name = name,
		.type = type,
		.location = location,
		.binding = binding,
	};

	ioTable[ShaderTableGroup::Inputs].push_back(elm);
}

void ShaderGraph::AddOutput(int location, ShaderVarType type, const std::string& name)
{
	ShaderTableElement elm{
	.name = name,
	.type = type,
	.location = location,
	};

	ioTable[ShaderTableGroup::Outputs].push_back(elm);
}

void ShaderGraph::AddUniform(ShaderVarType type, const std::string& name)
{
	ShaderTableElement elm{
	.name = name,
	.type = type,
	};

	ioTable[ShaderTableGroup::Uniforms].push_back(elm);
}

void ShaderGraph::AddConstant()
{
	
}

void ShaderGraph::AddTexture2D(const std::string& filepath)
{
	uint32_t width = 16u;
	uint32_t height = 16u;
	TextureFormat format = TextureFormat::ARGB32_SFLOAT;

	// Load the Texture.
	auto texture = TextureFactory::CreateSampledTexture2D(width, height, format);
	textures[++textureCount] = texture;

}

void ShaderGraph::AddMain(const std::string& fn)
{
	func.clear();
	func = fn;
}

void ShaderGraph::AddNode(CG_Node* node)
{	
	nodes.push_back(node);
}

void ShaderGraph::ConnectNodes(CG_Node* left, CG_Node* right)
{
	edges[left] = right;
	edges[right] = left;
}


TReturn ShaderGraph::Compile()
{
	std::string hlslFilePath = directory + fileName + ShaderGraph::extension_GLSL;
	FileSystem::CreateIFFNoneExist(hlslFilePath);
	
	-GenerateShaderData();


	-ConvertSourceToSPIRV();
	-LoadSPIRVByteCode();
	-CreateModule();

	return  TReturn::SUCCESS;
}

VkShaderModule ShaderGraph::Get()
{
	return shaderModule;
}

std::vector<VkVertexInputAttributeDescription> ShaderGraph::GetAttributes()
{
	std::vector<VkVertexInputAttributeDescription> attributes;

	// build the attrutes list based on the current inputs.
	auto list = ioTable[ShaderTableGroup::Inputs];
	uint32_t offset = 0;

	for (auto it = list.begin(); it != list.end(); it++)
	{
		VkVertexInputAttributeDescription attr;
		attr.binding = it->binding;
		attr.location = it->location;
		
		int components_count = 1;

		switch (it->type)
		{
		case ShaderVarType::_INT_:
			attr.format = VK_FORMAT_R32_SINT;
			components_count = 1;
			break;

		case ShaderVarType::_DOUBLE_:
		case ShaderVarType::_FLOAT_:
		case ShaderVarType::_BOOL_:
			components_count = 1;
			attr.format = VK_FORMAT_R32_SFLOAT;
			break;


		case ShaderVarType::_IVEC2_:
			attr.format = VK_FORMAT_R32G32_SINT;
			components_count = 2;
			break;

		case ShaderVarType::_DVEC2_:
		case ShaderVarType::_VEC2_:
		case ShaderVarType::_BVEC2_:
			components_count = 2;
			attr.format = VK_FORMAT_R32G32_SFLOAT;
			break;



		case ShaderVarType::_IVEC3_:
			attr.format = VK_FORMAT_R32G32B32_SINT;
			components_count = 3;
			break;

		case ShaderVarType::_DVEC3_:
		case ShaderVarType::_VEC3_:
		case ShaderVarType::_BVEC3_:
			components_count = 3;
			attr.format = VK_FORMAT_R32G32B32_SFLOAT;
			break;

		case ShaderVarType::_IVEC4_:
			components_count = 4;
			attr.format = VK_FORMAT_R32G32B32A32_SINT;
			break;

		case ShaderVarType::_BVEC4_:
		case ShaderVarType::_VEC4_:
		case ShaderVarType::_DVEC4_:
			components_count = 4;
			attr.format = VK_FORMAT_R32G32B32A32_SFLOAT;
			break;
		default:
			std::cout << "UnRecognized Type" << std::endl;
			break;

		}

		attr.offset = offset;
		
		bool isInt = it->type == ShaderVarType::_INT_
			|| it->type == ShaderVarType::_IVEC2_
			|| it->type == ShaderVarType::_IVEC3_
			|| it->type == ShaderVarType::_IVEC4_;

		bool isBool = it->type == ShaderVarType::_BOOL_
			|| it->type == ShaderVarType::_BVEC2_
			|| it->type == ShaderVarType::_BVEC3_
			|| it->type == ShaderVarType::_BVEC4_;

		// calculate the next attributes offset, from the beginning of the buffer
		float size = 0;

		if (isInt) {
			size = sizeof(int32_t) * components_count;
		}
		else if (isBool) {
			size = sizeof(bool) * components_count;
		}
		else {
			size = sizeof(float_t) * components_count;
		}

		offset += components_count; // working in 32bit instead of 8s

		attributes.push_back(attr);
	}


	return attributes;
}

TReturn ShaderGraph::ConvertSourceToSPIRV() {

	std::string hlslFilePath = directory + fileName + ShaderGraph::extension_GLSL ;
	std::string spvFilePath = directory + fileName + ShaderGraph::extension_SPIRV;

	auto str = FileSystem::ReadFileFromDisc(hlslFilePath);

	Console::Log("File Contents: \n", str);

	bool isGeometryShader = (stage & VK_SHADER_STAGE_GEOMETRY_BIT) != 0;
	bool isFragmentShader = (stage & VK_SHADER_STAGE_FRAGMENT_BIT) != 0;

	std::string shader_stage = (isGeometryShader ? " geom " : isFragmentShader ? " frag " : " vert ");

	std::string cmd = "glslangValidator -S" + shader_stage  + "\"" + hlslFilePath + "\"" + " -V -o " + "\"" + spvFilePath + "\"";
	Console::Log("Running: cmd: ", cmd);
	
	std::stringstream output;
	// Capture command output
	FILE* pipe = _popen(cmd.c_str(), "r"); // execute the command 
	if (!pipe) {
		Console::Fatal("Failed to execute command!");
		return TReturn::COMMAND_EXECUTECUTION_FAILED;
	}

	char buffer[128];
	while (!feof(pipe)) {
		if (fgets(buffer, 128, pipe) != nullptr) {
			output << buffer;
		}
	}
	
	// Check result of the command
	int result = _pclose(pipe);

	if (result != 0) {
		Console::Warn("System Command Failed! ErrorCode: ", result);
		Console::Log("Errors: ", output.str());
			
		return TReturn::FAILURE;
	}

	return TReturn::SUCCESS;
}

TReturn ShaderGraph::LoadSPIRVByteCode() {
	std::string spvFilePath = directory + fileName + ShaderGraph::extension_SPIRV;

	// Open the SPIR-V file in binary mode
	std::ifstream file(spvFilePath, std::ios::binary);
	if (!file.is_open()) {
		// Failed to open the file
		return TReturn::FILE_NOT_FOUND;
	}

	// Get the file size
	file.seekg(0, std::ios::end);
	std::streampos fileSize = file.tellg();
	file.seekg(0, std::ios::beg);

	// Read the file data into a vector of bytes
	std::vector<char> fileData(fileSize);
	file.read(fileData.data(), fileSize);

	// Close the file
	file.close();

	// Convert the byte data to a vector of unsigned integers (SPIR-V bytecode)
	data.assign(reinterpret_cast<const unsigned int*>(fileData.data()), reinterpret_cast<const unsigned int*>(fileData.data() + fileSize));

	if (data.empty())
		return TReturn::FAILURE;

	// Check if any data was read
	return TReturn::SUCCESS;
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


TReturn ShaderGraph::GenerateShaderData()
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
			ss << "layout(location=" << std::to_string(elm.location) << ") in " << ShaderVarTypeToGLSLTypeString(elm.type) << " " << elm.name << ";\n";
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
			ss << "layout(location=" << i->location << ") out " << ShaderVarTypeToGLSLTypeString(elm.type) << " " << elm.name << "; \n";
		}
	}
	ss << "\n";

	ss << "void main()\n{\n";
	ss << func;
	// ------------------------------------
	// CODE GRAPH DECOMPOSITION
	// ------------------------------------

	ss << "}\n";

	// write stream to file.
	auto filepath = directory + fileName + ShaderGraph::extension_GLSL;

	FileSystem::WriteToFile(filepath, ss.str());

	return TReturn::SUCCESS;
}

TReturn ShaderGraph::CreateModule() {

	-VulkanAPI::CreateShaderModule(data, shaderModule);

	return TReturn::SUCCESS;
}


