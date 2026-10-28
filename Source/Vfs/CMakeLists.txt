add_library(IronRingVfs STATIC
    ArchiveFormat.h      ArchiveFormat.cpp
    ArchiveWriter.h      ArchiveWriter.cpp
    DirectoryMount.h     DirectoryMount.cpp
    Mount.h
    VirtualFileSystem.h  VirtualFileSystem.cpp
    ZipMount.h           ZipMount.cpp)
target_include_directories(IronRingVfs PUBLIC "${PROJECT_SOURCE_DIR}/Source")
target_link_libraries(IronRingVfs
    PUBLIC  nlohmann_json::nlohmann_json
    PRIVATE miniz)
ir_configure_target(IronRingVfs)
