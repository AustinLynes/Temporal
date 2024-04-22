#pragma once

#include "defs.h"

#include "datastructures/datastructures_pch.h"
#include "Graphics/gfx_pch.h"

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
	int binding = -1;
};

struct CG_Node
{ };
//
//struct FloatNode : public CG_Node {
//	
//	CG_Node output;
//	
//	float value;
//};
//
//template<typename TNODE>
//struct MultiplyNode : public CG_Node {
//	
//	TNODE input_a; // input a
//	TNODE input_b; // input b 
//
//	TNODE output; // out put node
//};

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

#define MAX_TEXTURES 16
class Texture2D;

class ShaderGraph {
public:
	ShaderGraph(const std::string& filepath, VkShaderStageFlagBits stage);

	~ShaderGraph();
	
	void AddInput(int location, ShaderVarType type, const std::string& name, int binding);
	void AddOutput(int location, ShaderVarType type, const std::string& name);
	void AddUniform(ShaderVarType type, const std::string& name);
	void AddConstant();
	void AddTexture2D(const std::string& filepath);

	void AddMain(const std::string& fn);

	void AddNode(CG_Node* node);
	void ConnectNodes(CG_Node* left, CG_Node* right);

	TReturn Compile();

	VkShaderModule Get();

	std::vector<VkVertexInputAttributeDescription> GetAttributes();

private:
	TReturn GenerateShaderData();

	TReturn CreateModule();
	TReturn LoadSPIRVByteCode();
	TReturn ConvertSourceToSPIRV();


private:
	const std::string extension_GLSL = ".glsl";
	const std::string extension_SPIRV = ".spv";

	std::vector<CG_Node*> nodes;
	std::unordered_map<CG_Node*, CG_Node*> edges;


	// shader generation elements
	std::unordered_map<ShaderTableGroup, std::vector<ShaderTableElement>> ioTable;

	std::string directory;
	std::string fileName;
	std::string version;
	std::string func;

	VkShaderStageFlagBits stage;

	std::vector<unsigned int> data;
	std::array<Texture2D*, MAX_TEXTURES> textures;
	int textureCount = -1;

	
	VkShaderModule shaderModule;

};