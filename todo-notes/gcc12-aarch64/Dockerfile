# The image gaol-bookworm-cross of cross-test: GCC 12.2 for aarch64 (Debian 12
# Bookworm), CMake and qemu-aarch64 7.2, the compiler of the job "Debian 12
# Bookworm arm64" of the CI (process.md, section 11).
FROM debian:bookworm
RUN apt-get update -qq \
 && apt-get install -y -qq --no-install-recommends \
      g++-aarch64-linux-gnu gcc-aarch64-linux-gnu cmake make qemu-user ca-certificates \
 && rm -rf /var/lib/apt/lists/*
