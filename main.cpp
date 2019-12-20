#include "vulkan/vulkan.hpp"
#include <algorithm>
#include <iostream>
#include <optional>
#include <set>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <fstream>
#ifndef NDEBUG
#define NDEBUG 0
#pragma warning("NDEBUG Didn't exist priror to compile!!")
#endif

static std::vector<char> read_file(const std::string &filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("failed ot open " + filename);
    }

    size_t file_size = (size_t)file.tellg();
    std::vector<char> buffer(file_size);
    file.seekg(0);
    file.read(buffer.data(), file_size);
    file.close();

    return buffer;
}

class Renderer
{
  private:
    void InitWindow();
    void InitVulkan();
    void SetupDebugMessenger();
    bool CheckValidationLaterSupport();
    void CreateInstance();
    void MainLoop();
  void DrawFrame();
    void Cleanup();
  void CleanupSwapChain();
    void PickPhysicalDevice();
    void CreateLogicalDevice();
    void CreateSurface();
    void CreateSwapChain();
    void CreateImageViews();
    void CreateGraphicsPipeline();
    void CreateRenderPass();
    void CreateFrameBuffers();
  void CreateCommandPool();
  void CreateCommandBuffers();
  void CreateSemaphores();
  void RecreateSwapChain();
  static void FramebufferResizeCB(GLFWwindow* window, int width, int height);
    //
    GLFWwindow *window = nullptr;
    VkInstance instance = VK_NULL_HANDLE;
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    VkDevice logical_device = VK_NULL_HANDLE;
    VkQueue graphics_queue = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debug_messenger;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkQueue present_queue = VK_NULL_HANDLE;
    VkSwapchainKHR swap_chain = VK_NULL_HANDLE;
    VkFormat swap_chain_image_format;
    VkExtent2D swap_chain_extent;
    VkRenderPass render_pass;
    VkPipelineLayout pipeline_layout;
    VkPipeline graphics_pipeline;
  VkCommandPool command_pool;
  std::vector<VkSemaphore> image_available_semaphores;
  std::vector<VkSemaphore> render_finished_semaphores;
  std::vector<VkFence> in_flight_fences;
  size_t current_frame = 0;
  std::vector<VkImage> swap_chain_images;
    std::vector<VkImageView> swap_chain_image_views;
  std::vector<VkFramebuffer> swap_chain_framebuffers;
  std::vector<VkCommandBuffer> command_buffers;

  const int max_frames_in_flight = 2;
  bool framebuffer_resized = false;

  public:
    Renderer();
    ~Renderer();
    void run()
    {
        InitWindow();
        InitVulkan();
        std::cout << "Begin Drawing!" << std::endl;
        MainLoop();
        Cleanup();
    }
    static const unsigned int window_width = 400;
    static const unsigned int window_height = 300;
    const bool enable_validation_layers = NDEBUG ? false : true;
    const std::vector<const char *> validation_layers = {
        // I appear to have very old vulkan libs or something
        // The below line should be valid.
        // "VK_LAYER_KHRONOS_validation"
        "VK_LAYER_LUNARG_standard_validation"};

    const std::vector<const char *> device_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

};

void Renderer::RecreateSwapChain()
{
  int width = 0, height = 0;
  while (width == 0 || height == 0)
    {
      glfwGetFramebufferSize(window, &width, &height);
      glfwWaitEvents();
    }
  vkDeviceWaitIdle(logical_device);

  CreateSwapChain();
  CreateImageViews();
  CreateRenderPass();
  CreateRenderPass();
  CreateGraphicsPipeline();
  CreateFrameBuffers();
  CreateCommandBuffers();
}

void CleanupSwapChain()
{
  
}

struct QueueFamilyIndices
{
  std::optional<uint32_t> graphics_family;
  std::optional<uint32_t> present_family;
  bool IsComplete()
  {
    return graphics_family.has_value() && present_family.has_value();
  }
};

QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice physical_device, VkSurfaceKHR surface);

