#
# Copyright Elasticsearch B.V. and/or licensed to Elasticsearch B.V. under one
# or more contributor license agreements. Licensed under the Elastic License;
# you may not use this file except in compliance with the Elastic License.
#

FROM centos:7 AS builder

# This is basically automating the setup instructions in build-setup/linux.md

MAINTAINER David Roberts <dave.roberts@elastic.co>

# Make sure OS packages are up to date and required packages are installed
# libffi and openssl are required for building Python
RUN \
  rm /var/lib/rpm/__db.* && \
  yum install -y bzip2 gcc gcc-c++ git libffi-devel make openssl-devel texinfo unzip wget which zip zlib-devel

# For compiling with hardening and optimisation
ENV CFLAGS "-g -O3 -fstack-protector -D_FORTIFY_SOURCE=2 -msse4.2 -mfpmath=sse"
ENV CXXFLAGS "-g -O3 -fstack-protector -D_FORTIFY_SOURCE=2 -msse4.2 -mfpmath=sse"
ENV LDFLAGS "-Wl,-z,relro -Wl,-z,now"
ENV LDFLAGS_FOR_TARGET "-Wl,-z,relro -Wl,-z,now"

ARG build_dir=/usr/src

# Build gcc 9.3
RUN \
  cd ${build_dir} && \
  wget --quiet -O - http://ftpmirror.gnu.org/gcc/gcc-9.3.0/gcc-9.3.0.tar.gz | tar zxf - && \
  cd gcc-9.3.0 && \
  contrib/download_prerequisites && \
  sed -i -e 's/$(SHLIB_LDFLAGS)/-Wl,-z,relro -Wl,-z,now $(SHLIB_LDFLAGS)/' libgcc/config/t-slibgcc && \
  cd .. && \
  mkdir gcc-9.3.0-build && \
  cd gcc-9.3.0-build && \
  ../gcc-9.3.0/configure --prefix=/usr/local/gcc93 --enable-languages=c,c++ --enable-vtable-verify --with-system-zlib --disable-multilib && \
  make -j`nproc` && \
  make install && \
  cd .. && \
  rm -rf gcc-9.3.0 gcc-9.3.0-build

# Update paths to use the newly built compiler in C++17 mode
ENV LD_LIBRARY_PATH /usr/local/gcc93/lib64:/usr/local/gcc93/lib:/usr/lib:/lib
ENV PATH /usr/local/gcc93/bin:/usr/bin:/bin:/usr/sbin:/sbin
ENV CXX "g++ -std=gnu++17"

# Build binutils
RUN \
  cd ${build_dir} && \
  wget --quiet -O - http://ftpmirror.gnu.org/binutils/binutils-2.34.tar.bz2 | tar jxf - && \
  cd binutils-2.34 && \
  ./configure --prefix=/usr/local/gcc93 --enable-vtable-verify --with-system-zlib --disable-libstdcxx --with-gcc-major-version-only && \
  make -j`nproc` && \
  make install && \
  cd .. && \
  rm -rf binutils-2.34

# Build libxml2
RUN \
  cd ${build_dir} && \
  wget --quiet -O - ftp://anonymous@ftp.xmlsoft.org/libxml2/libxml2-2.9.7.tar.gz | tar zxf - && \
  cd libxml2-2.9.7 && \
  ./configure --prefix=/usr/local/gcc93 --without-python --without-readline && \
  make -j`nproc` && \
  make install && \
  cd .. && \
  rm -rf libxml2-2.9.7

