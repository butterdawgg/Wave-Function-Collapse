set(SOURCES 
    "src/main.cpp"
    "src/wfc/grid.cpp"
    "src/app/app.cpp"
    "src/app/grid_renderer.cpp"
    "src/app/camera.cpp"
    "src/wfc/wfc_solver.cpp"
    "src/wfc/wfc_trainer.cpp"
    "src/wfc/wfc_settings_io.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE raylib nlohmann_json::nlohmann_json imgui rlimgui)

target_include_directories(${PROJECT_NAME} PRIVATE src)

# Copy data folder to output directory after every build, preserving structure
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/data
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/data
    COMMENT "Copying data folder to output directory"
)