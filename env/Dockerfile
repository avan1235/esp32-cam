FROM debian:bullseye

ENV DEBIAN_FRONTEND=noninteractive

CMD ["bash"]

RUN apt-get update  \
    && apt-get install -y  \
        ssh \
        build-essential \
        gcc \
        g++ \
        gdb \
        clang \
        make \
        ninja-build \
        cmake \
        autoconf \
        automake \
        libtool \
        valgrind \
        locales-all \
        dos2unix \
        rsync \
        tar \
        python \
        python-dev \
        apt-utils \
        bison \
        ca-certificates \
        ccache \
        check \
        cmake  \
        curl \
        dfu-util \
        flex \
        git \
        git-lfs \
        gperf \
        lcov \
        libbsd-dev \
        libffi-dev \
        libncurses-dev \
        libssl-dev  \
        libusb-1.0-0-dev \
        make \
        ninja-build \
        python3 \
        python3-venv \
        ruby \
        unzip \
        wget \
        xz-utils \
        zip \
    && apt-get autoremove -y \
    && rm -rf /var/lib/apt/lists/* \
    && update-alternatives --install /usr/bin/python python /usr/bin/python3 10

RUN cd /opt && \
    git clone --recursive https://github.com/espressif/esp-idf.git

RUN cd /opt/esp-idf && \
    git checkout tags/v5.0.1 -b latest

RUN cd /opt/esp-idf && git submodule update --init --recursive && ./install.sh all
RUN echo "if [ -f /root/.bashrc ]; then source /root/.bashrc; fi" >> /root/.bash_profile
RUN echo "export IDF_PATH=/opt/esp-idf" >> /root/.bashrc
ENV IDF_PATH=/opt/esp-idf
RUN echo "export PATH=/opt/esp-idf/components/esptool_py/esptool:/opt/esp-idf/components/espcoredump:/opt/esp-idf/components/partition_table:/opt/esp-idf/components/app_update:/root/.espressif/tools/xtensa-esp-elf-gdb/11.2_20220823/xtensa-esp-elf-gdb/bin:/root/.espressif/tools/riscv32-esp-elf-gdb/11.2_20220823/riscv32-esp-elf-gdb/bin:/root/.espressif/tools/xtensa-esp32-elf/esp-2022r1-11.2.0/xtensa-esp32-elf/bin:/root/.espressif/tools/xtensa-esp32s2-elf/esp-2022r1-11.2.0/xtensa-esp32s2-elf/bin:/root/.espressif/tools/xtensa-esp32s3-elf/esp-2022r1-11.2.0/xtensa-esp32s3-elf/bin:/root/.espressif/tools/riscv32-esp-elf/esp-2022r1-11.2.0/riscv32-esp-elf/bin:/root/.espressif/tools/esp32ulp-elf/2.35_20220830/esp32ulp-elf/bin:/root/.espressif/tools/openocd-esp32/v0.11.0-esp32-20220706/openocd-esp32/bin:/root/.espressif/python_env/idf5.0_py3.9_env/bin:/opt/esp-idf/tools:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin" >> /root/.bashrc
ENV PATH=/opt/esp-idf/components/esptool_py/esptool:/opt/esp-idf/components/espcoredump:/opt/esp-idf/components/partition_table:/opt/esp-idf/components/app_update:/root/.espressif/tools/xtensa-esp-elf-gdb/11.2_20220823/xtensa-esp-elf-gdb/bin:/root/.espressif/tools/riscv32-esp-elf-gdb/11.2_20220823/riscv32-esp-elf-gdb/bin:/root/.espressif/tools/xtensa-esp32-elf/esp-2022r1-11.2.0/xtensa-esp32-elf/bin:/root/.espressif/tools/xtensa-esp32s2-elf/esp-2022r1-11.2.0/xtensa-esp32s2-elf/bin:/root/.espressif/tools/xtensa-esp32s3-elf/esp-2022r1-11.2.0/xtensa-esp32s3-elf/bin:/root/.espressif/tools/riscv32-esp-elf/esp-2022r1-11.2.0/riscv32-esp-elf/bin:/root/.espressif/tools/esp32ulp-elf/2.35_20220830/esp32ulp-elf/bin:/root/.espressif/tools/openocd-esp32/v0.11.0-esp32-20220706/openocd-esp32/bin:/root/.espressif/python_env/idf5.0_py3.9_env/bin:/opt/esp-idf/tools:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin

RUN cd /opt/esp-idf/components && git clone --recursive https://github.com/espressif/esp32-camera.git

RUN ( \
    echo 'LogLevel DEBUG2'; \
    echo 'PermitRootLogin yes'; \
    echo 'PasswordAuthentication yes'; \
    echo 'Subsystem sftp /usr/lib/openssh/sftp-server'; \
  ) > /etc/ssh/sshd_config_esp_idf_clion \
  && mkdir /run/sshd

RUN yes password | passwd root

CMD ["/usr/sbin/sshd", "-D", "-e", "-f", "/etc/ssh/sshd_config_esp_idf_clion"]