void Renderer::DrawFrame()
{
  vkWaitForFences(logical_device, 1, &in_flight_fences[current_frame], VK_TRUE, std::numeric_limits<uint64_t>::max());

  uint32_t image_index;
  VkResult result = vkAcquireNextImageKHR(logical_device, swap_chain, std::numeric_limits<uint64_t>::max(), image_available_semaphores[current_frame], VK_NULL_HANDLE, &image_index);

  if(result == VK_ERROR_OUT_OF_DATE_KHR || framebuffer_resized)
    {
      framebuffer_resized = false;
      RecreateSwapChain();
      return;
    }
  else if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
      throw std::runtime_error("failed to acquire swap chain image!");
    }

  VkSubmitInfo submit_info = {};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  VkSemaphore wait_semaphores[] = {image_available_semaphores[current_frame]};
  VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submit_info.waitSemaphoreCount = 1;
  submit_info.pWaitSemaphores = wait_semaphores;
  submit_info.pWaitDstStageMask = wait_stages;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &command_buffers[image_index];

  VkSemaphore signal_semaphores[] = {render_finished_semaphores[current_frame]};
  submit_info.signalSemaphoreCount = 1;
  submit_info.pSignalSemaphores = signal_semaphores;

  vkResetFences(logical_device, 1, &in_flight_fences[current_frame]);

  if(vkQueueSubmit(graphics_queue, 1, &submit_info, in_flight_fences[current_frame]) != VK_SUCCESS)
    {
      throw std::runtime_error("failed to submit draw command buffer!");
    }
  VkPresentInfoKHR present_info = {};
  present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  present_info.waitSemaphoreCount = 1;
  present_info.pWaitSemaphores = signal_semaphores;
  VkSwapchainKHR swap_chains[] = {swap_chain};
  present_info.swapchainCount = 1;
  present_info.pSwapchains = swap_chains;
  present_info.pImageIndices = &image_index;
  present_info.pResults = nullptr;

  
  result = vkQueuePresentKHR(present_queue, &present_info);
  if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
      RecreateSwapChain();
    }
  else if(result != VK_SUCCESS)
    {
      throw std::runtime_error("failed to present swap chain image!");
    }
  current_frame = (current_frame+1)%max_frames_in_flight;
}

void Renderer::CreateSurface()
{
    std::cout << "Create Surface" << std::endl;
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create window surface!");
    }
}

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                    VkDebugUtilsMessageTypeFlagsEXT messagetype,
                                                    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                    void *pUserData)
{
    std::cerr << "error: [Vulkan]: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}
bool Renderer::CheckValidationLaterSupport()
{
    uint32_t layer_count = 0;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
    std::vector<VkLayerProperties> available_layers(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());
    for (const char *layer_name : validation_layers)
    {
        std::cout << "finding: " << layer_name << std::endl;
        bool layer_found = false;
        for (const auto &layer_properties : available_layers)
        {
            std::cout << "\tcomparing: " << layer_properties.layerName << std::endl;
            if (strcmp(layer_name, layer_properties.layerName) == 0)
            {
                layer_found = true;
                break;
            }
        }
        std::cout << std::endl;
        if (layer_found == false)
        {
            std::cout << "RUH ROH" << std::endl;
            return false;
        }
    }
    return true;
}
Renderer::Renderer()
{
}
Renderer::~Renderer()
{
}
void Renderer::FramebufferResizeCB(GLFWwindow* window, int width, int height)
{
  auto renderer = reinterpret_cast<Renderer*>(glfwGetWindowUserPointer(window));
  renderer->framebuffer_resized = true;
  std::cout << "window resized!" << std::endl;
}
void Renderer::InitWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(window_width, window_height, "Vulkanism", nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, FramebufferResizeCB);
    if (window == nullptr)
    {
        glfwTerminate();
    }
}



