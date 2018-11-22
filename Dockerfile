FROM alpine

RUN apk add --no-cache build-base
RUN apk add --no-cache build-base gcc abuild binutils binutils-doc gcc-doc
RUN apk add --no-cache cmake cmake-doc

RUN mkdir /code
ADD . /code
WORKDIR /code

RUN cmake .
RUN make install

ENTRYPOINT ["gpkg"]
