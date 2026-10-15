# Docker-Tasmota
Quickly set up a build environment for [Tasmota](https://github.com/arendst/Tasmota) using Docker

## Gruff Hospitality relays

This copy builds the firmware for Gruff Hospitality's relays (Shelly 1PM
Gen4). Everything Gruff-specific is in `user_config_override.h`, which
explains each setting.

1. Copy `user_config_secrets.example.h` to `user_config_secrets.h` and fill
   in the Wi-Fi and the web UI password (kept in Bitwarden). It's gitignored,
   and `compile.sh` won't build without it.
2. `./compile.sh tasmota32c6`. It also copies in the Gruff CA trust anchor
   from `../gruff-automation-ca`. The firmware is `tasmota32c6.bin`
   (`tasmota32c6.factory.bin` for a first flash over serial).
3. Give each relay its client certificate:
   `../gruff-automation-ca/generate-client-cert.sh <MAC>` prints the
   `TLSKey1`/`TLSKey2` commands for its console.
4. After flashing over an earlier Gruff build, send `Reset 1`: the defaults
   only apply on first boot and after a reset.

The relays connect to `hospitality.grufftechnology.ee:8883` with mutual
TLS, as the topic `%012X` (their MAC). With no broker for 30 minutes, or
after a power cut, they switch on and leave the room to its thermostat.
The web UI's user is `admin`.

Note that `compile.sh` pulls the latest Tasmota development branch first.

This repo is a fork of `tasmota/docker-tasmota`, kept as the `upstream`
remote. To take in upstream changes:

```
git fetch upstream
git merge upstream/master
git push
```

If you've cloned the fork fresh, add the remote first:
`git remote add upstream https://github.com/tasmota/docker-tasmota.git`.

## compile.sh
This bash script makes compiling a lot easier without the need to type lengthy commands each time.

compile.sh is intended to run on a linux machine with docker and git installed. If you're missing any of these the script will pop a warning with instructions how to install.

Running the script for the first time will pull the latest blakadder/docker-tasmota container (you can edit the script to use your own docker container), clone the latest Tasmota development branch (you can change to clone the latest stable release version by setting `USE_STABLE=1`) inside the script folder and copy platformio_override.ini and user_config_override.h to Tasmota folder.

Running the script with one or more build names (as listed in platformio_tasmota_env.ini) as parameters will compile only those builds regardless of platformio.ini or platformio_override.ini

`./compile.sh tasmota-sensors tasmota-PT`    
compiles both the tasmota-sensors.bin and the portuguese language version of Tasmota

If you have a `user_config_override.h` or `platformio_override.ini` file with your custom settings you can put them in the script folder and they will be used on the next script run. 

Script will update the repo folder with the latest one every run.

To check compiling logs use `cat docker-tasmota.log`

## Option 1: Setup the prebuilt docker container using the compile.sh script
1. Clone this repo and cd to the dir where its cloned:
    ```
    git clone https://github.com/tasmota/docker-tasmota
    cd docker-tasmota
    ```
2. Update the `user_config_override.h` and/or `platformio_override.ini` files with your custom build settings
3. Run compile.sh with the desired build name, ex:
    ```
    ./compile.sh tasmota
    ```
    If necessary the `compile.sh` script will install the container and Tasmota repo if you type 'yes' when prompted

5. When compiling finishes you should have the compiled binary and gzipped version in `Tasmota/build_output/firmware` which can be flashed on your devices

Note: If you want to update the docker image installed previously by the `compile.sh` script run:
```
docker pull blakadder/docker-tasmota
```

## Option 2: Setup a docker container from scratch

1. Clone this repo and cd to the dir where its cloned:    
    ```
    git clone https://github.com/tasmota/docker-tasmota
    cd docker-tasmota
    ```

2. Run this to build the docker container:
    `docker build -t docker-tasmota .`

3. Move to a directory where you want to clone Tasmota repo:
    ```
    git clone https://github.com/arendst/Tasmota.git
    ```
If you have a `user_config_override.h` or `platformio_override.ini` file with your custom settings, you will need to put them under `Tasmota/tasmota`.

4. From the same directory run to compile the desired build   
`docker run -ti --rm -v $(pwd)/Tasmota:/tasmota -e HOST_UID=$UID -e HOST_GID=$GID docker-tasmota -e tasmota-PT`

> The container runs as root to avoid permission issues with PlatformIO. The `HOST_UID` and `HOST_GID` environment variables ensure that the compiled firmware files have the correct ownership for your host user.

> `-e <buildname>` where <buildname> can be any of the [builds listed in platformio.ini](https://github.com/arendst/Tasmota/blob/063611314777d4dd9dc8c25905f19f8b25f510aa/platformio.ini#L18). If you don't define a build then ***every*** build will get compiled.

5. When compiling finishes you should have the compiled binary and gzipped version in `Tasmota/build_output/firmware` which can be flashed on your devices.

## Switch to a branch other than development

`cd Tasmota`

List branches with `git branch -a`

Switch to release branch with

`git checkout release`

Build it and run:
```docker
docker run -ti --rm \
-v $(pwd)/Tasmota:/tasmota \
-e HOST_UID=$UID -e HOST_GID=$GID docker-tasmota
```
