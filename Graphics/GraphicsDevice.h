#ifndef GRAPHICS_DEVICE_H
#define GRAPHICS_DEVICE_H

#include <exception>
#include <optional>
#include <vector>
#include <array>
#include <source_location>
#include <string>
#include <memory>
#include <format>
#include <set>
#include <limits>
#include <algorithm>
#include <fstream>
#include <filesystem>

#ifdef GFX_REFAC
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>

namespace clm {
	class GfxDevice {
	public:
		GfxDevice();
		~GfxDevice();

		GfxDevice(const GfxDevice&) = delete;
		GfxDevice(GfxDevice&&) = delete;
		GfxDevice& operator=(const GfxDevice&) = delete;
		GfxDevice& operator=(GfxDevice&&) = delete;
	private:
		static VkBool32 debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT,
									   VkDebugUtilsMessageTypeFlagsEXT,
									   const VkDebugUtilsMessengerCallbackDataEXT*,
									   void*);
		vk::Instance m_instance;
		VkDebugUtilsMessengerEXT m_debugMessenger;
		vk::PhysicalDevice m_physicalDevice;
		vk::Device m_device;
	};
}
#else
#include <clmMath/clm_vector.h>
#include <clmMath/clm_rect.h>
#include <clmUtil/clm_system.h>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

#include "Font.h"
#include "BuildSystemInput.h"

// 
#ifdef SHADER_DIR_PATH
constexpr std::string_view sourceDirectory{SHADER_DIR_PATH};
#else
constexpr std::string_view sourceDirectory{"./"};
#endif
template<size_t n>
constexpr std::string get_shader_path(const char (&name)[n])
{
	return std::format("{}{}{}", sourceDirectory, "/""Shaders/", name);
}

const std::vector<const char*> g_validationLayers{
	"VK_LAYER_KHRONOS_validation"
};
const std::vector<const char*> g_deviceExtensions{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef _DEBUG
constexpr bool enableValidationLayers = true;
#else
constexpr bool enableValidationLayers = false;
#endif
constexpr const size_t MAX_FRAMES_IN_FLIGHT = 2;

std::vector<char> read_file(const std::string& filename);

namespace clm::gfx {
	struct Vertex {
		point_t position;
		clm::math::Point3f color;

		static VkVertexInputBindingDescription get_binding_description()
		{
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindingDescription;
		}

		static std::array<VkVertexInputAttributeDescription, 2> get_attribute_description()
		{
			std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(Vertex, position);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(Vertex, color);

			return attributeDescriptions;
		}
	};
}

namespace clm {
	class GraphicsDevice {
	public:
		GraphicsDevice(HWND, const math::Rect_t&);
		~GraphicsDevice();
		GraphicsDevice(const GraphicsDevice&) = delete;
		GraphicsDevice& operator=(const GraphicsDevice&) = delete;
		GraphicsDevice(GraphicsDevice&&) = delete;
		GraphicsDevice& operator=(GraphicsDevice&&) = delete;
		void draw_triangles(const std::vector<font_triangle_t>&);
		void draw_frame();

		void resize_buffer(const math::Rect_t&);
	private:
		const HWND m_hwnd;
		math::Rect_t m_windowDimensions;
		VkInstance m_instance;
		VkDebugUtilsMessengerEXT m_debugMessenger;
		VkPhysicalDevice m_physicalDevice;
		VkDevice m_device;
		VkQueue m_graphicsQueue;
		VkQueue m_presentQueue;
		VkSurfaceKHR m_surface;
		VkSwapchainKHR m_swapChain;
		std::vector<VkImage> m_swapChainImages;
		VkFormat m_swapChainImageFormat;
		VkExtent2D m_swapChainExtent;
		std::vector<VkImageView> m_swapChainImageViews;
		VkRenderPass m_renderPass;
		VkPipelineLayout m_pipelineLayout;
		VkPipeline m_graphicsPipeline;
		std::vector<VkFramebuffer> m_swapChainFramebuffers;
		VkCommandPool m_commandPool;
		std::vector<VkCommandBuffer> m_commandBuffers;
		std::vector<VkSemaphore> m_imageAvailableSemaphores;
		std::vector<VkSemaphore> m_renderFinishedSemaphores;
		std::vector<VkFence> m_inFlightFences;
		std::vector<VkFence> m_imagesInFlight;
		size_t m_currentFrame;
		bool m_framebufferResized;

		bool m_updateVertexBuffer;
		std::vector<gfx::Vertex> m_vertices;
		VkBuffer m_vertexBuffer;
		VkDeviceMemory m_vertexBufferMemory;

		struct QueueFamilyIndices {
			std::optional<std::uint32_t> graphicsFamily;
			std::optional<std::uint32_t> presentFamily;

			bool is_complete()
			{
				return graphicsFamily.has_value() && presentFamily.has_value();
			}
			std::vector<uint32_t> get_queue_family_indices() const noexcept
			{
				std::vector<uint32_t> ret{};
				if (graphicsFamily.has_value())
				{
					ret.push_back(*graphicsFamily);
				}
				if (presentFamily.has_value())
				{
					ret.push_back(*presentFamily);
				}
				return ret;
			}
		};
		struct SwapChainSupportDetails {
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector<VkSurfaceFormatKHR> formats;
			std::vector<VkPresentModeKHR> presentModes;
		};

		void create_instance();
		VkResult create_debug_utils_messenger_ext(VkInstance instance,
												  const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
												  const VkAllocationCallbacks* pAllocator,
												  VkDebugUtilsMessengerEXT* pDebugMessenger);
		void destroy_debug_utils_messenger_ext(VkInstance,
											   VkDebugUtilsMessengerEXT,
											   const VkAllocationCallbacks*);
		void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT&);
		void setup_debug_messenger();
		bool check_validation_layer_support();
		void pick_physical_device();
		bool is_device_suitable(VkPhysicalDevice);
		bool check_device_extension_support(VkPhysicalDevice);
		void create_logical_device();
		QueueFamilyIndices find_queue_families(VkPhysicalDevice);
		SwapChainSupportDetails query_swapchain_support(VkPhysicalDevice);
		VkSurfaceFormatKHR choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>&);
		VkPresentModeKHR choose_swap_present_mode(const std::vector<VkPresentModeKHR>&);
		VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR&);
		void create_swapchain();
		void create_image_views();
		void create_render_pass();
		uint32_t find_memory_type(uint32_t, VkMemoryPropertyFlags);
		void create_vertex_buffer();
		void create_graphics_pipeline();
		VkShaderModule create_shader_module(const std::vector<char>& code);
		void create_framebuffers();
		void create_command_pool();
		void create_command_buffers();
		void create_sync_objects();
		void recreate_swapchain();
		void cleanup_swapchain();

		void destroy_vertex_buffer();

		static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback([[maybe_unused]] VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
															 [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageType,
															 [[maybe_unused]] const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
															 [[maybe_unused]] void* pUserData)
		{
			std::string callbackOut = std::format("Validation layer: {}\n", pCallbackData->pMessage);
			OutputDebugStringA(callbackOut.c_str());

			return VK_FALSE;
		}
	};
}
#endif
#endif