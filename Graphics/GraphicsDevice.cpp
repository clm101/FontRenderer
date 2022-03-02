#include <GraphicsDevice.h>
#include <gfxUtil.h>

#include <string>
#include <format>
#include <tuple>
#include <iostream>
#include <array>

#include <clmUtil/clm_err.h>

#include "Delaunay.h"
#include "BuildSystemInput.h"

#ifdef GFX_REFAC
namespace clm {
	static std::vector<const char*> g_layerNames{"VK_LAYER_KHRONOS_validation"};
	static std::vector<const char*> g_extensionNames{"VK_EXT_debug_utils", "VK_EXT_validation_features", "VK_KHR_surface", "VK_KHR_win32_surface"};
	static std::array<std::pair<vk::QueueFlagBits, const char*>, 5> g_queueFlags{std::pair{vk::QueueFlagBits::eCompute, "Compute"},
																				 std::pair{vk::QueueFlagBits::eGraphics, "Graphics"},
																				 std::pair{vk::QueueFlagBits::eProtected, "Protected"},
																				 std::pair{vk::QueueFlagBits::eSparseBinding, "Sparse Binding"},
																				 std::pair{vk::QueueFlagBits::eTransfer, "Transfer"}};

	VkBool32 GfxDevice::debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
							  VkDebugUtilsMessageTypeFlagsEXT messageTypes,
							  const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
							  void* userData)
	{
		if (callbackData)
		{
			std::string messageIDInfo{std::format("{}: {}", 
												  callbackData->messageIdNumber, 
												  callbackData->pMessageIdName)};
			switch (messageTypes)
			{
			case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
			{
				std::cout << vkutil::table("General Message",
										   vkutil::entry("Message ID", messageIDInfo),
										   vkutil::entry("Message", callbackData->pMessage));
				break;
			}
			case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
			{
				std::cout << vkutil::table("Validation Message",
										   vkutil::entry("Message ID", messageIDInfo),
										   vkutil::entry("Message", callbackData->pMessage));
				break;
			}
			case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
			{
				std::cout << vkutil::table("Performance Message",
										   vkutil::entry("Message ID", messageIDInfo),
										   vkutil::entry("Message", callbackData->pMessage));
				break;
			}
			default:
				std::cout << vkutil::table("Unknown/Composite Message",
										   vkutil::entry("Message ID", messageIDInfo),
										   vkutil::entry("Message", callbackData->pMessage));
			};
		}
		else
		{
			std::cout << "Debug callback called, but no data provided.\n";
		}

		return VK_FALSE;
	}

	GfxDevice::GfxDevice()
		:
		m_instance(),
		m_physicalDevice(),
		m_device(),
		m_debugMessenger()
	{
		uint32_t apiVersion{};
		check_vk_ret(vk::enumerateInstanceVersion(&apiVersion),
					 "Failed to get API version");


		//check_vk_ret(vkEnumerateInstanceVersion(&apiVersion),
		//			 "Failed to get requested API version.");

		//uint32_t layerCount{};
		//check_vk_ret(vkEnumerateInstanceLayerProperties(&layerCount,
		//												nullptr),
		//			 "Failed to get layer count");
		//std::vector<VkLayerProperties> layerProperties{layerCount};
		//check_vk_ret(vkEnumerateInstanceLayerProperties(&layerCount,
		//												layerProperties.data()),
		//			 "Failed to get layer properties");
#ifdef VULKAN_INIT_INFO
		{
			std::vector<vk::ExtensionProperties> defaultInstanceExtensions = vk::enumerateInstanceExtensionProperties();
			std::string defaultInstanceExtensionEntries{};
			for (const auto& extension : defaultInstanceExtensions)
			{
				defaultInstanceExtensionEntries += vkutil::entry("Name", extension.extensionName);
				defaultInstanceExtensionEntries += vkutil::entry("Spec Version", extension.specVersion);
			}
			std::cout << vkutil::table("Default Instance Extensions", defaultInstanceExtensionEntries) << '\n';
		}

		std::vector<vk::LayerProperties> layerProperties = vk::enumerateInstanceLayerProperties();
		for (const auto& layerProperty : layerProperties)
		{
			/*uint32_t propertyCount{};
			check_vk_ret(vkEnumerateInstanceExtensionProperties(layerProperty.layerName,
																&propertyCount,
																nullptr),
						 "Failed to get count of instance extension properties");
			std::vector<VkExtensionProperties> extensionProperties{propertyCount};
			check_vk_ret(vkEnumerateInstanceExtensionProperties(layerProperty.layerName,
																&propertyCount,
																extensionProperties.data()),
						 "Failed to get instance extension properties");*/
			std::string layerName = layerProperty.layerName;
			std::vector<vk::ExtensionProperties> extensionProperties = vk::enumerateInstanceExtensionProperties(layerName);
			std::string title{"Layer Property"};
			std::string instanceExtensionsTitle{"Instance extensions"};
			std::string instanceExtensionEntries{};
			if (extensionProperties.size() == 0)
			{
				instanceExtensionEntries = "None";
			}
			else
			{
				for (const auto& extensionProperty : extensionProperties)
				{
					instanceExtensionEntries += vkutil::entry("Name", extensionProperty.extensionName);
					instanceExtensionEntries += vkutil::entry("Spec Version", vkutil::MAKE_VERSION_STRING(extensionProperty.specVersion));
				}
			}


			std::string output = vkutil::table(title,
											   vkutil::entry("Name", layerProperty.layerName),
											   vkutil::entry("Spec Version", vkutil::MAKE_VERSION_STRING(layerProperty.specVersion)),
											   vkutil::entry("Implementation Version", layerProperty.implementationVersion),
											   vkutil::entry("Description", layerProperty.description),
											   vkutil::table(instanceExtensionsTitle, instanceExtensionEntries));
			std::cout << output << '\n';
		}
#endif
		VkDebugUtilsMessengerCreateInfoEXT debugUtilsCreateInfo{};
		debugUtilsCreateInfo.sType = static_cast<VkStructureType>(vk::StructureType::eDebugUtilsMessengerCreateInfoEXT);
		debugUtilsCreateInfo.pNext = nullptr;
		debugUtilsCreateInfo.flags = 0;
		debugUtilsCreateInfo.messageSeverity = (static_cast<VkDebugUtilsMessageSeverityFlagBitsEXT>(vk::DebugUtilsMessageSeverityFlagBitsEXT::eError) |
												static_cast<VkDebugUtilsMessageSeverityFlagBitsEXT>(vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo) |
												static_cast<VkDebugUtilsMessageSeverityFlagBitsEXT>(vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning) |
												static_cast<VkDebugUtilsMessageSeverityFlagBitsEXT>(vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose));
		debugUtilsCreateInfo.messageType = (static_cast<VkDebugUtilsMessageTypeFlagBitsEXT>(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral) |
											static_cast<VkDebugUtilsMessageTypeFlagBitsEXT>(vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation) |
											static_cast<VkDebugUtilsMessageTypeFlagBitsEXT>(vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance));
		debugUtilsCreateInfo.pfnUserCallback = GfxDevice::debug_callback;
		debugUtilsCreateInfo.pUserData = nullptr;

		vk::ApplicationInfo applicationInfo{};
		applicationInfo.pNext = nullptr;
		applicationInfo.pApplicationName = "Font renderer";
		applicationInfo.applicationVersion = vkutil::MAKE_VERSION<0, 1, 0, 0>();
		applicationInfo.pEngineName = nullptr;
		applicationInfo.engineVersion = 0;
		applicationInfo.apiVersion = apiVersion;

		vk::InstanceCreateInfo instanceCreateInfo{};
		instanceCreateInfo.pNext = &debugUtilsCreateInfo;
		instanceCreateInfo.pApplicationInfo = &applicationInfo;
		instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(g_extensionNames.size());
		instanceCreateInfo.ppEnabledExtensionNames = g_extensionNames.data();
		instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(g_layerNames.size());
		instanceCreateInfo.ppEnabledLayerNames = g_layerNames.data();
		m_instance = vk::createInstance(instanceCreateInfo);

		void* createDebugUtilsVoidFunc = vkGetInstanceProcAddr(m_instance,
															   "vkCreateDebugUtilsMessengerEXT");
		if (createDebugUtilsVoidFunc)
		{
			auto createDebugUtilsFunc = static_cast<PFN_vkCreateDebugUtilsMessengerEXT>(createDebugUtilsVoidFunc);
			check_vk_ret(createDebugUtilsFunc(m_instance,
											  &debugUtilsCreateInfo,
											  nullptr,
											  &m_debugMessenger),
						 "Failed to create debug utils messenger.");
		}
		else
		{
			throw std::runtime_error{"Failed to get debug utils create function"};
		}
		
		std::string physicalDeviceTables{};
		std::vector<vk::PhysicalDevice> physicalDevices = m_instance.enumeratePhysicalDevices();
#ifdef VULKAN_INIT_INFO
		for (size_t j = 0; j < physicalDevices.size(); j += 1)
		{
			const auto& physicalDevice = physicalDevices[j];
			vk::StructureChain physicalDeviceProperties = physicalDevice.getProperties2<vk::PhysicalDeviceProperties2,
																						vk::PhysicalDeviceVulkan12Properties,
																						vk::PhysicalDeviceVulkan11Properties>();
			const vk::PhysicalDeviceVulkan11Properties& vulkan11DeviceProperties = physicalDeviceProperties.get<vk::PhysicalDeviceVulkan11Properties>();
			const vk::PhysicalDeviceVulkan12Properties& vulkan12DeviceProperties = physicalDeviceProperties.get<vk::PhysicalDeviceVulkan12Properties>();
			std::vector<uint8_t> pipelineCacheUUID = std::vector<uint8_t>(VK_UUID_SIZE);
			for (size_t k = 0; k < VK_UUID_SIZE; k += 1)
			{
				pipelineCacheUUID[k] = physicalDeviceProperties.get<vk::PhysicalDeviceProperties2>().properties.pipelineCacheUUID[k];
			}

			std::string deviceExtensionTables{};
			for (const auto& layer : layerProperties)
			{
				std::vector<vk::ExtensionProperties> deviceExtensions = physicalDevice.enumerateDeviceExtensionProperties(std::string(layer.layerName.data()));
				std::string deviceExtensionEntries{};
				if (deviceExtensions.size() == 0)
				{
					deviceExtensionEntries = "None";
				}
				else
				{
					for (const auto& deviceExtension : deviceExtensions)
					{
						deviceExtensionEntries += vkutil::entry("Name", deviceExtension.extensionName);
						deviceExtensionEntries += vkutil::entry("Spec Version", vkutil::MAKE_VERSION_STRING(deviceExtension.specVersion));
					}
				}
				deviceExtensionTables += vkutil::table(std::string(layer.layerName.data()),
													   deviceExtensionEntries);
			}

			std::string deviceQueueFamilyTables{};
			std::vector<vk::QueueFamilyProperties2> deviceQueueFamilyProperties = physicalDevice.getQueueFamilyProperties2();
			for (size_t k = 0; k < deviceQueueFamilyProperties.size(); k += 1)
			{
				const auto& familyProperty = deviceQueueFamilyProperties[k].queueFamilyProperties;
				std::string queueFlagEntry{};
				for (const auto& queueFlag : g_queueFlags)
				{
					if (vk::Flags{familyProperty.queueFlags}&
						vk::Flags{queueFlag.first})
					{
						queueFlagEntry += std::format("{} | ", queueFlag.second);
					}
				}
				queueFlagEntry.erase(queueFlagEntry.size() - 3, 3);

				deviceQueueFamilyTables += vkutil::table(std::format("Queue {}", k),
														 vkutil::entry("Type", queueFlagEntry),
														 vkutil::entry("Count", familyProperty.queueCount),
														 vkutil::entry("Timestamp Valid Bits", familyProperty.timestampValidBits),
														 vkutil::entry("Minimum Image Transfer Granularity", std::format("({}, {}, {})",
																														 familyProperty.minImageTransferGranularity.width,
																														 familyProperty.minImageTransferGranularity.height,
																														 familyProperty.minImageTransferGranularity.depth)));
			}

			physicalDeviceTables += vkutil::table(std::format("Physical Device {}", j),
												  vkutil::entry("Name", physicalDeviceProperties.get<vk::PhysicalDeviceProperties2>().properties.deviceName),
												  vkutil::entry("API Version", vkutil::MAKE_VERSION_STRING(physicalDeviceProperties.get<vk::PhysicalDeviceProperties2>().properties.apiVersion)),
												  vkutil::entry("Driver Version", physicalDeviceProperties.get<vk::PhysicalDeviceProperties2>().properties.driverVersion),
												  vkutil::entry("Vendor ID", physicalDeviceProperties.get<vk::PhysicalDeviceProperties2>().properties.vendorID),
												  vkutil::entry("Device ID", physicalDeviceProperties.get<vk::PhysicalDeviceProperties2>().properties.deviceID),
												  //vkutil::entry("Device Type", vkutil::translate_physical_device_type(physicalDeviceProperties.properties.deviceType)),
												  vkutil::entry("Pipeline Cache UUID", vkutil::list_uuid(pipelineCacheUUID)),
												  vkutil::table("Device extensions", deviceExtensionTables),
												  vkutil::table("Queue Families", deviceQueueFamilyTables));
		}

		std::cout << vkutil::table("Physical Devices", physicalDeviceTables) << '\n';
#endif
	}

	GfxDevice::~GfxDevice()
	{
		void* destroyDebugUtilsVoidFunc = vkGetInstanceProcAddr(m_instance,
																"vkDestroyDebugUtilsMessengerEXT");
		if (destroyDebugUtilsVoidFunc)
		{
			auto destroyDebugUtilsFunc = static_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(destroyDebugUtilsVoidFunc);
			destroyDebugUtilsFunc(m_instance,
								  m_debugMessenger,
								  nullptr);

		}
		m_instance.destroy();
	}
}
#else
std::vector<char> read_file(const std::string& filename)
{
	//auto fsystem = std::filesystem::current_path();
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		throw std::runtime_error("Failed to open file.");
	}

	size_t fileSize = static_cast<size_t>(file.tellg());
	std::vector<char> buffer(fileSize, '\0');
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();
	return buffer;
}

