#! /bin/bash

# installs GridPACK dependencies to the current directory

set -xeuo pipefail

function install_boost {
  local boost_version=$1

  # check args
  : "${boost_version:?}"

  echo "--- Installing Boost ${boost_version} ---"

  # remove existing
  rm -rf boost*

  # download
  echo "Downloading Boost"
  wget \
    "https://boostorg.jfrog.io/artifactory/main/release/${boost_version}/source/boost_${boost_version//./_}.tar.gz" \
    -O boost.tar.gz \
    --quiet

  # unpack
  echo "Unpacking Boost"
  tar -xf boost.tar.gz && rm boost.tar.gz

  # remove version from dir name
  mv "boost_${boost_version//./_}" boost

  pushd boost || exit

  # bootstrap
  echo "Bootstrapping Boost"
  ./bootstrap.sh \
    --prefix=install_for_gridpack \
    --with-libraries=mpi,serialization,random,filesystem,system \
    >../log/boost_bootstrap.log 2>&1
  echo 'using mpi ;' >>project-config.jam

  # build
  echo "Building Boost"
  ./b2 -a -d+2 link=shared stage >../log/boost_build.log 2>&1

  # install
  echo "Installing Boost"
  ./b2 -a -d+2 link=shared install >../log/boost_install.log 2>&1

  popd || exit

  echo "Boost installation complete"
}

function install_ga {
  local ga_version=$1

  # check args
  : "${ga_version:?}"

  echo "--- Installing Global Arrays ${ga_version} ---"

  # download
  echo "Downloading Global Arrays"
  wget \
    "https://github.com/GlobalArrays/ga/releases/download/v${ga_version}/ga-${ga_version}.tar.gz" \
    -O ga.tar.gz \
    --quiet

  # unpack
  echo "Unpacking Global Arrays"
  tar -xf ga.tar.gz && rm ga.tar.gz

  # remove version from dir name
  mv "ga-${ga_version}" ga

  pushd ga || exit

  # build
  echo "Configuring Global Arrays"
  ./configure \
    --with-mpi-ts \
    --disable-f77 \
    --without-blas \
    --enable-cxx \
    --enable-i4 \
    --prefix="${PWD}/install_for_gridpack" \
    --enable-shared \
    >../log/ga_configure.log 2>&1

  # install
  echo "Installing Global Arrays"
  make -j 10 install >../log/ga_install.log 2>&1

  popd || exit

  echo "Global Arrays installation complete"
}

function install_petsc {
  local petsc_version=$1

  # check args
  : "${petsc_version:?}"

  echo "--- Installing PETSc ${petsc_version} ---"

  # clone
  echo "Cloning PETSc repository"
  git clone https://gitlab.com/petsc/petsc.git

  pushd petsc || exit

  git checkout "tags/v${petsc_version}" -b "v${petsc_version}"

  export PETSC_DIR=${PWD}
  export PETSC_ARCH=build-dir

  # install
  echo "Configuring PETSc"
  ./configure \
    --download-superlu_dist \
    --download-metis \
    --download-parmetis \
    --download-suitesparse \
    --download-f2cblaslapack \
    --download-cmake \
    --prefix="${PWD}"/install_for_gridpack \
    --scalar-type=complex \
    --with-shared-libraries=1 \
    --download-f2cblaslapack=1 \
    >../log/petsc_configure.log 2>&1

  # build
  echo "Building PETSc"
  make >../log/petsc_build.log 2>&1

  # install
  echo "Installing PETSc"
  make install >../log/petsc_install.log 2>&1

  # check
  echo "Checking PETSc"
  make check >../log/petsc_check.log 2>&1

  popd || exit

  echo "PETSc installation complete"
}

echo "Installing GridPACK dependencies"
date

mkdir -p log

install_boost "1.78.0"
install_ga "5.8"
install_petsc "3.16.4"

echo "GridPACK dependency installation complete"
