#include "ModelManager.hpp"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tinyobjloader/tiny_obj_loader.h"
#include <glm/gtx/normal.hpp>
#include "MaterialManager.hpp"

namespace Ge
{
	std::vector<Model *> ModelManager::m_models;
	bool ModelManager::initiliaze(VulkanMisc *vM)
	{
		vulkanM = vM;		
		if (!BufferManager::createBuffer(sizeof(UniformBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_vmaUniformBuffers, vM->str_VulkanDeviceMisc))
		{
			Debug::Error("Echec de la creation d'un uniform buffer");
			return false;
		}
		updateDescriptor();
		Debug::INITSUCCESS("ModelManager");
		return true;
	}

	void ModelManager::updateDescriptor()
	{
		std::vector<VkDescriptorBufferInfo> bufferInfoModel{};
		VkDescriptorBufferInfo bufferIM{};
        for (int i = 0 ; i < m_models.size();i++)
        {
            bufferIM.buffer = m_models[i]->getUniformBuffers();
			bufferIM.offset = 0;
			bufferIM.range = sizeof(UniformBufferObject);
			bufferInfoModel.push_back(bufferIM);
        }
		if(m_models.size() == 0)
		{
			bufferIM.buffer = m_vmaUniformBuffers.buffer;
			bufferIM.offset = 0;
			bufferIM.range = sizeof(UniformBufferObject);
			bufferInfoModel.push_back(bufferIM);
			m_descriptor->updateCount(vulkanM, 1, bufferInfoModel);			
		}
		else
		{
			m_descriptor->updateCount(vulkanM, m_models.size(), bufferInfoModel);
		}
	}

	void ModelManager::release()
	{
		for (int i = 0; i < m_shapeBuffers.size();i++)
		{
			delete (m_shapeBuffers[i]);
		}
		m_shapeBuffers.clear();
		for (int i = 0; i < m_models.size(); i++)
		{
			delete (m_models[i]);
		}
		m_models.clear();
		BufferManager::destroyBuffer(m_vmaUniformBuffers);
		delete(m_descriptor);
		Debug::RELEASESUCCESS("ModelManager");
	}

	Model * ModelManager::createModel(ShapeBuffer *buffer, std::string nom)
	{
		if (buffer == nullptr)
		{
			Debug::Warn("Le buffer n'existe pas");
			return nullptr;
		}
		Model * Mesh = new Model(buffer, m_models.size(), vulkanM);
		Mesh->setName(nom);
		Mesh->setMaterial(MaterialManager::getDefaultMaterial());
		m_models.push_back(Mesh);
		vulkanM->str_VulkanDescriptor->modelCount = m_models.size();
		updateDescriptor();
		return Mesh;
	}

	void ModelManager::destroyModel(Model *model)
	{
		m_destroyElement = true;
		m_destroymodels.erase(std::remove(m_destroymodels.begin(), m_destroymodels.end(), model), m_destroymodels.end());
		m_destroymodels.push_back(model);
		vulkanM->str_VulkanDescriptor->recreateCommandBuffer = true;
	}

	void ModelManager::destroyBuffer(ShapeBuffer *buffer)
	{
		m_destroyElement = true;
		m_destroyshapeBuffers.erase(std::remove(m_destroyshapeBuffers.begin(), m_destroyshapeBuffers.end(), buffer), m_destroyshapeBuffers.end());
		m_destroyshapeBuffers.push_back(buffer);
		vulkanM->str_VulkanDescriptor->recreateCommandBuffer = true;
	}

	void ModelManager::destroyElement()
	{		
		if (m_destroyElement)
		{
			for (int j = 0; j < m_destroymodels.size(); j++)
			{
				m_models.erase(std::remove(m_models.begin(), m_models.end(), m_destroymodels[j]), m_models.end());
				delete (m_destroymodels[j]);				
			}

			for (int j = 0; j < m_destroyshapeBuffers.size(); j++)
			{
				for (int i = 0; i < m_models.size(); i++)
				{
					if (m_models[i]->getShapeBuffer() == m_destroyshapeBuffers[j])
					{
						Model * m = m_models[i];
						m_models.erase(std::remove(m_models.begin(), m_models.end(), m), m_models.end());
						delete (m);
						i--;
					}
				}
				m_shapeBuffers.erase(std::remove(m_shapeBuffers.begin(), m_shapeBuffers.end(), m_destroyshapeBuffers[j]), m_shapeBuffers.end());
				delete (m_destroyshapeBuffers[j]);
			}

			m_destroyshapeBuffers.clear();
			m_destroymodels.clear();
			for (int i = 0; i < m_models.size(); i++)
			{
				m_models[i]->setIndexUbo(i);
			}
			vulkanM->str_VulkanDescriptor->modelCount = m_models.size();
			updateDescriptor();
			m_destroyElement = false;
		}
	}

	std::vector<Model *> ModelManager::GetModels()
	{
		return m_models;
	}

	void ModelManager::initDescriptor(VulkanMisc * vM)
	{
		if (m_descriptor == nullptr)
		{
			m_descriptor = new Descriptor(vM, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
		}
	}

	void ModelManager::printModelInfo(const char *path)
	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		glm::vec3 normalResult;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path))
		{
			Debug::Warn("%s  %s", nullptr, warn.c_str(), err.c_str());
		}

