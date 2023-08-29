#!/bin/sh

exec_path="$(pwd)"
build_path="$exec_path/build"
build_release_path="$build_path/release"
build_debug_path=""
grpc_path="$exec_path/grpc"

clear() {
  rm -rf "$build_path"
  rm -rf "$grpc_path"
  mkdir "$build_path"
  mkdir "$grpc_path"
  mkdir "$build_release_path"
}

generate_build() {
  clear
  cmake -B "${build_release_path}" -S "${exec_path}" --preset=x86_64-linux-g++-Release
}

grpc_lib_build() {
  cmake --build "${build_path}" --target grpc_lib -- -j"$(nproc)"
}

grpc_lite_lib_build() {
  export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/home/ragecpp/hey-cpp/ubuntu-22-04/protobuf_install/share/lib:/home/ragecpp/hey-cpp/ubuntu-22-04/grpc_install/share/lib";
  cmake --build "${build_release_path}" --target grpc_lite_lib --preset=release -- -j"$(nproc)"
}


cli_build() {
  cmake --build "${build_path}" --target cli -- -j"$(nproc)"
}

cli_lite_build() {
  cmake --build "${build_path}" --target cli_lite --preset=release -- -j"$(nproc)"
}

local_server_build() {
  cmake --build "${build_path}" --target local_server -- -j"$(nproc)"
}

remote_server_build() {
  cmake --build "${build_path}" --target remote_server -- -j"$(nproc)"
}

main() {
  case $1 in
    -c)
      clear
      ;;
    -g)
      generate_build
      ;;
    --grpc_lib)
      grpc_lib_build
      ;;
    --grpc_lite_lib)
      grpc_lite_lib_build
      ;;
    --cli)
      cli_build
      ;;
    --cli_lite)
      cli_lite_build
      ;;
    --local_server)
      local_server_build
      ;;
    --remote_server)
      remote_server_build
      ;;
    *)
      echo "not support command"
      exit 1
      ;;
  esac
}

main "$@" || exit 1