void Renderer::InitVulkan()
{
    CreateInstance();
    SetupDebugMessenger();
    if (window != nullptr)
    {
        CreateSurface();
    }
    PickPhysicalDevice();
    CreateLogicalDevice();
    CreateSwapChain();
    CreateImageViews();
    CreateRenderPass();
    CreateGraphicsPipeline();
    CreateFrameBuffers();
    CreateCommandPool();
    CreateCommandBuffers();
    CreateSemaphores();

}
void Renderer::CreateSemaphores()
{
  std::cout << "CreateSemaphores" << std::endl;

  image_available_semaphores.resize(max_frames_in_flight);
  render_finished_semaphores.resize(max_frames_in_flight);
  in_flight_fences.resize(max_frames_in_flight);
  
  VkSemaphoreCreateInfo semaphore_create_info = {};
  semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fence_create_info = {};
  fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for(int i = 0; i < max_frames_in_flight; i++)
    {
      if(vkCreateSemaphore(logical_device, &semaphore_create_info, nullptr, &image_available_semaphores[i]) != VK_SUCCESS)
        {
          throw std::runtime_error("failed to create semaphore!");
        }

      if(vkCreateSemaphore(logical_device, &semaphore_create_info, nullptr, &render_finished_semaphores[i]) != VK_SUCCESS)
        {
          throw std::runtime_error("failed to create semaphore!");
        }

      if(vkCreateFence(logical_device, &fence_create_info, nullptr, &in_flight_fences[i]) != VK_SUCCESS)
        {
          throw std::runtime_error("failed to create semaphore!");
        }
    }

}
void Renderer::CreateCommandBuffers()
{
    std::cout << "CreateCommandBuffers" << std::endl;
  command_buffers.resize(swap_chain_framebuffers.size());

  VkCommandBufferAllocateInfo alloc_info = {};
  alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  alloc_info.commandPool = command_pool;
  alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  alloc_info.commandBufferCount = (uint32_t)command_buffers.size();

  if(vkAllocateCommandBuffers(logical_device, &alloc_info, command_buffers.data()) != VK_SUCCESS)
    {
      throw std::runtime_error("failed to allocate command buffers!");
    }
  for(size_t i = 0; i < command_buffers.size(); i++)
    {
      VkCommandBufferBeginInfo begin_info = {};
      begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
      begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
      begin_info.pInheritanceInfo = nullptr;

      if(vkBeginCommandBuffer(command_buffers[i], &begin_info) != VK_SUCCESS)
        {
          throw std::runtime_error("failed to begin recording command buffer!");
        }

      VkRenderPassBeginInfo render_pass_begin_info = {};
      render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
      render_pass_begin_info.renderPass = render_pass;
      render_pass_begin_info.framebuffer = swap_chain_framebuffers[i];
      render_pass_begin_info.renderArea.offset = {0,0};
      render_pass_begin_info.renderArea.extent = swap_chain_extent;

      VkClearValue clear_color = {0.0f, 0.0f, 0.0f, 1.0f};
      render_pass_begin_info.clearValueCount = 1;
      render_pass_begin_info.pClearValues = &clear_color;

      vkCmdBeginRenderPass(command_buffers[i], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
      vkCmdBindPipeline(command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline);
      vkCmdDraw(command_buffers[i], 3, 1, 0, 0);
      vkCmdEndRenderPass(command_buffers[i]);
      if(vkEndCommandBuffer(command_buffers[i]) != VK_SUCCESS)
        {
          throw std::runtime_error("failed to record command buffer!");
        }
    }
}
void Renderer::CreateCommandPool()
{
  QueueFamilyIndices queue_family_indices = FindQueueFamilies(physical_device, surface);
  VkCommandPoolCreateInfo pool_create_info = {};
  pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  pool_create_info.queueFamilyIndex = queue_family_indices.graphics_family.value();
  pool_create_info.flags = 0;

  if(vkCreateCommandPool(logical_device, &pool_create_info, nullptr, &command_pool) != VK_SUCCESS)
    {
      throw std::runtime_error("failed to create command pool!");
    }

}
void Renderer::CreateFrameBuffers()
{
  swap_chain_framebuffers.resize(swap_chain_image_views.size());
  for (size_t i = 0; i < swap_chain_image_views.size(); i++)
    {
      VkImageView attachments[] = { swap_chain_image_views[i] };
      VkFramebufferCreateInfo framebuffer_create_info = {};
      framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
      framebuffer_create_info.renderPass = render_pass;
      framebuffer_create_info.attachmentCount = 1;
      framebuffer_create_info.pAttachments = attachments;
      framebuffer_create_info.width = swap_chain_extent.width;
      framebuffer_create_info.height = swap_chain_extent.height;
      framebuffer_create_info.layers = 1;

      if(vkCreateFramebuffer(logical_device, &framebuffer_create_info, nullptr, &swap_chain_framebuffers[i]) != VK_SUCCESS)
        {
          throw std::runtime_error("failed to create framebuffer!");
        }
    }
}
void Renderer::CreateRenderPass()
{
    std::cout << "CreateRenderPass" << std::endl;
    VkAttachmentDescription color_attachement = {};
    color_attachement.format = swap_chain_image_format;
    color_attachement.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachement.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachement.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachement.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachement.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachement.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachement.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachemnt_reference = {};
    color_attachemnt_reference.attachment = 0;
    color_attachemnt_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass_description = {};
    subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass_description.colorAttachmentCount = 1;
    subpass_description.pColorAttachments = &color_attachemnt_reference;

    VkSubpassDependency subpass_dependency = {};
    subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dependency.dstSubpass = 0;
    subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo render_pass_create_info = {};
    render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_create_info.attachmentCount = 1;
    render_pass_create_info.pAttachments = &color_attachement;
    render_pass_create_info.subpassCount = 1;
    render_pass_create_info.pSubpasses = &subpass_description;
    render_pass_create_info.dependencyCount = 1;
    render_pass_create_info.pDependencies = &subpass_dependency;

    if (vkCreateRenderPass(logical_device, &render_pass_create_info, nullptr, &render_pass) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create render pass!");
    }
}
VkShaderModule CreateShaderModule(const std::vector<char> &code, VkDevice logical_device)
{
    std::cout << "CreateShaderModule" << std::endl;
    VkShaderModuleCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = code.size();
    create_info.pCode = reinterpret_cast<const uint32_t *>(code.data());

    VkShaderModule shader_module;
    if (vkCreateShaderModule(logical_device, &create_info, nullptr, &shader_module) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create shader module!");
    }
    return shader_module;
}
void Renderer::CreateGraphicsPipeline()
{
    std::cout << "CreateGraphicsPipeline" << std::endl;
    auto vert_shader_code = read_file("vert.spv");
    auto frag_shader_code = read_file("frag.spv");

    VkShaderModule vert_shader_module = CreateShaderModule(vert_shader_code, logical_device);
    VkShaderModule frag_shader_module = CreateShaderModule(frag_shader_code, logical_device);

    VkPipelineShaderStageCreateInfo vert_stage_create_info = {};
    vert_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_stage_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vert_stage_create_info.module = vert_shader_module;
    vert_stage_create_info.pName = "main";

    VkPipelineShaderStageCreateInfo frag_stage_create_info = {};
    frag_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_stage_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_stage_create_info.module = frag_shader_module;
    frag_stage_create_info.pName = "main";

    VkPipelineShaderStageCreateInfo stage_create_info[] = {vert_stage_create_info, frag_stage_create_info};

    VkPipelineVertexInputStateCreateInfo vertex_input_create_info = {};
    vertex_input_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_create_info.vertexBindingDescriptionCount = 0;
    vertex_input_create_info.pVertexBindingDescriptions = nullptr;
    vertex_input_create_info.vertexAttributeDescriptionCount = 0;
    vertex_input_create_info.pVertexAttributeDescriptions = nullptr;

    VkPipelineInputAssemblyStateCreateInfo input_create_info = {};
    input_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    input_create_info.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swap_chain_extent.width;
    viewport.height = (float)swap_chain_extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = swap_chain_extent;

    VkPipelineViewportStateCreateInfo viewport_state_create_info = {};
    viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state_create_info.viewportCount = 1;
    viewport_state_create_info.pViewports = &viewport;
    viewport_state_create_info.scissorCount = 1;
    viewport_state_create_info.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasteriser_create_info = {};
    rasteriser_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasteriser_create_info.depthClampEnable = VK_FALSE;
    rasteriser_create_info.polygonMode = VK_POLYGON_MODE_FILL;
    rasteriser_create_info.lineWidth = 1.0f;
    rasteriser_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
    rasteriser_create_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasteriser_create_info.depthBiasEnable = VK_FALSE;
    rasteriser_create_info.depthBiasConstantFactor = 0.0f;
    rasteriser_create_info.depthBiasClamp = 0.0f;
    rasteriser_create_info.depthBiasSlopeFactor = 0.0f;

    VkPipelineMultisampleStateCreateInfo multisample_create_info = {};
    multisample_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample_create_info.sampleShadingEnable = VK_FALSE;
    multisample_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisample_create_info.minSampleShading = 1.0f;
    multisample_create_info.pSampleMask = nullptr;
    multisample_create_info.alphaToCoverageEnable = VK_FALSE;
    multisample_create_info.alphaToOneEnable = VK_FALSE;

    // create depth stencil here if you want...

    VkPipelineColorBlendAttachmentState color_blend_attachment_state = {};
    color_blend_attachment_state.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment_state.blendEnable = VK_FALSE;
    // blend is disabled so these options do nothing
    // they're just an example of some simple blending parameters.
    color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;
    //

    VkPipelineColorBlendStateCreateInfo color_blending_create_info = {};
    color_blending_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending_create_info.logicOpEnable = VK_FALSE;
    // logic op is disabled so this line is optional.
    color_blending_create_info.logicOp = VK_LOGIC_OP_COPY;
    //
    color_blending_create_info.attachmentCount = 1;
    color_blending_create_info.pAttachments = &color_blend_attachment_state;
    // optional constants. I think these are actually used, they're just initialised to zero by defualt.
    color_blending_create_info.blendConstants[0] = 0.0f;
    color_blending_create_info.blendConstants[1] = 0.0f;
    color_blending_create_info.blendConstants[2] = 0.0f;
    color_blending_create_info.blendConstants[3] = 0.0f;

    VkDynamicState dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_LINE_WIDTH};

    VkPipelineDynamicStateCreateInfo dynamic_state_create_info = {};
    dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state_create_info.dynamicStateCount = 2;
    dynamic_state_create_info.pDynamicStates = dynamic_states;

    VkPipelineLayoutCreateInfo pipeline_layout_create_info = {};
    pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_create_info.setLayoutCount = 0;
    pipeline_layout_create_info.pSetLayouts = nullptr;
    pipeline_layout_create_info.pushConstantRangeCount = 0;
    pipeline_layout_create_info.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(logical_device, &pipeline_layout_create_info, nullptr, &pipeline_layout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipeline_create_info = {};
    pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_create_info.stageCount = 2;
    // TODO: rename this to shader_stage_create_info;
    pipeline_create_info.pStages = stage_create_info;
    pipeline_create_info.pVertexInputState = &vertex_input_create_info;
    pipeline_create_info.pInputAssemblyState = &input_create_info;
    pipeline_create_info.pViewportState = &viewport_state_create_info;
    pipeline_create_info.pRasterizationState = &rasteriser_create_info;
    pipeline_create_info.pMultisampleState = &multisample_create_info;
    pipeline_create_info.pDepthStencilState = nullptr;
    pipeline_create_info.pColorBlendState = &color_blending_create_info;
    pipeline_create_info.pDynamicState = nullptr;
    pipeline_create_info.layout = pipeline_layout;
    pipeline_create_info.renderPass = render_pass;
    pipeline_create_info.subpass = 0;
    pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
    pipeline_create_info.basePipelineIndex = -1;

    if (vkCreateGraphicsPipelines(logical_device, VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr,
                                  &graphics_pipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(logical_device, vert_shader_module, nullptr);
    vkDestroyShaderModule(logical_device, frag_shader_module, nullptr);
}

void Renderer::CreateImageViews()
{
    std::cout << "Create Image Views" << std::endl;
    swap_chain_image_views.resize(swap_chain_images.size());

    for (size_t i = 0; i < swap_chain_images.size(); i++)
    {
        VkImageViewCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        create_info.image = swap_chain_images[i];
        create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        create_info.format = swap_chain_image_format;

        create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        create_info.subresourceRange.baseMipLevel = 0;
        create_info.subresourceRange.levelCount = 1;
        create_info.subresourceRange.baseArrayLayer = 0;
        create_info.subresourceRange.layerCount = 1;

        if (vkCreateImageView(logical_device, &create_info, nullptr, &swap_chain_image_views[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create image viaews!");
        }
    }
}

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilites;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> present_modes;
};

SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice physical_device, VkSurfaceKHR surface)
{
    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &details.capabilites);
    uint32_t format_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, nullptr);

    if (format_count != 0)
    {
        details.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, details.formats.data());
    }
    uint32_t present_mode_count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, nullptr);
    if (present_mode_count)
    {
        details.present_modes.resize(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count,
                                                  details.present_modes.data());
    }
    return details;
}
void Renderer::CreateLogicalDevice()
{
    std::cout << "Create Logical Device" << std::endl;
    QueueFamilyIndices indices = FindQueueFamilies(physical_device, surface);

    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    std::set<uint32_t> unique_queue_families = {indices.graphics_family.value(), indices.present_family.value()};
    float queue_priority = 1.0f;
    for (uint32_t queue_family : unique_queue_families)
    {
        VkDeviceQueueCreateInfo queue_create_info = {};
        queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.queueFamilyIndex = queue_family;
        queue_create_info.queueCount = 1;

        queue_create_info.pQueuePriorities = &queue_priority;
        queue_create_infos.push_back(queue_create_info);
    }
    VkPhysicalDeviceFeatures device_features = {};
    VkDeviceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.pQueueCreateInfos = queue_create_infos.data();
    create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
    create_info.pEnabledFeatures = &device_features;
    create_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
    create_info.ppEnabledExtensionNames = device_extensions.data();

    if (enable_validation_layers)
    {
        create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
        create_info.ppEnabledLayerNames = validation_layers.data();
    }
    else
    {
        create_info.enabledLayerCount = 0;
    }

    if (vkCreateDevice(physical_device, &create_info, nullptr, &logical_device) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create logical device!");
    }

    vkGetDeviceQueue(logical_device, indices.graphics_family.value(), 0, &graphics_queue);
    vkGetDeviceQueue(logical_device, indices.present_family.value(), 0, &present_queue);
}
void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo)
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = DebugCallback;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                      const VkAllocationCallbacks *pAllocator,
                                      VkDebugUtilsMessengerEXT *pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks *pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(instance, debugMessenger, pAllocator);
    }
}

