
#include "pch.h"
#include "VulkanDescriptorHeapManager.h"

ResourceViewHeaps::ResourceViewHeaps(VulkanDevice* device, uint32_t cbvDescriptorCount, uint32_t srvDescriptorCount, uint32_t uavDescriptorCount, uint32_t samplerDescriptorCount)
    : Device(device)
{
    m_allocatedDescriptorCount = 0;

    const VkDescriptorPoolSize type_count[] =
    {
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 16384 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 16384 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 16384 },    // Texture
        { VK_DESCRIPTOR_TYPE_SAMPLER, 2048 },                   
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 16384 }            // UAV
    };

    VkDescriptorPoolCreateInfo descriptor_pool = {};
    descriptor_pool.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptor_pool.pNext = NULL;
    descriptor_pool.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    descriptor_pool.maxSets = 8000;
    descriptor_pool.poolSizeCount = _countof(type_count);
    descriptor_pool.pPoolSizes = type_count;

    VkResult res = vkCreateDescriptorPool(Device->Get(), &descriptor_pool, NULL, &m_descriptorPool);
    assert(res == VK_SUCCESS);

    Device->SetResourceName(VK_OBJECT_TYPE_DESCRIPTOR_POOL, (uint64_t)m_descriptorPool, "ResourceViewHeap");
}

ResourceViewHeaps::~ResourceViewHeaps()
{
    vkDestroyDescriptorPool(Device->Get(), m_descriptorPool, NULL);
}

bool ResourceViewHeaps::AllocDescriptor(VkDescriptorSetLayout descLayout, VkDescriptorSet* pDescriptorSet)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    VkDescriptorSetAllocateInfo alloc_info;
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.pNext = NULL;
    alloc_info.descriptorPool = m_descriptorPool;
    alloc_info.descriptorSetCount = 1;
    alloc_info.pSetLayouts = &descLayout;

    VkResult res = vkAllocateDescriptorSets(Device->Get(), &alloc_info, pDescriptorSet);
    assert(res == VK_SUCCESS);

    m_allocatedDescriptorCount++;

    return res == VK_SUCCESS;
}

bool ResourceViewHeaps::AllocDescriptor(int size, const VkSampler* pSamplers, VkDescriptorSetLayout* pDescSetLayout, VkDescriptorSet* pDescriptorSet)
{
    std::vector<VkDescriptorSetLayoutBinding> layoutBindings(size);
    for (int i = 0; i < size; i++)
    {
        layoutBindings[i].binding = i;
        layoutBindings[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        layoutBindings[i].descriptorCount = 1;
        layoutBindings[i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        layoutBindings[i].pImmutableSamplers = (pSamplers != NULL) ? &pSamplers[i] : NULL;
    }

    return CreateDescriptorSetLayoutAndAllocDescriptorSet(&layoutBindings, pDescSetLayout, pDescriptorSet);
}

bool ResourceViewHeaps::AllocDescriptor(std::vector<uint32_t>& descriptorCounts, const VkSampler* pSamplers, VkDescriptorSetLayout* pDescSetLayout, VkDescriptorSet* pDescriptorSet)
{
    std::vector<VkDescriptorSetLayoutBinding> layoutBindings(descriptorCounts.size());
    for (int i = 0; i < descriptorCounts.size(); i++)
    {
        layoutBindings[i].binding = i;
        layoutBindings[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        layoutBindings[i].descriptorCount = descriptorCounts[i];
        layoutBindings[i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        layoutBindings[i].pImmutableSamplers = (pSamplers != NULL) ? &pSamplers[i] : NULL;
    }

    return CreateDescriptorSetLayoutAndAllocDescriptorSet(&layoutBindings, pDescSetLayout, pDescriptorSet);
}

bool ResourceViewHeaps::CreateDescriptorSetLayout(std::vector<VkDescriptorSetLayoutBinding>* pDescriptorLayoutBinding, VkDescriptorSetLayout* pDescSetLayout)
{
    // Next take layout bindings and use them to create a descriptor set layout

    VkDescriptorSetLayoutCreateInfo descriptor_layout = {};
    descriptor_layout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptor_layout.pNext = NULL;
    descriptor_layout.bindingCount = (uint32_t)pDescriptorLayoutBinding->size();
    descriptor_layout.pBindings = pDescriptorLayoutBinding->data();

    VkResult res = vkCreateDescriptorSetLayout(Device->Get(), &descriptor_layout, NULL, pDescSetLayout);
    assert(res == VK_SUCCESS);
    return (res == VK_SUCCESS);
}

bool ResourceViewHeaps::CreateDescriptorSetLayoutAndAllocDescriptorSet(std::vector<VkDescriptorSetLayoutBinding>* pDescriptorLayoutBinding, VkDescriptorSetLayout* pDescSetLayout, VkDescriptorSet* pDescriptorSet)
{
    // Next take layout bindings and use them to create a descriptor set layout

    VkDescriptorSetLayoutCreateInfo descriptor_layout = {};
    descriptor_layout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptor_layout.pNext = NULL;
    descriptor_layout.bindingCount = (uint32_t)pDescriptorLayoutBinding->size();
    descriptor_layout.pBindings = pDescriptorLayoutBinding->data();

    VkResult res = vkCreateDescriptorSetLayout(Device->Get(), &descriptor_layout, NULL, pDescSetLayout);
    assert(res == VK_SUCCESS);

    return AllocDescriptor(*pDescSetLayout, pDescriptorSet);
}

void ResourceViewHeaps::FreeDescriptor(VkDescriptorSet descriptorSet)
{
    m_allocatedDescriptorCount--;
    vkFreeDescriptorSets(Device->Get(), m_descriptorPool, 1, &descriptorSet);
}