namespace clm {
	GraphicsDevice::GraphicsDevice(HWND hWnd, const math::Rect_t& windowDim)
		:
		m_hwnd(hWnd),
		m_windowDimensions(windowDim),
		m_currentFrame(0),
		m_framebufferResized(false),
		m_updateVertexBuffer(false)
	{
		create_instance();
		setup_debug_messenger();
		// Create surface
		VkWin32SurfaceCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		createInfo.hwnd = hWnd;
		createInfo.hinstance = GetModuleHandle(nullptr);
		err::check_ret_val(vkCreateWin32SurfaceKHR(m_instance, &createInfo, nullptr, &m_surface),
						   "Failed to create window surface.");
		m_vertices.push_back({{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}});
		m_vertices.push_back({{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}});
		m_vertices.push_back({{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}});

		pick_physical_device();
		create_logical_device();
		create_swapchain();
		create_image_views();
		create_render_pass();
		create_graphics_pipeline();
		create_framebuffers();
		create_command_pool();
		create_vertex_buffer();
		create_command_buffers();
		create_sync_objects();
	}

	GraphicsDevice::~GraphicsDevice()
	{
		vkDeviceWaitIdle(m_device);
		cleanup_swapchain();
		destroy_vertex_buffer();
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroySemaphore(m_device, m_renderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(m_device, m_imageAvailableSemaphores[i], nullptr);
			vkDestroyFence(m_device, m_inFlightFences[i], nullptr);
		}
		vkDestroyCommandPool(m_device, m_commandPool, nullptr);
		vkDestroyDevice(m_device, nullptr);
		if (enableValidationLayers)
		{
			destroy_debug_utils_messenger_ext(m_instance, m_debugMessenger, nullptr);
		}
		vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
		vkDestroyInstance(m_instance, nullptr);
	}

