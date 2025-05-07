# ABX Mock Exchange Client

This C++ client connects to the ABX mock exchange server over TCP, sends requests to stream all packets or resend specific packets, and handles the responses according to the specified protocol.

## Requirements

- C++ compiler (GCC or Clang)
- Linux or macOS (or compatible system)
- The ABX server should be running on port 3000 (default)

1. Ensure you have a C++ compiler installed (e.g., GCC or Clang).
2. Run the following command to compile the `client.cpp` file:

   ```bash
   g++ -o client client.cpp