void Renderer::SetupDebugMessenger()
{
    if (!enable_validation_layers)
        return;
    VkDebugUtilsMessengerCreateInfoEXT create_info = {};
    PopulateDebugMessengerCreateInfo(create_info);

    if (CreateDebugUtilsMessengerEXT(instance, &create_info, nullptr, &debug_messenger) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to setup debug messenger!");
    }
}

QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice physical_device, VkSurfaceKHR surface)
{
    QueueFamilyIndices indices;
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());

    int i = 0;
    for (const auto &queue_family : queue_families)
    {
        if (queue_family.queueCount > 0 && queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphics_family = i;
        }
        
        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &present_support);
        if (present_support)
        {
            indices.present_family = i;
        }
        
        if (indices.IsComplete())
        {
            break;
        }
        i++;
    }
    return indices;
}
// VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window)
// {
//   if(capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
//     {
//       return capabilities.currentExtent;
//     }
//   else
//     {
//       int width, height;
//       glfwGetFramebufferSize(window, &width, &height);

//       VkExtent2D actual_extent =
//         {
//          static_cast<uint32_t>(width),
//          static_cast<uint32_t>(height)
//         };
//     }
// }
bool CheckDeviceExtensionSupport(VkPhysicalDevice physical_device, const std::vector<const char *> &device_extensions)
{
    uint32_t extension_count = 0;
    vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, nullptr);
    std::vector<VkExtensionProperties> extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, extensions.data());
    std::set<std::string> required_extensions(device_extensions.begin(), device_extensions.end());
    // std::cout << "device extensions: " << std::endl;
    for (const auto &extension : extensions)
    {
        // std::cout << "\t" << extension.extensionName << std::endl;
        required_extensions.erase(extension.extensionName);
    }
    // std::cout << std::endl;

    return required_extensions.empty();
}
VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> available_formats)
{
    for (const auto &available_format : available_formats)
    {
        if (available_format.format == VK_FORMAT_B8G8R8A8_UNORM &&
            available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return available_format;
        }
    }
    return available_formats[0];
}
VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &available_present_modes)
{
    VkPresentModeKHR best_mode = VK_PRESENT_MODE_FIFO_KHR;
    for (const auto &available_present_mode : available_present_modes)
    {
        if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return available_present_mode;
        }
        else if (available_present_mode == VK_PRESENT_MODE_IMMEDIATE_KHR)
        {
            best_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
        }
    }
    return best_mode;
}
VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities, GLFWwindow* window)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    else
    {
      int width, height;
      glfwGetFramebufferSize(window, &width, &height);

      VkExtent2D actual_extent =
        {
         static_cast<uint32_t>(width),
         static_cast<uint32_t>(height)
        };
        actual_extent.width = std::max(capabilities.minImageExtent.width,
                                       std::min(capabilities.maxImageExtent.width, actual_extent.width));
        actual_extent.height = std::max(capabilities.minImageExtent.height,
                                        std::min(capabilities.maxImageExtent.height, actual_extent.height));

        return actual_extent;
    }
}
void Renderer::CreateSwapChain()
{
    std::cout << "Create Swap Chain" << std::endl;
    SwapChainSupportDetails swap_chain_support = QuerySwapChainSupport(physical_device, surface);
    VkSurfaceFormatKHR surface_format = ChooseSwapSurfaceFormat(swap_chain_support.formats);
    VkPresentModeKHR present_mode = ChooseSwapPresentMode(swap_chain_support.present_modes);
    VkExtent2D extent = ChooseSwapExtent(swap_chain_support.capabilites, window);
    uint32_t image_count = swap_chain_support.capabilites.minImageCount + 1;

    if (swap_chain_support.capabilites.maxImageCount > 0 && image_count > swap_chain_support.capabilites.maxImageCount)
    {
        image_count = swap_chain_support.capabilites.maxImageCount;
    }

    VkSwapchainCreateInfoKHR create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = surface;
    create_info.minImageCount = image_count;
    create_info.imageFormat = surface_format.format;
    create_info.imageColorSpace = surface_format.colorSpace;
    create_info.imageExtent = extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = FindQueueFamilies(physical_device, surface);
    uint32_t queue_family_indices[] = {indices.graphics_family.value(), indices.present_family.value()};

    if (indices.graphics_family != indices.present_family)
    {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queue_family_indices;
    }
    else
    {
      std::cout << "graphics_family is present_family" << std::endl;
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        create_info.queueFamilyIndexCount = 0;
        create_info.pQueueFamilyIndices = nullptr;
    }
    create_info.preTransform = swap_chain_support.capabilites.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = present_mode;
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(logical_device, &create_info, nullptr, &swap_chain) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(logical_device, swap_chain, &image_count, nullptr);
    swap_chain_images.resize(image_count);
    vkGetSwapchainImagesKHR(logical_device, swap_chain, &image_count, swap_chain_images.data());

    swap_chain_image_format = surface_format.format;
    swap_chain_extent = extent;
}

