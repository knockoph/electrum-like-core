FROM docker.io/ubuntu:24.04

RUN apt update \
&& apt install -y wget

ENV BITCOIN_CORE_VERSION=28.0

RUN wget https://bitcoincore.org/bin/bitcoin-core-${BITCOIN_CORE_VERSION}/bitcoin-${BITCOIN_CORE_VERSION}-x86_64-linux-gnu.tar.gz \
&& tar -xf bitcoin-${BITCOIN_CORE_VERSION}-x86_64-linux-gnu.tar.gz \
&& rm bitcoin-${BITCOIN_CORE_VERSION}-x86_64-linux-gnu.tar.gz

ENV PATH=bitcoin-${BITCOIN_CORE_VERSION}/bin:$PATH