# Build Boost
RUN \
  cd ${build_dir} && \
  wget --quiet -O - http://dl.bintray.com/boostorg/release/1.71.0/source/boost_1_71_0.tar.bz2 | tar jxf - && \
  cd boost_1_71_0 && \
  ./bootstrap.sh --without-libraries=context --without-libraries=coroutine --without-libraries=graph_parallel --without-libraries=mpi --without-libraries=python --without-icu && \
  sed -i -e 's/(17ul)(29ul)(37ul)(53ul)(67ul)(79ul) \\/(3ul)(17ul)(29ul)(37ul)(53ul)(67ul)(79ul) \\/' boost/unordered/detail/implementation.hpp && \
  ./b2 -j`nproc` --layout=versioned --disable-icu pch=off optimization=speed inlining=full define=BOOST_MATH_NO_LONG_DOUBLE_MATH_FUNCTIONS define=BOOST_LOG_WITHOUT_DEBUG_OUTPUT define=BOOST_LOG_WITHOUT_EVENT_LOG define=BOOST_LOG_WITHOUT_SYSLOG define=BOOST_LOG_WITHOUT_IPC define=_FORTIFY_SOURCE=2 cxxflags='-std=gnu++17 -fstack-protector -msse4.2 -mfpmath=sse' linkflags='-std=gnu++17 -Wl,-z,relro -Wl,-z,now' && \
  ./b2 install --prefix=/usr/local/gcc93 --layout=versioned --disable-icu pch=off optimization=speed inlining=full define=BOOST_MATH_NO_LONG_DOUBLE_MATH_FUNCTIONS define=BOOST_LOG_WITHOUT_DEBUG_OUTPUT define=BOOST_LOG_WITHOUT_EVENT_LOG define=BOOST_LOG_WITHOUT_SYSLOG define=BOOST_LOG_WITHOUT_IPC define=_FORTIFY_SOURCE=2 cxxflags='-std=gnu++17 -fstack-protector -msse4.2 -mfpmath=sse' linkflags='-std=gnu++17 -Wl,-z,relro -Wl,-z,now' && \
  cd .. && \
  rm -rf boost_1_71_0

# Build patchelf
RUN \
  cd ${build_dir} && \
  wget --quiet -O - http://nixos.org/releases/patchelf/patchelf-0.10/patchelf-0.10.tar.gz | tar zxf - && \
  cd patchelf-0.10 && \
  ./configure --prefix=/usr/local/gcc93 && \
  make -j`nproc` && \
  make install && \
  cd .. && \
  rm -rf patchelf-0.10

# Build Python 3.7
# --enable-optimizations for a stable/release build
# Not using --prefix=/usr/local/gcc93 so that this can be excluded from the final image
RUN \
  cd ${build_dir} && \
  wget --quiet -O - https://www.python.org/ftp/python/3.7.9/Python-3.7.9.tgz | tar xzf - && \
  cd Python-3.7.9 && \
  ./configure --enable-optimizations && \
  make altinstall && \
  cd .. && \
  rm -rf Python-3.7.9

# Install Python dependencies
RUN \
  /usr/local/bin/pip3.7 install numpy ninja pyyaml setuptools cffi typing_extensions future six requests dataclasses

# Install CMake
# v3.5 minimum is required
RUN \
  cd ${build_dir} && \
  wget --quiet https://github.com/Kitware/CMake/releases/download/v3.19.3/cmake-3.19.3-Linux-x86_64.sh && \
  chmod +x cmake-3.19.3-Linux-x86_64.sh && \
  mkdir /usr/local/cmake && \
  ./cmake-3.19.3-Linux-x86_64.sh --skip-license --prefix=/usr/local/cmake && \
  rm -f cmake-3.19.3-Linux-x86_64.sh

# Add cmake to PATH
ENV PATH=${PATH}:/usr/local/cmake/bin

# Clone PyTorch and build LibTorch
# If the PyTorch branch is changed also update PYTORCH_BUILD_VERSION
RUN \
  cd ${build_dir} && \
  git -c advice.detachedHead=false clone --depth=1 --branch=v1.8.0 https://github.com/pytorch/pytorch.git && \
  cd pytorch && \
  git submodule sync && \
  git submodule update --init --recursive && \
  sed -i -e 's/system(/strlen(/' torch/csrc/jit/codegen/fuser/cpu/fused_kernel.cpp && \
  export BLAS=Eigen && \
  export BUILD_TEST=OFF && \
  export BUILD_CAFFE2=OFF && \
  export USE_NUMPY=OFF && \
  export USE_DISTRIBUTED=OFF && \
  export USE_MKLDNN=OFF && \
  export USE_QNNPACK=OFF && \
  export USE_PYTORCH_QNNPACK=OFF && \
  export USE_XNNPACK=OFF && \
  export PYTORCH_BUILD_VERSION=1.8.0 && \
  export PYTORCH_BUILD_NUMBER=1 && \
  /usr/local/bin/python3.7 setup.py install && \
  mkdir /usr/local/gcc93/include/pytorch && \
  cp -r torch/include/* /usr/local/gcc93/include/pytorch/ && \
  cp torch/lib/libtorch_cpu.so /usr/local/gcc93/lib && \
  cp torch/lib/libc10.so /usr/local/gcc93/lib && \
  cd .. && \
  rm -rf pytorch

FROM centos:7
COPY --from=builder /usr/local/gcc93 /usr/local/gcc93
RUN \
  rm /var/lib/rpm/__db.* && \
  yum install -y bzip2 gcc git make unzip which zip zlib-devel
