#pragma once
#include <vector>
#include <string>

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>
#include <unordered_map>

enum class ShaderVarType {
	// default single value
	_BOOL_,
	_INT_,
	_UINT_,
	_FLOAT_,
	_DOUBLE_,
	// boolean vectors 
	_BVEC2_,
	_BVEC3_,
	_BVEC4_,
	// integer vectors
	_IVEC2_,
	_IVEC3_,
	_IVEC4_,
	// double vectors
	_DVEC2_,
	_DVEC3_,
	_DVEC4_,
	// float vectors
	_VEC2_,
	_VEC3_,
	_VEC4_,

	// matricies
	_MAT3x3_,
	_MAT4x4_
};

enum class ShaderTableGroup {
	Inputs,
	Uniforms,
	Outputs,
};

struct ShaderTableElement {
	std::string name;
	ShaderVarType type;
	int location = -1;
};

struct CG_Node
{ };

struct FloatNode : public CG_Node {
	
	CG_Node output;
	
	float value;
};

template<typename TNODE>
struct MultiplyNode : public CG_Node {
	
	TNODE input_a; // input a
	TNODE input_b; // input b 

	TNODE output; // out put node
};

/*
* 
*  A * B = C 
* 
	auto n1 = Shader.AddNode(FloatNode{4})
	auto n2 = Shader.AddNode(FloatNode{8})
	
	auto mNode = Shader.AddNode(MultiplyNode<FloatNode>{})
	
	Shader.ConnectNodes(n1.output, mNode.input_a) 
	Shader.ConnectNodes(n2.output, mNode.input_b) 

*/


class Shader {
public:
	Shader(VkDevice device, const std::string& filepath, VkShaderStageFlagBits stage);

	~Shader();
	
	void AddInput(int location, ShaderVarType type, const std::string& name);
	void AddOutput(ShaderVarType type, const std::string& name);
	void AddUniform(ShaderVarType type, const std::string& name);

	void AddNode(CG_Node node);
	void ConnectNodes(CG_Node left, CG_Node right);

	bool Compile();

	VkShaderModule Get();

private:
	void GenerateShaderData();

	bool CreateModule();
	bool LoadData();
	bool ConvertSourceToSPIRV();


private:
	const std::string exension_GLSL = ".glsl";
	const std::string exension_SPIRV = ".spv";

	
	CG_Node* root;

	// shader generation elements
	std::unordered_map<ShaderTableGroup, std::vector<ShaderTableElement>> ioTable;

	std::string directory;
	std::string fileName;
	std::string version;

	VkShaderStageFlagBits stage;

	std::vector<unsigned int> data;

	VkShaderModule shaderModule;
	VkDevice device;
};