bool IsDeviceSuitable(VkPhysicalDevice physical_device, VkSurfaceKHR surface,
                      const std::vector<const char *> &device_extensions)
{
    VkPhysicalDeviceProperties device_properties;
    vkGetPhysicalDeviceProperties(physical_device, &device_properties);

    VkPhysicalDeviceFeatures device_features;
    vkGetPhysicalDeviceFeatures(physical_device, &device_features);

    QueueFamilyIndices indices = FindQueueFamilies(physical_device, surface);
    bool extensions_supported = CheckDeviceExtensionSupport(physical_device, device_extensions);
    bool suitable_device = indices.IsComplete();
    suitable_device = suitable_device && device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
    suitable_device = suitable_device && device_features.geometryShader;
    suitable_device = suitable_device && extensions_supported;
    bool swap_chain_adequate = false;
    if (extensions_supported)
    {

        SwapChainSupportDetails swap_chain_support = QuerySwapChainSupport(physical_device, surface);
        swap_chain_adequate = !swap_chain_support.formats.empty();
        swap_chain_adequate = swap_chain_adequate && !swap_chain_support.present_modes.empty();
    }

    if (!suitable_device)
    {
        std::cout << "\t" << device_properties.deviceName << " failed suitability test." << std::endl;
    }
    return suitable_device;
}

