add_library(app_infra STATIC
    logging/Log.cpp
)

target_include_directories(app_infra PUBLIC
    "${PROJECT_SOURCE_DIR}/src"
)
target_compile_features(app_infra PUBLIC cxx_std_20)
