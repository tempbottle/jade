
# Add libraries
add_subdirectory(lib/ConfigurationEngine)
add_subdirectory(lib/RVCEngine)
add_subdirectory(lib/Graph)
add_subdirectory(lib/IRActor)
add_subdirectory(lib/IRCore)
add_subdirectory(lib/IRJit)
add_subdirectory(lib/IRMerger)
add_subdirectory(lib/IROptimize)
add_subdirectory(lib/IRSerialize)
add_subdirectory(lib/IRUtil)
add_subdirectory(lib/RoundRobinScheduler)
add_subdirectory(lib/Scenario)
add_subdirectory(lib/TinyXml)
add_subdirectory(lib/XCFSerialize)
add_subdirectory(lib/XDFSerialize)

if(BUILD_GPAC_RVC_DECODER)
    add_subdirectory(gpac/RVCDecoder)
endif()

# Aplications
add_subdirectory(jade)
