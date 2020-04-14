FROM alpine:latest

ADD entrypoint.sh /entrypoint.sh

RUN \
  addgroup -g 1200 -S app && \
  adduser -g 1200 -u 1200 -S app && \
  apk add --upgrade --no-cache git zip && \
  chmod +x /entrypoint.sh

USER app
VOLUME [ "/git", "/packed" ]
ENTRYPOINT ["/entrypoint.sh"]