		std::unordered_map<Vertex, uint32_t> uniqueVertices{};

		for (const auto &shape : shapes)
		{
			Debug::Log("%s", path);
			for (const auto &index : shape.mesh.indices)
			{
				Vertex vertex{};

				vertex.pos = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2] };

				vertex.texCoord = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1] };

				vertex.normal = {
					attrib.normals[3 * index.normal_index + 0],
					attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 2] };

				if (uniqueVertices.count(vertex) == 0)
				{
					uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.push_back(vertex);
				}

				indices.push_back(uniqueVertices[vertex]);
			}
		}
		Debug::Info("POS: %d", vertices.size());
		for (int i = 0; i < vertices.size(); i++)
		{
			std::cout << vertices[i].pos.x << "," << vertices[i].pos.y << "," << vertices[i].pos.z << ",";
		}
		std::cout << std::endl;
		Debug::Info("TEXCORD: %d", vertices.size());
		for (int i = 0; i < vertices.size(); i++)
		{
			std::cout << vertices[i].texCoord.x << "," << vertices[i].texCoord.y << ",";
		}
		std::cout << std::endl;
		Debug::Info("NORMAL: %d", vertices.size());
		for (int i = 0; i < vertices.size(); i++)
		{
			std::cout << vertices[i].normal.x << "," << vertices[i].normal.y << "," << vertices[i].normal.z << ",";
		}
		std::cout << std::endl;
		Debug::Info("INDICE: %d", indices.size());
		for (int i = 0; i < indices.size(); i++)
		{
			std::cout << indices[i] << ",";
		}
		std::cout << std::endl;
	}

	ShapeBuffer * ModelManager::allocateBuffer(float * pos, float * texCord, float * normal, unsigned int * indice, unsigned vertexSize, unsigned indiceSize)
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		for (int i = 0; i < vertexSize; i++)
		{
			Vertex vertex{};

			vertex.pos = {
				pos[3 * i + 0],
				pos[3 * i + 1],
				pos[3 * i + 2] };

			vertex.texCoord = {
				texCord[2 * i + 0],
				texCord[2 * i + 0] };

			vertex.normal = {
				normal[3 * i + 0],
				normal[3 * i + 1],
				normal[3 * i + 2] };
			vertices.push_back(vertex);
		}
		for (int i = 0; i < indiceSize; i++)
		{
			indices.push_back(indice[i]);
		}		
		ShapeBuffer *buffer = new ShapeBuffer(vertices, indices, vulkanM);
		m_shapeBuffers.push_back(buffer);
		return buffer;
	}

	ShapeBuffer *ModelManager::allocateBuffer(const char *path)
	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		glm::vec3 normalResult;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path))
		{
			Debug::Warn("%s  %s", nullptr, warn.c_str(), err.c_str());
			return nullptr;
		}
		
		std::unordered_map<Vertex, uint32_t> uniqueVertices{};

		for (const auto &shape : shapes)
		{
			for (const auto &index : shape.mesh.indices)
			{
				Vertex vertex{};
				
				vertex.pos = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]};

				vertex.texCoord = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1]};

				vertex.normal = {
					attrib.normals[3 * index.normal_index + 0],
					attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 2]};

				if (uniqueVertices.count(vertex) == 0)
				{
					uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.push_back(vertex);					
				}

				indices.push_back(uniqueVertices[vertex]);
			}
		}
		ShapeBuffer *buffer = new ShapeBuffer(vertices, indices, vulkanM);
		m_shapeBuffers.push_back(buffer);
		return buffer;
	}
}