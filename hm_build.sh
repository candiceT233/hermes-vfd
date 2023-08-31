  
HERMES_REPO=/people/tang584/hermes_stage/hermes-vfd

# source $HERMES_REPO/load_hermes_deps.sh

mkdir -p $HERMES_REPO/build

cd $HERMES_REPO/build

# build_type="Release" # Release, RelWithDebInfo, Debug

build_type="$1"
clean_build="$2"

# 0: Release, 1: RelWithDebInfo, 2: Debug
if [ "$build_type" == "0" ]; then
  build_type="Release"
  HERMES_PATH=$HOME/install/vfd_hermes

elif [ "$build_type" == "1" ]; then
  build_type="RelWithDebInfo"
  HERMES_PATH=$HOME/install/vfd_debug_hermes

elif [ "$build_type" == "2" ]; then
  build_type="Debug"
  HERMES_PATH=$HOME/install/vfd_debug_hermes

else
  echo "Invalid build type: $build_type"
  # display instructions
  echo "Usage: ./hm_build.sh [build_type] [optional: clean_build]"
  echo "build_type: 0: Release, 1: RelWithDebInfo, 2: Debug"
  echo "clean_build: 0: clean, 1: not clean"
  exit 1
fi

# If there is option 2, clean the build
if [ "$clean_build" == "0" ]; then
  echo "Clean build"
  rm -rf $HERMES_REPO/build/*
  rm -rf $HERMES_PATH
fi

echo "Build type: $build_type"

cmake ../ -DCMAKE_BUILD_TYPE=$build_type \
  -DCMAKE_INSTALL_PREFIX=$HERMES_PATH \
  -DBUILD_Boost_TESTS=OFF \
  -DBUILD_HSHM_BENCHMARKS=OFF \
  -DBUILD_MPI_TESTS=OFF \
  -DBUILD_OpenMP_TESTS=OFF \
  -DBUILD_TESTING=OFF \
  -DHERMES_BUILD_ADAPTER_TESTS=OFF \
  -DHERMES_BUILD_BENCHMARKS=OFF \
  -DHERMES_ENABLE_PUBSUB_ADAPTER=OFF \
  -DHERMES_ENABLE_STDIO_ADAPTER=OFF \
  -DHERMES_ENABLE_MPIIO_ADAPTER=OFF \
  -DHERMES_ENABLE_POSIX_ADAPTER=ON \
  -DHERMES_ENABLE_VFD=ON \

make -j12
make install