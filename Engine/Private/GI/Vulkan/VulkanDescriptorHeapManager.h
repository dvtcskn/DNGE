
#pragma once

#include "VulkanDevice.h"

class ResourceViewHeaps
{
public:
	ResourceViewHeaps(VulkanDevice* device, uint32_t cbvDescriptorCount, uint32_t srvDescriptorCount, uint32_t uavDescriptorCount, uint32_t samplerDescriptorCount);
	~ResourceViewHeaps();

	bool AllocDescriptor(VkDescriptorSetLayout descriptorLayout, VkDescriptorSet* pDescriptor);
	bool AllocDescriptor(int size, const VkSampler* pSamplers, VkDescriptorSetLayout* descriptorLayout, VkDescriptorSet* pDescriptor);
	bool AllocDescriptor(std::vector<uint32_t>& descriptorCounts, const VkSampler* pSamplers, VkDescriptorSetLayout* descriptorLayout, VkDescriptorSet* pDescriptor);
	bool CreateDescriptorSetLayout(std::vector<VkDescriptorSetLayoutBinding>* pDescriptorLayoutBinding, VkDescriptorSetLayout* pDescSetLayout);
	bool CreateDescriptorSetLayoutAndAllocDescriptorSet(std::vector<VkDescriptorSetLayoutBinding>* pDescriptorLayoutBinding, VkDescriptorSetLayout* descriptorLayout, VkDescriptorSet* pDescriptor);
	void FreeDescriptor(VkDescriptorSet descriptorSet);

private:
	VulkanDevice* Device;
	VkDescriptorPool m_descriptorPool;
	std::mutex       m_mutex;
	std::int32_t     m_allocatedDescriptorCount = 0;
};
