#!/bin/bash

echo "🚀 Starting NiceTop_OS setup..."

# Detect OS
OS_TYPE=$(uname -s)
echo "🔍 Detected OS: $OS_TYPE"

# Function to install dependencies on macOS
install_macos_deps() {
    echo "📦 Installing macOS dependencies..."
    
    # Check if Homebrew is installed
    if ! command -v brew &> /dev/null; then
        echo "Installing Homebrew..."
        /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
    fi
    
    # Install required packages
    brew install nasm qemu xorriso wget mtools
}

# Function to install dependencies on Linux
install_linux_deps() {
    echo "📦 Installing Linux dependencies..."
    sudo apt-get update
    sudo apt-get install -y nasm qemu-system-x86 xorriso mtools build-essential wget
    
    # Check if cross-compiler exists
    if ! command -v x86_64-elf-gcc &> /dev/null; then
        echo "⚠️  Warning: x86_64-elf-gcc not found. Please ensure you have the x86_64 ELF cross-compiler installed."
    fi
}

# Install dependencies based on OS
if [[ "$OS_TYPE" == "Darwin" ]]; then
    install_macos_deps
elif [[ "$OS_TYPE" == "Linux" ]]; then
    install_linux_deps
else
    echo "❌ Unsupported OS: $OS_TYPE"
    exit 1
fi

# Build and run
echo "🔨 Cleaning and Building NiceTop_OS..."
make clean
make all

echo "✅ Build complete! Starting QEMU..."
make run