	void GraphicsDevice::create_instance()
	{
		if (enableValidationLayers && !check_validation_layer_support())
		{
			throw err::VulkanException{"Validation layers requested, but not available."};
		}
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Hello Triangle";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		std::vector<const char*> extensionNames = {"VK_KHR_surface",
			"VK_KHR_win32_surface",
			"VK_EXT_debug_utils"};
		if (enableValidationLayers)
		{
			extensionNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}
		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = static_cast<std::uint32_t>(extensionNames.size());
		createInfo.ppEnabledExtensionNames = extensionNames.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		if (enableValidationLayers)
		{
			createInfo.enabledLayerCount = static_cast<std::uint32_t>(g_validationLayers.size());
			createInfo.ppEnabledLayerNames = g_validationLayers.data();

			populate_debug_messenger_create_info(debugCreateInfo);
			createInfo.pNext = static_cast<VkDebugUtilsMessengerCreateInfoEXT*>(&debugCreateInfo);
		}
		else
		{
			createInfo.enabledLayerCount = 0;
			createInfo.pNext = nullptr;
		}

		err::check_ret_val(vkCreateInstance(&createInfo, nullptr, &m_instance),
						   "Failed to create instance");

		std::uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> extensions{extensionCount};
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
		OutputDebugStringA("available extensions:\n");
		for (const auto& extension : extensions)
		{
			std::string extensionOut = std::format("\t{}\n", extension.extensionName);
			OutputDebugStringA(extensionOut.c_str());
		}
	}

	VkResult GraphicsDevice::create_debug_utils_messenger_ext(VkInstance instance,
															  const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
															  const VkAllocationCallbacks* pAllocator,
															  VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance,
																							   "vkCreateDebugUtilsMessengerEXT"));
		if (func != nullptr)
		{
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void GraphicsDevice::destroy_debug_utils_messenger_ext(VkInstance instance,
														   VkDebugUtilsMessengerEXT debugMessenger,
														   const VkAllocationCallbacks* pAllocator)
	{
		auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance,
																								"vkDestroyDebugUtilsMessengerEXT"));
		if (func != nullptr)
		{
			func(instance, debugMessenger, pAllocator);
		}
	}

	void GraphicsDevice::populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
	{
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debug_callback;
		createInfo.pUserData = nullptr;
	}

	void GraphicsDevice::setup_debug_messenger()
	{
		if (!enableValidationLayers) return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		populate_debug_messenger_create_info(createInfo);

		err::check_ret_val(create_debug_utils_messenger_ext(m_instance,
															&createInfo,
															nullptr,
															&m_debugMessenger),
						   "Failed to set up debug messenger");
	}

	bool GraphicsDevice::check_validation_layer_support()
	{
		std::uint32_t layerCount = 0;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : g_validationLayers)
		{
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers)
			{
				if (std::strcmp(layerName, layerProperties.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}

			if (!layerFound)
			{
				return false;
			}
		}

		return true;
	}

	void GraphicsDevice::pick_physical_device()
	{
		std::uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
		if (deviceCount == 0)
		{
			throw err::VulkanException{"Failed to find GPUs with Vulkan support",
										std::source_location::current()};
		}
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());
		for (const auto& device : devices)
		{
			if (is_device_suitable(device))
			{
				m_physicalDevice = device;
				break;
			}
		}
		if (m_physicalDevice == VK_NULL_HANDLE)
		{
			throw err::VulkanException{"Failed to find a suitable GPU",
										std::source_location::current()};
		}
	}

	bool GraphicsDevice::is_device_suitable(VkPhysicalDevice device)
	{
		QueueFamilyIndices indices = find_queue_families(device);
		bool extensionsSupported = check_device_extension_support(device);
		bool swapChainAdequate = false;
		if (extensionsSupported)
		{
			SwapChainSupportDetails swapChainSupport = query_swapchain_support(device);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}
		return indices.is_complete() && extensionsSupported && swapChainAdequate;
	}

	bool GraphicsDevice::check_device_extension_support(VkPhysicalDevice device)
	{
		std::uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device,
											 nullptr,
											 &extensionCount,
											 availableExtensions.data());

		std::set<std::string> requiredExtensions(g_deviceExtensions.begin(), g_deviceExtensions.end());

		for (const auto& extension : availableExtensions)
		{
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	GraphicsDevice::QueueFamilyIndices GraphicsDevice::find_queue_families(VkPhysicalDevice device)
	{
		QueueFamilyIndices indices;
		std::uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		std::uint32_t i = 0;
		VkBool32 presentSupport = 0;
		for (const auto& queueFamily : queueFamilies)
		{
			if ((queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) == VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphicsFamily = i;
			}

			presentSupport = 0;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentSupport);
			if (presentSupport != 0)
			{
				indices.presentFamily = i;
			}

			if (indices.is_complete())
			{
				break;
			}

			i++;
		}
		return indices;
	}

	void GraphicsDevice::create_logical_device()
	{
		QueueFamilyIndices indices = find_queue_families(m_physicalDevice);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
		std::set<std::uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(),
														indices.presentFamily.value()};
		float queuePriority = 1.0f;
		for (std::uint32_t queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}


		VkPhysicalDeviceFeatures deviceFeatures{};

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = static_cast<std::uint32_t>(g_deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = g_deviceExtensions.data();
		createInfo.queueCreateInfoCount = static_cast<std::uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		/*
		* Not needed for up-to-date Vulkan implementations.
		* enabledLayerCount and ppEnabledLayerNames are deprecated and no longer used,
		* according to the Vulkan spec
		*/
		if (enableValidationLayers)
		{
			createInfo.enabledLayerCount = static_cast<std::uint32_t>(g_validationLayers.size());
			createInfo.ppEnabledLayerNames = g_validationLayers.data();
		}
		else
		{
			createInfo.enabledLayerCount = 0;
		}

		err::check_ret_val(vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device),
						   "Failed to create logical device.");

		vkGetDeviceQueue(m_device, indices.graphicsFamily.value(), 0, &m_graphicsQueue);
		vkGetDeviceQueue(m_device, indices.presentFamily.value(), 0, &m_presentQueue);
	}

	GraphicsDevice::SwapChainSupportDetails GraphicsDevice::query_swapchain_support(VkPhysicalDevice device)
	{
		SwapChainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &details.capabilities);

		std::uint32_t formatCount = 0;;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, nullptr);
		if (formatCount != 0)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device,
												 m_surface,
												 &formatCount,
												 details.formats.data());
		}

		std::uint32_t presentModeCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, nullptr);
		if (presentModeCount != 0)
		{
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device,
													  m_surface,
													  &presentModeCount,
													  details.presentModes.data());
		}

		return details;
	}

	VkSurfaceFormatKHR GraphicsDevice::choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (const auto& availableFormat : availableFormats)
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
				availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return availableFormat;
			}
		}
		return availableFormats[0];
	}

	VkPresentModeKHR GraphicsDevice::choose_swap_present_mode(const std::vector<VkPresentModeKHR>& availablePresentModes)
	{
		for (const auto& availablePresentMode : availablePresentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return availablePresentMode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D GraphicsDevice::choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != std::numeric_limits<unsigned int>::max())
		{
			return capabilities.currentExtent;
		}
		else
		{
			VkExtent2D actualExtent = {
				static_cast<std::uint32_t>(m_windowDimensions.right - m_windowDimensions.left),
				static_cast<std::uint32_t>(m_windowDimensions.bottom - m_windowDimensions.top)
			};
			actualExtent.width = std::clamp(actualExtent.width,
											capabilities.minImageExtent.width,
											capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height,
											 capabilities.minImageExtent.height,
											 capabilities.maxImageExtent.height);
			return actualExtent;
		}
	}

	void GraphicsDevice::create_swapchain()
	{
		SwapChainSupportDetails swapChainSupport = query_swapchain_support(m_physicalDevice);

		VkSurfaceFormatKHR surfaceFormat = choose_swap_surface_format(swapChainSupport.formats);
		VkPresentModeKHR presentMode = choose_swap_present_mode(swapChainSupport.presentModes);
		VkExtent2D extent = choose_swap_extent(swapChainSupport.capabilities);

		std::uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.maxImageCount > 0 &&
			imageCount > swapChainSupport.capabilities.maxImageCount)
		{
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = m_surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		QueueFamilyIndices indices = find_queue_families(m_physicalDevice);
		std::array<std::uint32_t, 2> queueFamilyIndices = {indices.graphicsFamily.value(),
															indices.presentFamily.value()};
		if (indices.graphicsFamily != indices.presentFamily)
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
		}
		else
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			//createInfo.queueFamilyIndexCount = 0;
			//createInfo.pQueueFamilyIndices = nullptr;
		}
		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		err::check_ret_val(vkCreateSwapchainKHR(m_device,
												&createInfo,
												nullptr,
												&m_swapChain),
						   "Failed to create swap chain.");
		vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, nullptr);
		m_swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, m_swapChainImages.data());
		m_swapChainImageFormat = surfaceFormat.format;
		m_swapChainExtent = extent;
	}

	void GraphicsDevice::create_image_views()
	{
		m_swapChainImageViews.resize(m_swapChainImages.size());

		for (size_t i = 0; i < m_swapChainImages.size(); i++)
		{
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = m_swapChainImageFormat;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			createInfo.image = m_swapChainImages[i];
			err::check_ret_val(vkCreateImageView(m_device,
												 &createInfo,
												 nullptr,
												 &m_swapChainImageViews[i]),
							   "Failed to create image views");
		}
	}

	void GraphicsDevice::create_render_pass()
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = m_swapChainImageFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		err::check_ret_val(vkCreateRenderPass(m_device,
											  &renderPassInfo,
											  nullptr,
											  &m_renderPass),
						   "Failed to create render pass.");
	}

	uint32_t GraphicsDevice::find_memory_type(uint32_t typeFilter,
											  VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties{};
		vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);
		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if ((typeFilter & (1 << i)) &&
				(memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		throw std::runtime_error{"Failed to find suitable memory type."};
	}

	void GraphicsDevice::create_vertex_buffer()
	{
		VkBufferCreateInfo bufferCreateInfo{};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.pNext = nullptr;
		bufferCreateInfo.flags = 0;
		bufferCreateInfo.size = static_cast<VkDeviceSize>(m_vertices.size() * sizeof(m_vertices[0]));
		bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		bufferCreateInfo.queueFamilyIndexCount = 0;
		bufferCreateInfo.pQueueFamilyIndices = nullptr;

		err::check_ret_val(vkCreateBuffer(m_device,
										  &bufferCreateInfo,
										  nullptr,
										  &m_vertexBuffer),
						   "Failed to create vertex buffer.");

		VkMemoryRequirements memRequirements{};
		vkGetBufferMemoryRequirements(m_device, m_vertexBuffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = find_memory_type(memRequirements.memoryTypeBits,
													 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		err::check_ret_val(vkAllocateMemory(m_device,
											&allocInfo,
											nullptr,
											&m_vertexBufferMemory),
						   "Failed to allocate vertex buffer memory.");

		vkBindBufferMemory(m_device, m_vertexBuffer, m_vertexBufferMemory, 0);

		void* data{};
		vkMapMemory(m_device, m_vertexBufferMemory, 0, bufferCreateInfo.size, 0, &data);
		std::memcpy(data, m_vertices.data(), static_cast<size_t>(bufferCreateInfo.size));
		vkUnmapMemory(m_device, m_vertexBufferMemory);
	}

	void GraphicsDevice::create_graphics_pipeline()
	{
		std::vector<char> vertShaderCode = read_file(get_shader_path("vert.spv"));
		std::vector<char> fragShaderCode = read_file(get_shader_path("frag.spv"));
		VkShaderModule vertShaderModule = create_shader_module(vertShaderCode);
		VkShaderModule fragShaderModule = create_shader_module(fragShaderCode);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

		VkVertexInputBindingDescription bindingDescription = gfx::Vertex::get_binding_description();
		std::array<VkVertexInputAttributeDescription, 2> attributeDescription = gfx::Vertex::get_attribute_description();

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.size());
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescription.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(m_swapChainExtent.width);
		viewport.height = static_cast<float>(m_swapChainExtent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = {0, 0};
		scissor.extent = m_swapChainExtent;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f;
		rasterizer.depthBiasClamp = 0.0f;
		rasterizer.depthBiasSlopeFactor = 0.0f;

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f;
		multisampling.pSampleMask = nullptr;
		multisampling.alphaToCoverageEnable = VK_FALSE;
		multisampling.alphaToOneEnable = VK_FALSE;

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT |
			VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;

		err::check_ret_val(vkCreatePipelineLayout(m_device,
												  &pipelineLayoutInfo,
												  nullptr,
												  &m_pipelineLayout),
						   "Failed to create pipeline layout.");

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = nullptr;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = nullptr;
		pipelineInfo.layout = m_pipelineLayout;
		pipelineInfo.renderPass = m_renderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.basePipelineIndex = -1;

		err::check_ret_val(vkCreateGraphicsPipelines(m_device,
													 VK_NULL_HANDLE,
													 1,
													 &pipelineInfo,
													 nullptr,
													 &m_graphicsPipeline),
						   "Failed to create GraphicsDevice pipeline.");

		vkDestroyShaderModule(m_device, fragShaderModule, nullptr);
		vkDestroyShaderModule(m_device, vertShaderModule, nullptr);
	}

	VkShaderModule GraphicsDevice::create_shader_module(const std::vector<char>& code)
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		VkShaderModule shaderModule;
		err::check_ret_val(vkCreateShaderModule(m_device,
												&createInfo,
												nullptr,
												&shaderModule),
						   "Failed to create shader module.");
		return shaderModule;
	}

	void GraphicsDevice::create_framebuffers()
	{
		m_swapChainFramebuffers.resize(m_swapChainImageViews.size());

		for (size_t i = 0; i < m_swapChainImageViews.size(); i++)
		{
			VkImageView attachments[] = {m_swapChainImageViews[i]};
			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = m_renderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = m_swapChainExtent.width;
			framebufferInfo.height = m_swapChainExtent.height;
			framebufferInfo.layers = 1;

			err::check_ret_val(vkCreateFramebuffer(m_device,
												   &framebufferInfo,
												   nullptr,
												   &m_swapChainFramebuffers[i]),
							   "Failed to create framebuffer.");
		}
	}

	void GraphicsDevice::create_command_pool()
	{
		QueueFamilyIndices queueFamilyIndices = find_queue_families(m_physicalDevice);

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
		poolInfo.flags = 0;

		err::check_ret_val(vkCreateCommandPool(m_device,
											   &poolInfo,
											   nullptr,
											   &m_commandPool),
						   "Failed to create command pool.");
	}

	void GraphicsDevice::create_command_buffers()
	{
		m_commandBuffers.resize(m_swapChainFramebuffers.size());
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = static_cast<std::uint32_t>(m_commandBuffers.size());

		err::check_ret_val(vkAllocateCommandBuffers(m_device,
													&allocInfo,
													m_commandBuffers.data()),
						   "Failed to allocate command buffers.");

		for (size_t i = 0; i < m_commandBuffers.size(); i++)
		{
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = 0;
			beginInfo.pInheritanceInfo = nullptr;

			err::check_ret_val(vkBeginCommandBuffer(m_commandBuffers[i],
													&beginInfo),
							   "Failed to begin recording command buffer.");

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = m_renderPass;
			renderPassInfo.framebuffer = m_swapChainFramebuffers[i];
			renderPassInfo.renderArea.offset = {0,0};
			renderPassInfo.renderArea.extent = m_swapChainExtent;
			VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearColor;

			vkCmdBeginRenderPass(m_commandBuffers[i],
								 &renderPassInfo,
								 VK_SUBPASS_CONTENTS_INLINE);
			vkCmdBindPipeline(m_commandBuffers[i],
							  VK_PIPELINE_BIND_POINT_GRAPHICS,
							  m_graphicsPipeline);
			VkBuffer vertexBuffers[] = {m_vertexBuffer};
			VkDeviceSize offsets[] = {0};
			vkCmdBindVertexBuffers(m_commandBuffers[i],
								   0,
								   1,
								   vertexBuffers,
								   offsets);
			vkCmdDraw(m_commandBuffers[i],
					  static_cast<uint32_t>(m_vertices.size()),
					  1,
					  0,
					  0);
			vkCmdEndRenderPass(m_commandBuffers[i]);
			err::check_ret_val(vkEndCommandBuffer(m_commandBuffers[i]),
							   "Failed to record command buffer.");
		}
	}

	void GraphicsDevice::create_sync_objects()
	{
		m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
		m_imagesInFlight.resize(m_swapChainImages.size(), VK_NULL_HANDLE);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphoreInfo.pNext = nullptr;
		semaphoreInfo.flags = 0;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.pNext = nullptr;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			err::check_ret_val(vkCreateSemaphore(m_device,
												 &semaphoreInfo,
												 nullptr,
												 &m_imageAvailableSemaphores[i]),
							   "Failed to create semapore.");
			err::check_ret_val(vkCreateSemaphore(m_device,
												 &semaphoreInfo,
												 nullptr,
												 &m_renderFinishedSemaphores[i]),
							   "Failed to create semapore.");
			err::check_ret_val(vkCreateFence(m_device,
											 &fenceInfo,
											 nullptr,
											 &m_inFlightFences[i]),
							   "Failed to create fence.");
		}
	}

	void GraphicsDevice::draw_triangles(const std::vector<font_triangle_t>& triangles)
	{
		m_vertices.resize(3 * triangles.size());
		for (const auto& triangle : triangles)
		{
			m_vertices.push_back({std::get<0>(triangle),
								 {1.0f, 0.0f, 0.0f}});
			m_vertices.push_back({std::get<1>(triangle),
									{0.0f, 1.0f, 0.0f}});
			m_vertices.push_back({std::get<2>(triangle),
									{0.0f, 0.0f, 1.0f}});
		}
		m_updateVertexBuffer = true;
	}

	void GraphicsDevice::draw_frame()
	{
		vkWaitForFences(m_device,
						1,
						&m_inFlightFences[m_currentFrame],
						VK_TRUE,
						std::numeric_limits<std::uint64_t>::max());
		std::uint32_t imageIndex = 0;


		if (m_updateVertexBuffer)
		{
			recreate_swapchain();
		}
		VkResult result = vkAcquireNextImageKHR(m_device,
												m_swapChain,
												std::numeric_limits<std::uint64_t>::max(),
												m_imageAvailableSemaphores[m_currentFrame],
												VK_NULL_HANDLE,
												&imageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			recreate_swapchain();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw err::VulkanException{"Failed to acquire swapchain image."};
		}
		if (m_imagesInFlight[imageIndex] != VK_NULL_HANDLE)
		{
			vkWaitForFences(m_device,
							1,
							&m_imagesInFlight[imageIndex],
							VK_TRUE,
							std::numeric_limits<std::uint64_t>::max());
		}
		m_imagesInFlight[imageIndex] = m_inFlightFences[m_currentFrame];

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = {m_imageAvailableSemaphores[m_currentFrame]};
		VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_commandBuffers[imageIndex];
		VkSemaphore signalSemaphores[] = {m_renderFinishedSemaphores[m_currentFrame]};
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(m_device,
					  1,
					  &m_inFlightFences[m_currentFrame]);
		err::check_ret_val(vkQueueSubmit(m_graphicsQueue,
										 1,
										 &submitInfo,
										 m_inFlightFences[m_currentFrame]),
						   "Failed to submit draw command buffer.");

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;
		VkSwapchainKHR swapChains[] = {m_swapChain};
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr;
		result = vkQueuePresentKHR(m_presentQueue, &presentInfo);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_framebufferResized)
		{
			m_framebufferResized = false;
			recreate_swapchain();
		}
		else if (result != VK_SUCCESS)
		{
			throw err::VulkanException{"Failed to present swapchain image."};
		}
		m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void GraphicsDevice::recreate_swapchain()
	{
		vkDeviceWaitIdle(m_device);
		cleanup_swapchain();

		if (m_updateVertexBuffer)
		{
			m_updateVertexBuffer = false;
			destroy_vertex_buffer();
		}

		create_swapchain();
		create_image_views();
		create_render_pass();
		create_graphics_pipeline();
		create_framebuffers();
		create_vertex_buffer();
		create_command_buffers();
	}

	void GraphicsDevice::cleanup_swapchain()
	{
		for (auto framebuffer : m_swapChainFramebuffers)
		{
			vkDestroyFramebuffer(m_device, framebuffer, nullptr);
		}
		vkFreeCommandBuffers(m_device,
							 m_commandPool,
							 static_cast<std::uint32_t>(m_commandBuffers.size()),
							 m_commandBuffers.data());
		vkDestroyPipeline(m_device, m_graphicsPipeline, nullptr);
		vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
		vkDestroyRenderPass(m_device, m_renderPass, nullptr);
		for (auto imageView : m_swapChainImageViews)
		{
			vkDestroyImageView(m_device, imageView, nullptr);
		}
		vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);
	}

	void GraphicsDevice::destroy_vertex_buffer()
	{
		vkDestroyBuffer(m_device, m_vertexBuffer, nullptr);
		vkFreeMemory(m_device, m_vertexBufferMemory, nullptr);
	}

	void GraphicsDevice::resize_buffer(const math::Rect_t& rect)
	{

	}
}
#endif