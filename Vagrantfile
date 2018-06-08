# -*- mode: ruby -*-
# vi: set ft=ruby :

Vagrant.require_version ">= 2.0.0"

Vagrant.configure("2") do |config|
  config.vm.box = "bento/ubuntu-18.04"
  config.vm.box_check_update = false
  config.vm.synced_folder "./", "/workspace"
  config.vm.provision "shell", inline: <<-SHELL
  sudo apt-get update
    sudo DEBIAN_FRONTEND=noninteractive apt-get install -y \
        arping                                             \
        autoconf                                           \
        autogen                                            \
        build-essential                                    \
        clang-format                                       \
        cmake                                              \
        gdb                                                \
        libtool                                            \
        ninja-build                                        \
        pkg-config                                         \
        python3                                            \
        python3-pip                                        \
        python3-setuptools                                 \
        tmux                                               \
        unzip                                              \
        valgrind                                           \
        wget

    sudo pip3 install meson

    cd /tmp &&                                                                        \
        wget https://github.com/libevent/libevent/archive/release-2.1.8-stable.zip && \
        unzip -q release-2.1.8-stable.zip &&                                          \
        cd libevent-release-2.1.8-stable &&                                           \
        ./autogen.sh &&                                                               \
        ./configure &&                                                                \
        sudo make -j $(nproc) install

    cd /tmp &&                                                                 \
        wget https://github.com/gflags/gflags/archive/v2.2.2.zip &&            \
        unzip -q v2.2.2.zip &&                                                 \
        cd gflags-2.2.2 &&                                                     \
        mkdir build &&                                                         \
        cd build &&                                                            \
        cmake -DCMAKE_BUILD_TYPE=RELEASE .. &&                                 \
        sudo make -j $(nproc) install

    cd /tmp &&                                                                 \
        wget https://github.com/abseil/googletest/archive/release-1.8.1.zip && \
        unzip -q release-1.8.1.zip &&                                          \
        cd googletest-release-1.8.1 &&                                         \
        mkdir build &&                                                         \
        cd build &&                                                            \
        cmake -DCMAKE_BUILD_TYPE=RELEASE .. &&                                 \
        sudo make -j $(nproc) install

    cd /tmp &&                                                                 \
        wget https://github.com/google/benchmark/archive/v1.4.1.zip &&         \
        unzip -q v1.4.1.zip &&                                                 \
        cd benchmark-1.4.1 &&                                                  \
        mkdir build &&                                                         \
        cd build &&                                                            \
        cmake -DCMAKE_BUILD_TYPE=RELEASE .. &&                                 \
        sudo make -j $(nproc) install

    echo "cd /workspace" >> /home/vagrant/.bashrc
  SHELL
  config.vm.provider "virtualbox" do |vb|
    vb.cpus = "4"
    vb.memory = "4096"
  end
end
