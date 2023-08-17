cmake_minimum_required(VERSION 3.25)

project(hello-grpc-server VERSION 0.0.1 LANGUAGES CXX)

option(CMAKE_VERBOSE_MAKEFILE "show compilation process" ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "compile_commands.json generate")
add_compile_options(-lpthread -ldl -std=c++17)
cmake_path(SET dep_dir "/home/ragecpp/hey-cpp/ubuntu-22-04")
cmake_path(APPEND dep_dir "abseil-cpp_install" OUTPUT_VARIABLE absl_dir)
cmake_path(APPEND dep_dir "protobuf_install" OUTPUT_VARIABLE protobuf_dir)
cmake_path(APPEND dep_dir "zlib_install" OUTPUT_VARIABLE zlib_dir)
cmake_path(APPEND dep_dir "re2_install" OUTPUT_VARIABLE re2_dir)
cmake_path(APPEND dep_dir "grpc_install" OUTPUT_VARIABLE grpc_dir)
set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${absl_dir};${protobuf_dir};${zlib_dir};${re2_dir};${grpc_dir}")

cmake_path(SET proto_dir "${CMAKE_CURRENT_SOURCE_DIR}")
cmake_path(SET proto_file "${proto_dir}/people.proto")
cmake_path(SET proto_builded "${CMAKE_CURRENT_SOURCE_DIR}/grpc")
cmake_path(SET build_proto_cc "${proto_builded}/people.pb.cc")
cmake_path(SET build_proto_h "${proto_builded}/people.pb.h")
cmake_path(SET build_grpc_cc "${proto_builded}/people.grpc.pb.cc")
cmake_path(SET build_grpc_h "${proto_builded}/people.grpc.pb.h")
cmake_path(SET _PROTOBUF_PROTOC "${dep_dir}/protobuf_install/bin/protoc")
cmake_path(SET _GRPC_CPP_PLUGIN_EXECUTABLE "${dep_dir}/grpc_install/bin/grpc_cpp_plugin")
add_custom_command(
  OUTPUT "${build_proto_cc}" "${build_proto_h}" "${build_grpc_cc}" "${build_grpc_h}"
  COMMAND ${_PROTOBUF_PROTOC}
  ARGS --grpc_out "${proto_builded}" --cpp_out "${proto_builded}" -I "${proto_dir}" --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}" "${proto_file}"
  DEPENDS "${proto_file}")

include_directories("${proto_builded}")

list(APPEND GRPCSource ${build_proto_cc} ${build_proto_h} ${build_grpc_cc} ${build_grpc_h})
cmake_path(SET local_server_path "${CMAKE_SOURCE_DIR}/local_server")
cmake_path(SET remote_server_path "${CMAKE_SOURCE_DIR}/remote_server")
cmake_path(SET cli_path "${CMAKE_SOURCE_DIR}/cli")
add_library(grpc_lib ${GRPCSource})
add_library(grpc_lite_lib ${GRPCSource})
add_executable(local_server  ${GRPCSource} "${local_server_path}/main.cpp")
add_executable(remote_server ${GRPCSource} "${remote_server_path}/main.cpp")
add_executable(cli ${GRPCSource} "${cli_path}/main.cpp")

find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

set(_REFLECTION gRPC::grpc++_reflection)
set(_GRPC_GRPCPP gRPC::grpc++)
set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
set(_PROTOBUF_LIBPROTOBUF_LITE protobuf::libprotobuf-lite)
target_link_libraries(grpc_lib
  ${_GRPC_GRPCPP}
  ${_REFLECTION}
  ${_PROTOBUF_LIBPROTOBUF})
target_link_libraries(grpc_lite_lib
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF_LITE})

foreach(_target local_server remote_server cli)
  target_link_libraries(${_target}
    PRIVATE
    grpc_lib
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF}
    ${_REFLECTION}
    absl::flags
    absl::flags_parse)
endforeach()
