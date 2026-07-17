ARG IMAGE_NAME
ARG IMAGE_VERSION

FROM ${IMAGE_NAME}:${IMAGE_VERSION}

WORKDIR /opt/cfractions

RUN apt-get update \
 && apt-get install -y --no-install-recommends build-essential \
 && rm -rf /var/lib/apt/lists/*
COPY pyproject.toml .
COPY README.md .
COPY setup.py .
COPY cfractions cfractions
COPY src src
COPY tests tests

RUN pip install -e '.[tests]'
