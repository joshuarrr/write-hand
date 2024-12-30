# WriteHand

A minimalist writing app built with Qt6 that focuses on distraction-free writing and efficient file management.

## Features

- 🎯 Distraction-free writing interface
- 📁 Smart file organization with locations, favorites, and tags
- 🗄️ Archive system for managing older documents
- 🌓 Dark mode support
- 📝 Rich text editing capabilities
- 🔄 Auto-save functionality
- 🎨 Modern, native macOS look and feel

## Building from Source

### Prerequisites

- Qt 6.x
- CMake 3.16 or higher
- macOS 11.0 or higher
- Xcode Command Line Tools

### Build Instructions

1. Clone the repository:

```bash
git clone https://github.com/joshuarrr/write-hand.git
cd write-hand
```

2. Create a build directory and run CMake:

```bash
mkdir build && cd build
cmake ..
```

3. Build the project:

```bash
make
```

4. Run the application:

```bash
open WriteHand.app
```

## Development

### Project Structure

- `src/` - Source files
  - Main application logic
  - UI components
  - File management
- `icons/` - Application icons and assets
- `styles/` - QSS style sheets
- `scripts/` - Build and development scripts

### Development Scripts

- `dev.sh` - Sets up development environment
- `watch.sh` - Watches for changes and triggers rebuild
- `scripts/generate_icons.sh` - Generates application icons

## License

MIT License - See [LICENSE](LICENSE) for details.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.
