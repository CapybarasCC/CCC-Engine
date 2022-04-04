#
# First download and extract SFML 2.4.1 for the respective OS we are on
#
FIND_PACKAGE( Boost 1.55 COMPONENTS program_options REQUIRED )

set(DEPS_DIR "${CMAKE_CURRENT_LIST_DIR}/dependencies/")
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(SFML_URL "http://www.sfml-dev.org/files/SFML-2.4.1-osx-clang.tar.gz")
    set(SFML_DIR ${DEPS_DIR}/SFML-2.4.1-osx-clang)
	if (NOT EXISTS "${SFML_DIR}")
		message("Downloading SFML for Mac OS X")
		file(DOWNLOAD "${SFML_URL}" "${DEPS_DIR}/sfml.tar.gz")
		execute_process(
			COMMAND ${CMAKE_COMMAND} -E tar xzf  ${DEPS_DIR}/sfml.tar.gz
			WORKING_DIRECTORY ${DEPS_DIR}
    	)
		# copy freetype over to Frameworks/ so rpath resoultion works
		execute_process(
				COMMAND ${CMAKE_COMMAND} -E copy_directory ${SFML_DIR}/extlibs/freetype.framework ${SFML_DIR}/Frameworks/freetype.framework
				WORKING_DIRECTORY ${SFML_DIR}
		)
	endif()
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(SFML_URL "http://www.sfml-dev.org/files/SFML-2.5.1-linux-gcc-64-bit.tar.gz")
    set(SFML_DIR ${DEPS_DIR}/SFML-2.4.1)
	if (NOT EXISTS ${SFML_DIR})
		message("Downloading SFML for Linux 64-bit")
		file(DOWNLOAD "${SFML_URL}" "${DEPS_DIR}/sfml.tar.gz")
		execute_process(
			COMMAND ${CMAKE_COMMAND} -E tar xzf  ${DEPS_DIR}/sfml.tar.gz
			WORKING_DIRECTORY ${DEPS_DIR}
    	)
	endif()
else()
	set(SFML_URL "http://www.sfml-dev.org/files/SFML-2.4.1-windows-vc14-32-bit.zip")
	set(SFML_DIR ${DEPS_DIR}/SFML-2.4.1)
	if (NOT EXISTS ${SFML_DIR})
		message("Downloading SFML for Windows 32-bit")
		file(DOWNLOAD "${SFML_URL}" "${DEPS_DIR}/sfml.zip")
		execute_process(
			COMMAND ${CMAKE_COMMAND} -E tar x  ${DEPS_DIR}/sfml.zip
			WORKING_DIRECTORY ${DEPS_DIR}
    	)
	endif()
endif()

if(MSVC)
	message("MSCV detected")
	set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_CRT_SECURE_NO_WARNINGS")
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_CRT_SECURE_NO_WARNINGS")
else()
	set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic -std=c89")
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wnon-virtual-dtor -pedantic -std=c++11 -fexceptions -fno-rtti")
endif()

# Define spine-sfml-cpp library
include_directories(spine ${SFML_DIR}/include include include/egif ${Boost_INCLUDE_DIR})
file(GLOB_RECURSE EGIF_SOURCES "include/egif/*.h" "include/egif/*.cpp" "include/*.h" "include/*.hpp")
file(GLOB INCLUDES "include/**/*.h" "include/**/*.hpp")
file(GLOB SOURCES "spine/*.cpp")
add_library(spine-sfml-cpp STATIC ${SOURCES} ${INCLUDES})
add_library(egif STATIC ${EGIF_SOURCES})
target_link_libraries(spine-sfml-cpp )
install(TARGETS spine-sfml-cpp DESTINATION dist/lib)
install(FILES ${INCLUDES} DESTINATION dist/include)

# Define spine-sfml src executable
add_executable(create_collection src/main.cpp)

# Link in SFML libraries and OS dependencies like OpenGL
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	find_library(SFML SFML PATHS ${SFML_DIR}/Frameworks)
	find_library(SFML_SYSTEM "sfml-system" PATHS ${SFML_DIR}/Frameworks)
	find_library(SFML_WINDOW sfml-window PATHS ${SFML_DIR}/Frameworks)
	find_library(SFML_GRAPHICS sfml-graphics PATHS ${SFML_DIR}/Frameworks)
	target_link_libraries(create_collection ${SFML} ${SFML_SYSTEM} ${SFML_WINDOW} ${SFML_GRAPHICS})
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	target_link_libraries(create_collection sfml-graphics sfml-window sfml-system ${Boost_LIBRARIES})
else()
	set(SFML_LIBS ${SFML_DIR}/lib)
	target_link_libraries(create_collection ${SFML_LIBS}/sfml-main-d.lib)
	target_link_libraries(create_collection ${SFML_LIBS}/sfml-graphics-s-d.lib)
	target_link_libraries(create_collection ${SFML_LIBS}/sfml-window-s-d.lib)
	target_link_libraries(create_collection ${SFML_LIBS}/sfml-system-s-d.lib)
	target_link_libraries(create_collection ${SFML_LIBS}/freetype.lib)
	target_link_libraries(create_collection ${SFML_LIBS}/jpeg.lib)
	target_link_libraries(create_collection opengl32)
	target_link_libraries(create_collection gdi32)
	target_link_libraries(create_collection winmm)
	add_definitions(-DSFML_STATIC)
endif()

# copy data to build directory
# add_custom_command(TARGET create_collection PRE_BUILD
# 		COMMAND ${CMAKE_COMMAND} -E copy_directory
# 		${CMAKE_CURRENT_LIST_DIR}/data $<TARGET_FILE_DIR:create_collection>/data)

target_link_libraries(create_collection spine-sfml-cpp egif )
