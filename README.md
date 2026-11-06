# Docker-Tasmota
Quickly set up a build environment for [Tasmota](https://github.com/arendst/Tasmota) using Docker

## Gruff Hospitality relays

This copy builds the firmware for Gruff Hospitality's relays. Everything
Gruff-specific is in `user_config_override.h`, which explains each setting.

| Relay           | Chip     | Build         |
| --------------- | -------- | ------------- |
| Shelly 1PM Gen4 | ESP32-C6 | `tasmota32c6` |
| Shelly 1PM Gen3 | ESP32-C3 | `tasmota32c3` |

1. Copy `user_config_secrets.example.h` to `user_config_secrets.h` and fill
   in the site's Wi-Fi, the broker's enrollment password and the web UI
   password (the last two are in Bitwarden). It's gitignored, and
   `compile.sh` won't build without it.
2. `./compile.sh <build>`. It also copies in the Gruff CA trust anchor
   from `../gruff-automation-ca`. The firmware is `<build>.bin`
   (`<build>.factory.bin` for a first flash over serial).
3. Flash it and power the relay up. It joins the Wi-Fi and waits on the
   broker's enrollment port, `8884`, where it can't do anything else.
4. `../gruff-automation-ca/enroll-relays.sh` lists the waiting relays. Check
   the MACs are yours, then `enroll-relays.sh <MAC>...` gives them their
   certificate and moves them to `8883`. It works from anywhere, with no
   access to the relay's network.
5. After flashing over an earlier Gruff build, send `Reset 1`: the defaults
   only apply on first boot and after a reset. It also puts the relay back on
   `8884`, so enroll it again.

Enrolled relays connect to `hospitality.grufftechnology.ee:8883` with mutual
TLS, as the topic `%012X` (their MAC). With no broker for 30 minutes, or
after a power cut, they switch on and leave the room to its thermostat.
The web UI's user is `admin`.

The firmware knows one Wi-Fi network, the site's. To enroll relays before
they reach the site, run a temporary network with the same name and password.
A relay that can't join that network can only be recovered over serial, so
check the name and password before flashing a batch.

### Clock and NTP

A relay needs a plausible clock to accept the broker's certificate, on `8884`
and `8883` alike. Without one its log shows `MQT: TLS connection error: 54`
(`BR_ERR_X509_EXPIRED`, which also covers "not yet valid") and it keeps
retrying. `Status 7` shows what it thinks the time is.

The TLS check uses the later of the NTP time and the time settings were last
saved to flash:

- **A freshly flashed relay** has no saved time, so it can't connect until NTP
  answers. Tasmota retries about once a minute, and even with NTP working a
  relay has taken about 10 minutes.
- **An enrolled relay** that lost power starts from its saved time, and
  connects without NTP while that time is within the broker certificate's
  validity. Let's Encrypt's certificates last 90 days and renew about 30 days
  before they expire, so a relay left unplugged across a renewal has a saved
  time from before the new certificate, and needs NTP to connect.

So every site's network has to allow outbound NTP (UDP 123). If it doesn't,
use an NTP server it can reach: `NTP_SERVER1` in the firmware, or
`NtpServer1 <address>` on a relay. To get one relay going by hand,
`Time <unix time>` sets the clock and turns NTP off; `Time 0` turns it back on.

### Building

`compile.sh` pulls the latest Tasmota development branch first. It runs one
compile job at a time, because two can run Docker out of memory, which kills
the compiler. With more memory for Docker, `PIO_JOBS=2 ./compile.sh <build>`
is faster. It exits with an error if a build fails, and the compiler's output
is in `docker-tasmota.log`.

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
