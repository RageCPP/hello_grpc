#!/bin/sh

exec_path="$(pwd)"
build_path="$exec_path/build"
grpc_path="$exec_path/grpc"

clear() {
  rm -rf "$build_path"
  rm -rf "$grpc_path"
  mkdir "$build_path"
  mkdir "$grpc_path"
}

generate_build() {
  clear
  cmake -B "${build_path}" -S "${exec_path}"
}

grpc_lib_build() {
  cmake --build "${build_path}" --target grpc_lib -- -j"$(nproc)"
}

grpc_lite_lib_build() {
  cmake --build "${build_path}" --target grpc_lite_lib -- -j"$(nproc)"
}


cli_build() {
  cmake --build "${build_path}" --target cli -- -j"$(nproc)"
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
    --local_server)
      local_server_build
      ;;
    --remote_server)
      remote_server_build
      ;;
    *)
      ;;
  esac
}

main "$@" || exit 1
