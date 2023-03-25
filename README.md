## About UE4VNCClient

UE4VNCClient - Unreal Engine 4 blueprint library plugin representing VNC client.

It supports the following VNC encodings: Raw, CopyRect, Tight.

Also VNC Authentitication is supported.

## Getting Started

Move the ```Plugins``` folder into your UE4 project directory and build it.

## Usage

### Functions
Plugin provides two blueprint functions:
- ConnectToVNCServer - setups client settings and connects to the VNC server.
- UpdateClient - requests a framebuffer update from the server and processes the responses. Returns the current framebuffer in a texture form.

### Example

To see example of usage the plugin's functions move the assets from the ```Example``` folder to the ```Content``` folder of your UE4 project.