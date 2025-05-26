# ff-wii

Forwarder Factory for the Nintendo Wii

TODO: Port ssock to Wii and use it here.

## Building

As far as I can tell, this only builds on an Arch Linux host, and quite frankly I cannot be bothered to port it to anything else.
You can use Docker as a development environment if you want to.

1. Run the following:
`source /etc/profile.d/devkit-env.sh`
If you do not run this, /opt/devkitpro will be used, which will work fine, so this is mostly optional.
If this file does not exist, install DevkitPro with `pacman -S wii-dev`.
2. Run `./install-deps.sh` to download and install the dependencies. This might be optional for now, I don't know.
3. Run `./create-build-directory.sh` to create a build directory.
4. Run `cmake --build cmake-build-debug`

If you're using a JetBrains IDE such as CLion, the included dotfiles should work well enough.

## License

See included LICENSE file.
