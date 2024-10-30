// C libraries
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>
// GLFW for windowing
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
// Window macros
#define WIN_WIDTH 800u //uint32_t
#define WIN_HEIGHT 800u //uint32_t
#define WIN_TITLE "This is not X11"
// Vulkan macros
#ifdef NDEBUG
#define enableValidationLayers false
#else
#define enableValidationLayers true
#endif
// some debug stuff
//static uint32_t StupidCounter=0u;
#define IAMHERE printf("I AM HERE %d\n",++StupidCounter);
// end of preprocessor



const char *validationLayers[] = {"VK_LAYER_KHRONOS_validation"};
const uint32_t validationLayersCount=sizeof(validationLayers)/sizeof(validationLayers[0]);


typedef struct{
    GLFWwindow *window;
    VkInstance instance;
}App;

inline static void ex(const char *message){
    if(message!=NULL)
    fprintf(stderr,"%s",message);
    glfwTerminate();
    exit(EXIT_FAILURE);
}

void initWindow(App*);
void initVulkan(App*);
void mainLoop(App*);
void cleanup(App*);

void createInstance(App*);

bool checkValidationLayerSupport(void);
////////////////////////////////////////////////////////////////////////////////////////////////
int main(/*int argc,char *argv[],char *env[]*/){
    App pp={0};
    initWindow(&pp);
    initVulkan(&pp);
    mainLoop(&pp);
    cleanup(&pp);

    glfwTerminate();
    return EXIT_SUCCESS;
}

void initWindow(App *pApp){
    glfwInitHint(GLFW_PLATFORM,GLFW_PLATFORM_WAYLAND);
    // Chooses window protocol

    glfwInitHint(GLFW_ANGLE_PLATFORM_TYPE,GLFW_ANGLE_PLATFORM_TYPE_VULKAN);
    // Chooses the renderer

    //glfwInitHint(GLFW_WAYLAND_LIBDECOR,GLFW_WAYLAND_PREFER_LIBDECOR);
    // Wayland stuff

    if(!glfwInit())ex(NULL);

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); //no openGL
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); //no window resizing

    pApp->window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, WIN_TITLE, NULL, NULL);
}
void initVulkan(App *pApp){
    createInstance(pApp);
}
void mainLoop(App *pApp){
    while(!glfwWindowShouldClose(pApp->window)){
        glfwPollEvents();
    }
}
void cleanup(App *pApp){
    vkDestroyInstance(pApp->instance, NULL);

    glfwDestroyWindow(pApp->window);
    glfwTerminate();
}


bool verifyExtensionSupport(
    const uint32_t extensionCount, //Vulkan extensionCount
    const uint32_t glfwExtensionCount,
    const char** restrict glfwExtensions,
    const VkExtensionProperties * restrict extensions){
    bool foundExtension;
    for(uint32_t i=0u; i<glfwExtensionCount; ++i){
        foundExtension = false;
        for(uint32_t j=0u; j<extensionCount; ++j)
            if(strcmp(extensions[j].extensionName,glfwExtensions[i]) == 0){
                foundExtension = true;
                break;
            }
        if(!foundExtension)return false;
    }
    return true;
}

void createInstance(App *pApp){
    if(enableValidationLayers && !checkValidationLayerSupport())
        ex("validation layers requested, but not available!\n");

    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = WIN_TITLE,
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_0,
        .pNext = NULL
    };

    uint32_t glfwExtensionCount = 0u;
    const char **glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    VkInstanceCreateInfo createInfo = {
    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pApplicationInfo = &appInfo,
    .enabledExtensionCount = glfwExtensionCount,
    .ppEnabledExtensionNames = glfwExtensions,
    .enabledLayerCount = 0
    };

    if(vkCreateInstance(&createInfo, NULL, &(pApp->instance))!=VK_SUCCESS)ex(NULL);

    uint32_t extensionCount = 0u;
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);

    VkExtensionProperties *extensions = malloc(sizeof(VkExtensionProperties) * extensionCount);
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, extensions);

    // listing extensions and checking matches
    printf("Vulkan available extentions:\n");
    for(uint32_t i=0u; i<extensionCount; ++i)
        printf("\textension: %s\n",extensions[i].extensionName);
    printf("GLFW required extentions:\n");
    for(uint32_t i=0u; i<glfwExtensionCount; ++i)
        printf("\textension: %s\n",glfwExtensions[i]);
    // end of listing extensions
    if(verifyExtensionSupport(extensionCount,glfwExtensionCount,glfwExtensions,extensions))
        printf("All necessary GLFW extentions are available\n");
    else{
        free(extensions);
        ex("Missing necessary GLFW extentions\n");
    }
}

bool checkValidationLayerSupport(void){
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);

    VkLayerProperties *availableLayers = malloc(sizeof(VkLayerProperties) * layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

    bool layerFound;
    for(uint32_t i=0u; i<validationLayersCount; ++i){
        layerFound = false;
        for(uint32_t j=0u; j<layerCount; ++j)
            if(strcmp(availableLayers[j].layerName,validationLayers[i]) == 0){
                layerFound = true;
                break;
            }
        if(!layerFound){
            free(availableLayers);
            return false;
        }
    }
    free(availableLayers);
    return true;
}

