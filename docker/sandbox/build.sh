#!/bin/bash

# Build Sandbox Docker Image

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}Building Sandbox Docker Image...${NC}"

# Check if Docker is running
if ! docker info > /dev/null 2>&1; then
    echo -e "${RED}Error: Docker daemon is not running${NC}"
    exit 1
fi

# Build the image
cd "$SCRIPT_DIR"

echo -e "${YELLOW}Building avproject-sandbox:latest...${NC}"
if docker build -t avproject-sandbox:latest .; then
    echo -e "${GREEN}✓ Sandbox image built successfully${NC}"
else
    echo -e "${RED}✗ Failed to build sandbox image${NC}"
    exit 1
fi

# Verify the image
echo -e "${YELLOW}Verifying image...${NC}"
if docker images | grep "avproject-sandbox" | grep "latest"; then
    echo -e "${GREEN}✓ Image verification successful${NC}"
    
    # Show image details
    echo -e "${YELLOW}Image details:${NC}"
    docker images avproject-sandbox:latest
else
    echo -e "${RED}✗ Image verification failed${NC}"
    exit 1
fi

echo -e "${GREEN}Sandbox Docker image is ready for use!${NC}"
