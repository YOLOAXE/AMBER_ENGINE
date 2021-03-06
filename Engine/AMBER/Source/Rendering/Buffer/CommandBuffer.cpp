#include "CommandBuffer.hpp"
#include "GraphiquePipelineManager.hpp"
#include "ModelManager.hpp"
#include  "Descriptor.hpp"
#include "SkyboxManager.hpp"

namespace Ge
{

	bool CommandBuffer::initialize(VulkanMisc * vM, ptrClass * ptrC)
	{
		vulkanM = vM;
		std::vector<VkFramebuffer> swapChainFramebuffers = vM->str_VulkanCommandeBufferMisc->str_swapChainFramebuffers;
		std::vector< GraphiquePipeline * > all_pipeline = GraphiquePipelineManager::GetPipelines();
		std::vector<Model *> all_models = ModelManager::GetModels();
		std::vector<std::vector<VkDescriptorSet>> tab_Descriptor;
		std::vector<Descriptor *> all_descriptor = Descriptor::GetAllDescriptor();
		tab_Descriptor.resize(vM->str_VulkanSwapChainMisc->str_swapChainImages.size());
		for (int i = 0; i < vM->str_VulkanSwapChainMisc->str_swapChainImages.size(); i++)
		{
			for (int j = 0; j < all_descriptor.size(); j++)
			{
				tab_Descriptor[i].push_back(all_descriptor[j]->getDescriptorSets()[i]);
			}
		}
		m_commandBuffers.resize(swapChainFramebuffers.size());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = vM->str_VulkanCommandeBufferMisc->str_commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)m_commandBuffers.size();

		if (vkAllocateCommandBuffers(vM->str_VulkanDeviceMisc->str_device, &allocInfo, m_commandBuffers.data()) != VK_SUCCESS)
		{
			Debug::Error("Echec de l'allocation du command buffers");
			return false;
		}

		for (size_t i = 0; i < m_commandBuffers.size(); i++)
		{
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			std::vector<Model *> all_models = ModelManager::GetModels();
			if (vkBeginCommandBuffer(m_commandBuffers[i], &beginInfo) != VK_SUCCESS)
			{
				Debug::Error("Echec de l'enregistrer du commande buffer");
				return false;
			}

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = vM->str_VulkanSwapChainMisc->str_renderPass;
			renderPassInfo.framebuffer = swapChainFramebuffers[i];
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = vM->str_VulkanSwapChainMisc->str_swapChainExtent;

			std::array<VkClearValue, 2> clearValues{};
			clearValues[0].color = { ptrC->settingManager->getClearColor().r, ptrC->settingManager->getClearColor().g, ptrC->settingManager->getClearColor().b, ptrC->settingManager->getClearColor().a };
			clearValues[1].depthStencil = { 1.0f, 0 };
			VkDeviceSize offsets[] = { 0 };

			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();

			vkCmdBeginRenderPass(m_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			Skybox * sky = SkyboxManager::GetCurrentSkybox();
			if (sky != nullptr)
			{
				for (int p = 0; p < all_pipeline.size(); p++)
				{
					if (sky->getIndexPipeline() == all_pipeline[p]->getIndex())
					{
						SkyboxManager::GetCurrentSkybox()->render(m_commandBuffers[i], all_pipeline[p], tab_Descriptor[i], VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
					}
				}
			}
			for (int p = 0; p < all_pipeline.size();p++)
			{
				vkCmdBindPipeline(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, all_pipeline[p]->m_graphiquePipelineElement.m_graphicsPipeline);
				for (int m = 0; m < all_models.size();m++)
				{
					if (all_models[m]->getMaterial()->getPipelineIndex() == all_pipeline[p]->getIndex())
					{
						all_models[m]->render(m_commandBuffers[i], tab_Descriptor[i], all_pipeline[p]->m_graphiquePipelineElement.m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
						//all_models.erase(iterm->second); //TODO a verifier si on ne peut pas erase les model pour pas repasser dessus attention il y a 3 command buffer
					}
				}
			}

			vkCmdEndRenderPass(m_commandBuffers[i]);

			if (vkEndCommandBuffer(m_commandBuffers[i]) != VK_SUCCESS)
			{
				Debug::Error("Echec de l'enregistrer du commande buffer");
				return false;
			}
		}
		vM->str_VulkanCommandeBufferMisc->str_commandBuffers = m_commandBuffers;

		Debug::INITSUCCESS("CommandBuffers");
		return true;
	}

	void CommandBuffer::release()
	{
		vkFreeCommandBuffers(vulkanM->str_VulkanDeviceMisc->str_device, vulkanM->str_VulkanCommandeBufferMisc->str_commandPool, static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());
		Debug::RELEASESUCCESS("CommandBuffers");
	}

}