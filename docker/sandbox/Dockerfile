# Sandbox Container for Malware Analysis
FROM ubuntu:20.04

# Prevent interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Install necessary packages for file analysis
RUN apt-get update && apt-get install -y \
    strace \
    ltrace \
    procps \
    file \
    strings \
    hexdump \
    netcat \
    tcpdump \
    iproute2 \
    net-tools \
    curl \
    wget \
    unzip \
    python3 \
    python3-pip \
    vim \
    nano \
    && rm -rf /var/lib/apt/lists/*

# Create sandbox user for security
RUN useradd -m -s /bin/bash sandbox

# Create sandbox directories
RUN mkdir -p /sandbox /sandbox/temp /sandbox/logs /sandbox/quarantine
RUN chown -R sandbox:sandbox /sandbox

# Set up monitoring script
COPY monitor.sh /usr/local/bin/monitor.sh
RUN chmod +x /usr/local/bin/monitor.sh

# Copy sandbox utilities
COPY sandbox-utils.py /usr/local/bin/sandbox-utils.py
RUN chmod +x /usr/local/bin/sandbox-utils.py

# Set working directory
WORKDIR /sandbox

# Switch to sandbox user
USER sandbox

# Default command
CMD ["/bin/bash"]
