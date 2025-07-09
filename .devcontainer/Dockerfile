FROM mcr.microsoft.com/devcontainers/cpp:1-debian-11

ENV EMSDK=/emsdk
ENV EMSCRIPTEN_VERSION=4.0.10

RUN git clone https://github.com/emscripten-core/emsdk.git $EMSDK
RUN echo "## Install Emscripten ${EMSCRIPTEN_VERSION}" \
    && cd ${EMSDK} \
    && ./emsdk install ${EMSCRIPTEN_VERSION} \
    && echo "## Done"

RUN cd ${EMSDK} \
    && echo "## Generate configuration" \
    && ./emsdk activate ${EMSCRIPTEN_VERSION} \
    && chmod 777 ${EMSDK}/upstream/emscripten \
    && chmod -R 777 ${EMSDK}/upstream/emscripten/cache \
    && echo "int main() { return 0; }" > hello.c \
    && ${EMSDK}/upstream/emscripten/emcc -c hello.c \
    && cat ${EMSDK}/upstream/emscripten/cache/sanity.txt \
    && echo "## Done"

ENV PATH=$EMSDK:$EMSDK/upstream/emscripten/:$PATH

RUN echo ". /emsdk/emsdk_env.sh" >> /etc/bash.bashrc
RUN echo 'export EM_NODE_JS="$EMSDK_NODE"' >> /etc/bash.bashrc

RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
    && apt-get -y install --no-install-recommends wget gnupg libdigest-sha3-perl flex bison
