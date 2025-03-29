cmake_minimum_required(VERSION 3.10)

project(GithubUserActivity)

set(CMAKE_CXX_STANDARD 17)

add_executable(github-user-activity ./src/main.cc ./src/activity.cc)

target_include_directories(github-user-activity PRIVATE include)

find_package(CURL REQUIRED)
target_link_libraries(github-user-activity PRIVATE CURL::libcurl)
