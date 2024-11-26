// C libraries
#include <stdlib.h> //malloc/free/exit
#include <stdio.h> //puts/fputs/fprintf
#include <inttypes.h>
#include <stdbool.h>
#include <string.h> //strcmp
#include <limits.h>
#include <assert.h>
// POSIX headers
#include <unistd.h>
#include <signal.h>

// GLFW library
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// Personal libraries
#include "shorttypes.h"

// Window macros
#define WIN_WIDTH 1280u //u32
#define WIN_HEIGHT 720u //u32
#define WIN_TITLE "real"

// Vulkan macros
#define APP_NAME "real"
#ifdef NDEBUG //in cpp it means "Not debug" for us it means debug due to assert.h
#define enableValidationLayers true
#else
#define enableValidationLayers false
#endif

// some debug stuff
#ifdef NDEBUG
// #define uf8 u8
// #define uf16 u16
// #define uf32 u32
static u32 StupidCounter=0u;
#define IAMHERE printf("I AM HERE %u\n",++StupidCounter);fflush(stdout);
#include"debugV2.h"
#endif
// end of preprocessor

typedef struct{
    GLFWwindow *window;
    VkInstance instance;
}App;


inline static void cleanup(App * const pApp){
    if(pApp == NULL)return; //if ex() is called before initilisation (it segfaults)
    vkDestroyInstance(pApp->instance, NULL); //NULL or a valid instance, NULL.
    if(pApp->window != NULL)glfwDestroyWindow(pApp->window); //Documentation doesn't mention if NULL is accepted but it works
}

static App * globalAppPointer=NULL;
static void ex(const uf16 errcode, const char * const file, const uf16 line){
    fputs("\nError: ",stderr);
    switch(errcode){
        case 1u:fputs("malloc failure",stderr);break;
        case 101u:fputs("glfwInit failure",stderr);break;
        case 102u:fputs("Validation layers requested, but not available!",stderr);break;
        case 103u:fputs("vkCreateInstance failure",stderr);break;
        case 104u:fputs("Missing necessary GLFW extentions",stderr);break;
        case 105u:fputs("atexit failure",stderr);break;
        default:fputs("Unknown error",stderr);
    }
    if(file != NULL)
        fprintf(stderr,", at line %"PRINTuf16" in the file %s",line,file);
    fputc('\n',stderr);
    cleanup(globalAppPointer);
    exit(EXIT_FAILURE);
}

static void f_atexit(void){
    glfwTerminate(); // can be called before glfwInit
#ifdef NDEBUG
    debug_memory_end(); // can be called before debug_memory_init
#endif
}
#define ex(errcode) ex(errcode,__FILE__,__LINE__) //pApp is hard coded in the macro

inline static void mainLoop(const App * const pApp){
    while(!glfwWindowShouldClose(pApp->window))
        glfwPollEvents();
}


inline static void initWindow(App * const pApp){

    glfwInitHint(GLFW_ANGLE_PLATFORM_TYPE,GLFW_ANGLE_PLATFORM_TYPE_VULKAN);

    if(!glfwInit())ex(101u); //glfw initilisation (not an error checker)

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); //no openGL
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); //no window resizing

    pApp->window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, WIN_TITLE, NULL, NULL);
}



static const char * const validationLayers[] = {"VK_LAYER_KHRONOS_validation"};
#define validationLayersCount (sizeof validationLayers /sizeof validationLayers[0])//u32

inline static bool checkValidationLayerSupport(void){//UIcreateInstance
    u32 layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);

    VkLayerProperties *availableLayers = malloc(layerCount * sizeof(VkLayerProperties));
    if(availableLayers == NULL)ex(1u);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

    bool layerFound;
    for(u32 i=0u; i<validationLayersCount; ++i){
        layerFound = false;
        for(u32 j=0u; j<layerCount; ++j)
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

inline static bool verifyExtensionSupport(const u32 VulkanextensionCount,const u32 glfwExtensionCount,const char* const restrict* const restrict glfwExtensions,const VkExtensionProperties * const restrict extensions){ //UIcreateInstance
    bool foundExtension;
    for(u32 i=0u; i<glfwExtensionCount; ++i){
        foundExtension = false;
        for(u32 j=0u; j<VulkanextensionCount; ++j)
            if(strcmp(extensions[j].extensionName,glfwExtensions[i]) == 0){
                foundExtension = true;
                break;
            }
        if(!foundExtension)return false;
    }
    return true;
}

inline static void createInstance(App * const pApp){
    if(enableValidationLayers && !checkValidationLayerSupport())
        ex(102u);

    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = APP_NAME,
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_0,
        .pNext = NULL
    };

    u32 glfwExtensionCount = 0u;
    const char **glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    VkInstanceCreateInfo createInfo = {
    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pApplicationInfo = &appInfo,
    .enabledExtensionCount = glfwExtensionCount,
    .ppEnabledExtensionNames = glfwExtensions,
    };
    if(enableValidationLayers){
        createInfo.enabledLayerCount = validationLayersCount;
        createInfo.ppEnabledLayerNames = validationLayers;
    }else
        createInfo.enabledLayerCount = 0;

    if(vkCreateInstance(&createInfo, NULL, &(pApp->instance))!=VK_SUCCESS)ex(103u);

    u32 extensionCount = 0u;
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);

    VkExtensionProperties *extensions = malloc(extensionCount * sizeof(VkExtensionProperties));
    if(extensions == NULL)ex(1u);
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, extensions);

#ifdef NDEBUG // listing extensions and checking matches
    puts("Vulkan available extentions:");
    for(u32 i=0u; i<extensionCount; ++i)
        printf("\textension: %s\n",extensions[i].extensionName);
    puts("GLFW required extentions:");
    for(u32 i=0u; i<glfwExtensionCount; ++i)
        printf("\textension: %s\n",glfwExtensions[i]);
#endif // end of listing extensions
    if(verifyExtensionSupport(extensionCount,glfwExtensionCount,glfwExtensions,extensions)){
        free(extensions);
#ifdef NDEBUG
        puts("All necessary GLFW extentions are available");
#endif
    }
    else{
        free(extensions);
        ex(104u);
    }
}

inline static void initVulkan(App * const pApp){
    createInstance(pApp);
}

int main(/*int argc,char *argv[],char *env[]*/){
    if(atexit(f_atexit))ex(105u);
#ifdef NDEBUG
    debug_memory_init(250u); //NOTHING GOES BEFORE THIS
#endif
//////////////////////////////////////////////// PRE MAIN
    App al = {0};
    globalAppPointer = &al;
    initWindow(&al);
    initVulkan(&al);
    mainLoop(&al);
    cleanup(&al);

    return EXIT_SUCCESS;
}
//system implementation checks
static_assert(-9/5==-1,"Division implementation is incompatible");
static_assert(-11/-5==2,"Division implementation is incompatible");
static_assert(sizeof(double) == 8,"Double is not 64 bit");
static_assert(sizeof(float) == 4,"float is not 32 bit ");
//macro type checks
static_assert(WIN_WIDTH<=UINT32_MAX,"type failure");
static_assert(WIN_HEIGHT<=UINT32_MAX,"type failure");
static_assert(validationLayersCount<=UINT32_MAX,"type failure");