void Renderer::PickPhysicalDevice()
{
    std::cout << "Pick Physical Device:" << std::endl;
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(instance, &device_count, nullptr);
    if (device_count == 0)
    {
        throw std::runtime_error("failed to find GPUs with Vulkan supported!!");
    }
    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(instance, &device_count, devices.data());

    for (const auto &device : devices)
    {
        if (IsDeviceSuitable(device, surface, device_extensions))
        {
            physical_device = device;
            break;
        }
    }
    if (physical_device == VK_NULL_HANDLE)
    {
        std::runtime_error("Failed to find suitable a GPU!");
    }
    else
    {
        VkPhysicalDeviceProperties device_properties;
        vkGetPhysicalDeviceProperties(physical_device, &device_properties);
        std::cout << "\t" << device_properties.deviceName << " chosen as physical device." << std::endl;
    }
}
void Renderer::CreateInstance()
{
    std::cout << "Create Instance" << std::endl;
    if (enable_validation_layers && !CheckValidationLaterSupport())
    {
        throw std::runtime_error("validation layers requested, but not supported!");
    }
    // app info
    VkApplicationInfo app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "Vulkanism";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "No engine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_0;
    // create info
    VkInstanceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;
    
    VkDebugUtilsMessengerCreateInfoEXT create_debug_info = {};
    if (enable_validation_layers)
    {
        create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
        create_info.ppEnabledLayerNames = validation_layers.data();
        PopulateDebugMessengerCreateInfo(create_debug_info);
        create_info.pNext = &create_debug_info;
    }
    else
    {
        create_info.enabledLayerCount = 0;
        create_info.pNext = nullptr;
    }
    //
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char *> required_extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    if (enable_validation_layers)
    {
        required_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    create_info.enabledExtensionCount = static_cast<uint32_t>(required_extensions.size());
    create_info.ppEnabledExtensionNames = required_extensions.data();
    create_info.enabledLayerCount = 0;

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    // std::cout << "available extensions:" << std::endl;
    // for (const auto &extension : extensions)
    // {
    //     std::cout << "\t" << extension.extensionName << std::endl;
    // }
    // std::cout << std::endl;
    // VK_KHR_wayland_surface is supported, consider using that.

    VkResult result = vkCreateInstance(&create_info, nullptr, &instance);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create vk instance!");
    }
}
void Renderer::MainLoop()
{
    if (window != nullptr)
    {
        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();
            DrawFrame();
        }
    }
    vkDeviceWaitIdle(logical_device);
}
void Renderer::CleanupSwapChain()
{
  vkFreeCommandBuffers(logical_device, command_pool, static_cast<uint32_t>(command_buffers.size()), command_buffers.data());

  for(auto framebuffer : swap_chain_framebuffers)
    {
      vkDestroyFramebuffer(logical_device, framebuffer, nullptr);
    }
  vkDestroyRenderPass(logical_device, render_pass, nullptr);
  vkDestroyPipeline(logical_device, graphics_pipeline, nullptr);
  vkDestroyPipelineLayout(logical_device, pipeline_layout, nullptr);
  for (auto image_view : swap_chain_image_views)
    {
      vkDestroyImageView(logical_device, image_view, nullptr);
    }
  vkDestroySwapchainKHR(logical_device, swap_chain, nullptr);
}
void Renderer::Cleanup()
{
    std::cout << "clean up!!" << std::endl;
    CleanupSwapChain();
    for(int i = 0; i < max_frames_in_flight; i++)
      {
        vkDestroySemaphore(logical_device, image_available_semaphores[i], nullptr);
        vkDestroySemaphore(logical_device, render_finished_semaphores[i], nullptr);
        vkDestroyFence(logical_device, in_flight_fences[i], nullptr);
      }
    vkDestroyCommandPool(logical_device, command_pool, nullptr);
    vkDestroyDevice(logical_device, nullptr);
    if (enable_validation_layers)
    {
        DestroyDebugUtilsMessengerEXT(instance, debug_messenger, nullptr);
    }
    if (window != nullptr)
    {
        vkDestroySurfaceKHR(instance, surface, nullptr);
    }
    vkDestroyInstance(instance, nullptr);
    if (window != nullptr)
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }
}

int main()
{
    Renderer renderer;
    try
    {
        renderer.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
