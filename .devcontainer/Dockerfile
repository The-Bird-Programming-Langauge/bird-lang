FROM mcr.microsoft.com/devcontainers/cpp:1-debian-12

ARG REINSTALL_CMAKE_VERSION_FROM_SOURCE="3.20.0"

COPY ./reinstall-cmake.sh /tmp/

RUN sudo apt-get update && apt-get clean && \
    sudo apt-get install -y build-essential libzstd-dev libgtest-dev libgmock-dev 

RUN sudo apt-get install bison flex -y

RUN sudo apt-get install binaryen

RUN sudo apt-get install nodejs -y

RUN if [ "${REINSTALL_CMAKE_VERSION_FROM_SOURCE}" != "none" ]; then \
        chmod +x /tmp/reinstall-cmake.sh && /tmp/reinstall-cmake.sh ${REINSTALL_CMAKE_VERSION_FROM_SOURCE}; \
    fi \
    && rm -f /tmp/reinstall-cmake.sh

